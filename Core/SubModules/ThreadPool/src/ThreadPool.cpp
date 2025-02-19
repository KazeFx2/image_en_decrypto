//
// Created by Fx Kaze on 25-1-3.
//

#include "ThreadPool.h"

#define W_SEM(t) (t).wait()
#define P_SEM(t) (t).post()

#define C_SEM(t) {}

#ifdef __DEBUG
#define W_SEM_S(t) {printf("[Semaphore Wait Start]id: %d\n", (t).id); W_SEM((t).semaStart);}
#define W_SEM_F(t) {printf("[Semaphore Wait Finish]id: %d\n", (t).id); W_SEM((t).semaFinish);}
#define P_SEM_S(t) {printf("[Semaphore Post Start]id: %d\n", (t).id); P_SEM((t).semaStart);}
#define P_SEM_F(t) {printf("[Semaphore Post Finish]id: %d\n", (t).id); W_SEM((t).semaFinish);}
#else
#define W_SEM_S(t) W_SEM((t).semaStart)
#define W_SEM_F(t) W_SEM((t).semaFinish)
#define P_SEM_S(t) P_SEM((t).semaStart)
#define P_SEM_F(t) P_SEM((t).semaFinish)
#endif

#define CHECK_THROW_TERM(msg, ...) \
    if (mtxContext.termAll) {\
        __VA_ARGS__;\
        char tmp[256];\
        snprintf(tmp, sizeof(tmp), "%s: " msg " a destroy(ing/ed) thread pool is not allowed",\
            __FUNCTION__);\
        throw std::runtime_error(tmp);\
    }

#define CHECK_THROW_TERM_TH(msg, ...) CHECK_THROW_TERM(msg " a thread from", __VA_ARGS__)

ThreadPool::ThreadPool(const u_count_t nThreads): idx(getIdx()),
                                                  threadIdly(nThreads, true), threadEmpty(nThreads, false),
                                                  waitingTasks(1, false), emptyTasks(1, true),
                                                  threads(nThreads) {
    mtxContext.threadCount = nThreads;
    mtxContext.idlyCount = nThreads;
    mtxContext.maxCount = nThreads;
    mtxContext.waitReduceRefers = 0;
    mtxContext.waitFinishRefers = 0;
    mtxContext.waitIdlyRefers = 0;
    mtxContext.reduce = -1;
    mtxContext.termAll = false;
    for (u_count_t i = 0; i < nThreads; i++) {
        threads[i] = new ThreadContext;
        initThread(*threads[i], i);
    }
}

ThreadPool::~ThreadPool() {
    static int iii = 0;
    iii++;
    const auto lockAll = this->lockAll.writer();
    lockAll.lock();
    mtxContext.termAll = true;
    const bool noExists = mtxContext.threadCount == 0;
    const bool doWait = mtxContext.idlyCount != mtxContext.threadCount;
    if (doWait) {
        mtxContext.waitFinishRefers++;
    }
    lockAll.unlock();
    if (doWait) {
        W_SEM(finishWaitSem);
    }
    for (u_count_t i = 0; i < threads.size(); i++) {
        destroyThreadLocked(i);
    }
    if (!noExists)
        W_SEM(poolTermSem);
    else
        lockAll.lock();
    for (u_count_t i = 0; i < threads.size(); i++) {
        delete threads[i];
        C_SEM(threads[i]->semaStart);
        C_SEM(threads[i]->mtxContext.task);
    }
    for (u_count_t i = 0; i < mtxContext.taskArray.size(); i++) {
        delete mtxContext.taskArray[i];
    }
    C_SEM(finishWaitSem);
    C_SEM(reduceWaitSem);
    C_SEM(poolTermSem);
    lockAll.unlock();
}

ThreadPool::u_count_t ThreadPool::addTaskLocked(const funcHandler func, void *param, const bool wait) {
    auto id = emptyTasks.findNextTrue(0, mtxContext.taskArray.size());
    if (id == BITMAP_NOT_FOUND) {
        id = mtxContext.taskArray.size();
        mtxContext.taskArray.push_back(new TaskInfo{
            func, param, wait, 0, THREAD_FAILED, 0, false
        });
        emptyTasks[id] = false;
        waitingTasks[id] = true;
        return id;
    }
    mtxContext.taskArray[id]->func = func;
    mtxContext.taskArray[id]->param_return = param;
    mtxContext.taskArray[id]->wait = wait;
    mtxContext.taskArray[id]->refers = 0;
    mtxContext.taskArray[id]->threadId = THREAD_FAILED;
    mtxContext.taskArray[id]->uniqueId++;
    mtxContext.taskArray[id]->finished = false;
    emptyTasks[id] = false;
    waitingTasks[id] = true;
    return id;
}

ThreadPool::task_descriptor_t ThreadPool::addThread(const funcHandler func, void *param, const bool wait,
                                                    TaskInfo **taskOut) {
    const auto lockAll = this->lockAll.writer();
    lockAll.lock();
    if (mtxContext.termAll) {
        lockAll.unlock();
        throw std::runtime_error("ThreadPool::addThread: trying to add a thread into a destroying thread pool.");
        return THREAD_FAILED;
    }
    const auto taskId = addTaskLocked(func, param, wait);
    auto *task = mtxContext.taskArray[taskId];
    if (taskOut != nullptr) {
        *taskOut = task;
    }
    count_t id = threadIdly.findNextTrue(0, threads.size());
    while (id != BITMAP_NOT_FOUND && threads[id]->mtxContext.forceTerminate)
        id = threadIdly.findNextTrue(id + 1, threads.size());
    if (id != BITMAP_NOT_FOUND) {
        mtxContext.idlyCount--;
        threads[id]->mtxContext.task = task;
        task->threadId = id;
        status(id) = Working;
        threadIdly[id] = false;
        waitingTasks[taskId] = false;
        P_SEM_S(*threads[id]);
        lockAll.unlock();
        return taskId;
    }
    id = threadEmpty.findNextTrue(0, threads.size());
    if (id != BITMAP_NOT_FOUND) {
        startThread(id);
        mtxContext.threadCount++;
        threads[id]->mtxContext.task = task;
        task->threadId = id;
        status(id) = Working;
        threadIdly[id] = false;
        waitingTasks[taskId] = false;
        P_SEM_S(*threads[id]);
        lockAll.unlock();
        return taskId;
    }
    if (threads.size() >= mtxContext.maxCount) {
        lockAll.unlock();
        return taskId;
    }
    const u_count_t last = threads.size();
    // threads.push_back(ThreadContext());
    threads.push_back(new ThreadContext);
    initThread(*threads[last], last);
    mtxContext.threadCount++;
    threads[last]->mtxContext.task = task;
    task->threadId = last;
    status(last) = Working;
    threadIdly[last] = false;
    threadEmpty[last] = false;
    waitingTasks[taskId] = false;
    P_SEM_S(*threads[last]);
    lockAll.unlock();
    return taskId;
}

bool ThreadPool::addThreadFromWaitingArrayLocked(const u_count_t current_id) {
    const count_t id = waitingTasks.findNextTrue(0, mtxContext.taskArray.size());
    if (id == BITMAP_NOT_FOUND) { return false; }
    waitingTasks[id] = false;
    auto *task = mtxContext.taskArray[id];
    task->threadId = current_id;
    threads[current_id]->mtxContext.task = task;
    return true;
}

bool ThreadPool::isDescriptorAvailable(const task_descriptor_t descriptor) {
    return !emptyTasks[descriptor];
}

void *ThreadPool::waitThread(const task_descriptor_t index) {
    lockAll.reader().lock();
    CHECK_THROW_TERM_TH("wait", lockAll.reader().unlock());
    if (index >= mtxContext.taskArray.size()) {
        lockAll.reader().unlock();
        return nullptr;
    }
    lockTask(index);
    if (!isDescriptorAvailable(index)) {
        unlockTask(index);
        lockAll.reader().unlock();
        return nullptr;
    }
    if (!mtxContext.taskArray[index]->wait) {
        unlockTask(index);
        lockAll.reader().unlock();
        return nullptr;
    }
    const auto &task = mtxContext.taskArray[index];
    void *ret;
    if (task->finished) {
        ret = task->param_return;
        W_SEM_F(*task);
        emptyTasks[index] = true;
        unlockTask(index);
        lockAll.reader().unlock();
        return ret;
    }
    task->refers++;
    unlockTask(index);
    lockAll.reader().unlock();
    W_SEM_F(*task);
    ret = task->param_return;
    lockAll.reader().lock();
    lockTask(index);
    task->refers--;
    if (task->refers == 0) {
        emptyTasks[index] = true;
    }
    unlockTask(index);
    lockAll.reader().unlock();
    return ret;
}

bool ThreadPool::destroyThread(const u_count_t index, const bool onlyIdly) {
    if (index >= threads.size()) { return false; }
    lockAll.reader().lock();
    lockThread(index);
    CHECK_THROW_TERM_TH("destroy", unlockThread(index), lockAll.reader().unlock());
    const bool ret = destroyThreadLocked(index, onlyIdly);
    unlockThread(index);
    lockAll.reader().unlock();
    return ret;
}

bool ThreadPool::destroyThreadLocked(const u_count_t index, const bool onlyIdly) const {
    if (index >= threads.size()) { return false; }
    if (status(index) == Empty) {
        return false;
    }
    if (onlyIdly && status(index) != Idly) {
        return false;
    }
    if (terminate(index))
        return true;
    terminate(index) = true;
    P_SEM_S(*threads[index]);
    return true;
}

void ThreadPool::reduceTo(const s_count_t tar, const bool force) {
    const auto lockAll = this->lockAll.writer();
    lockAll.lock();
    CHECK_THROW_TERM("reduce threads in", lockAll.unlock());
    if (tar < 0 || tar >= mtxContext.threadCount) {
        lockAll.unlock();
        return;
    }
    mtxContext.reduce = tar;
    u_count_t deduct = mtxContext.threadCount - tar;
    for (u_count_t i = 0; deduct != 0 && i < threads.size(); i++) {
        if (destroyThreadLocked(i, !force)) {
            deduct--;
        }
    }
    lockAll.unlock();
}

void ThreadPool::setMax(const u_count_t max) {
    lockAll.writer().lock();
    mtxContext.maxCount = max;
    mtxContext.reduce = -1;
    while (mtxContext.waitReduceRefers > 0) {
        P_SEM(reduceWaitSem);
        mtxContext.waitReduceRefers--;
    }
    lockAll.writer().unlock();
}

void ThreadPool::waitReduce() {
    const auto lockAll = this->lockAll.writer();
    lockAll.lock();
    CHECK_THROW_TERM("wait reduce from", lockAll.unlock());
    if (mtxContext.reduce < 0 || mtxContext.reduce >= mtxContext.threadCount) {
        lockAll.unlock();
        return;
    }
    u_count_t deduct = mtxContext.threadCount - mtxContext.reduce;
    for (u_count_t i = 0; deduct != 0 && i < threads.size(); i++) {
        if (destroyThreadLocked(i, true)) {
            deduct--;
        }
    }
    mtxContext.waitReduceRefers++;
    lockAll.unlock();
    W_SEM(reduceWaitSem);
}

void ThreadPool::waitFinish() {
    const auto lockAll = this->lockAll.writer();
    lockAll.lock();
    CHECK_THROW_TERM("wait finish of all threads from", lockAll.unlock());
    if (mtxContext.idlyCount == mtxContext.threadCount) {
        lockAll.unlock();
        return;
    }
    mtxContext.waitFinishRefers++;
    lockAll.unlock();
    W_SEM(finishWaitSem);
}

ThreadPool::u_count_t ThreadPool::getNumThreads() const {
    CHECK_THROW_TERM("operate");
    return mtxContext.threadCount;
}

ThreadPool::u_count_t ThreadPool::getIdlyThreads() const {
    CHECK_THROW_TERM("operate");
    return mtxContext.idlyCount;
}

ThreadPool::u_count_t ThreadPool::getMaxThreads() const {
    CHECK_THROW_TERM("operate");
    return mtxContext.maxCount;
}

ThreadPool::u_count_t ThreadPool::getIdx() {
    static u_count_t idx = 0;
    static Mutex mutex;
    mutex.lock();
    idx++;
    const u_count_t ret = idx - 1;
    mutex.unlock();
    return ret;
}

void *ThreadPool::threadFunc(void *arg) {
    ThreadContext &thread = *static_cast<ThreadContext *>(arg);
    ThreadPool &pool = *thread.pool;
    ThreadMtx &tMtx = thread.mtxContext;
    PoolMtx &pMtx = pool.mtxContext;
    while (true) {
        W_SEM_S(thread);
        // Check if terminate
        pool.lockAll.reader().lock();
        pMtx.mtx.lock();
        tMtx.mtx.lock();
        if (tMtx.task == nullptr && (pMtx.termAll
                                     || tMtx.forceTerminate
                                     || (pMtx.reduce != -1 && pMtx.reduce < pMtx.threadCount))) {
            // Do terminate
        DO_TERMINATE:
            resetThread(tMtx);
            tMtx.status = Empty;
            pool.threadIdly[thread.id] = false;
            pool.threadEmpty[thread.id] = true;
            pMtx.idlyCount--;
            pMtx.threadCount--;
            // If reduceWaited
            if (pMtx.reduce != -1 && pMtx.reduce >= pMtx.threadCount) {
                while (pMtx.waitReduceRefers > 0) {
                    P_SEM(pool.reduceWaitSem);
                    pMtx.waitReduceRefers--;
                }
                pMtx.waitReduceRefers = 0, pMtx.reduce = -1;
            }
            // If termAll
            if (pMtx.threadCount == 0 && pMtx.termAll) {
                tMtx.mtx.unlock();
                pMtx.mtx.unlock();
                pool.lockAll.reader().unlock();
                pool.lockAll.writer().lock();
                P_SEM(pool.poolTermSem);
                return nullptr;
            }
            tMtx.mtx.unlock();
            pMtx.mtx.unlock();
            pool.lockAll.reader().unlock();
            return nullptr;
        }
        // tMtx.status = Working;
    QUICK_START:
        tMtx.mtx.unlock();
        pMtx.mtx.unlock();
        pool.lockAll.reader().unlock();
        // perform function
        tMtx.task->param_return = tMtx.task->func(tMtx.task->param_return);
        pool.lockAll.reader().lock();
        pMtx.mtx.lock();
        tMtx.mtx.lock();
        tMtx.task->mtx.lock();
        tMtx.task->finished = true;
        // if this task has already waited by some threads, no matter if this task need wait,
        // this task will be freed
        if (tMtx.task->refers > 0) {
            // notify threads
            // task->refers will be reduced in method `waitThread`
            auto tmp = tMtx.task->refers;
            while (tmp > 0) {
                P_SEM_F(*tMtx.task);
                tmp--;
            }
        } else if (tMtx.task->wait) {
            P_SEM_F(*tMtx.task);
        }
        tMtx.task->mtx.unlock();
        if (tMtx.forceTerminate || (pMtx.reduce != -1 && pMtx.reduce < pMtx.threadCount)) {
            pMtx.idlyCount++;
            pool.wakeFinishSem();
            goto DO_TERMINATE;
        }
        if (!tMtx.forceTerminate && pool.addThreadFromWaitingArrayLocked(thread.id)) {
            // if this task is redistributed
            goto QUICK_START;
        }
        tMtx.status = Idly;
        pool.threadIdly[thread.id] = true;
        pMtx.idlyCount++;
        pool.wakeFinishSem();
        resetThread(tMtx);
        tMtx.mtx.unlock();
        pMtx.mtx.unlock();
        pool.lockAll.reader().unlock();
    }
}

void ThreadPool::startThread(const u_count_t id) {
    if (status(id) != Empty) { return; }
    status(id) = Idly;
    threadEmpty[id] = false;
    threadIdly[id] = true;
    pthread_create(&threads[id]->thread, nullptr, threadFunc, threads[id]);
    pthread_detach(threads[id]->thread);
}

void ThreadPool::initThread(ThreadContext &th, const u_count_t id) {
    th.id = id;
    th.pool = this;
    th.mtxContext.task = nullptr;
    th.mtxContext.status = Empty;
    th.mtxContext.forceTerminate = false;
    startThread(id);
}

void ThreadPool::resetThread(ThreadMtx &tMtx) {
    tMtx.task = nullptr;
    // tMtx.param_return = nullptr;
    tMtx.forceTerminate = false;
}

volatile u8 &ThreadPool::status(const u_count_t id) const {
    return threads[id]->mtxContext.status;
}

volatile bool &ThreadPool::terminate(const u_count_t id) const {
    return threads[id]->mtxContext.forceTerminate;
}

void ThreadPool::lockThread(const u_count_t id) const {
    threads[id]->mtxContext.mtx.lock();
}

void ThreadPool::unlockThread(const u_count_t id) const {
    threads[id]->mtxContext.mtx.unlock();
}

void ThreadPool::lockTask(const task_descriptor_t id) const {
    mtxContext.taskArray[id]->mtx.lock();
}

void ThreadPool::unlockTask(const task_descriptor_t id) const {
    mtxContext.taskArray[id]->mtx.unlock();
}

void ThreadPool::wakeFinishSem() {
    if (mtxContext.waitFinishRefers > 0 && mtxContext.idlyCount == mtxContext.threadCount) {
        while (mtxContext.waitFinishRefers > 0) {
            P_SEM(finishWaitSem);
            mtxContext.waitFinishRefers--;
        }
    }
}
