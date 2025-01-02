//
// Created by Fx Kaze on 24-12-30.
//

#include "private/ThreadPool.h"

#include <mutex>

#define W_SEM_S(t) rk_sema_wait(&(t).semaStart)
#define W_SEM_F(t) rk_sema_wait(&(t).semaFinish)
#define P_SEM_S(t) rk_sema_post(&(t).semaStart)
#define P_SEM_F(t) rk_sema_post(&(t).semaFinish)

#define W_SEM(t) rk_sema_wait(&(t))
#define P_SEM(t) rk_sema_post(&(t))

#define C_SEM(t) rk_sema_close(&(t))

static std::mutex t_mtx;

FastBitmap Mutex::bitmap;

Mutex::~Mutex() {
    t_mtx.lock();
    bitmap[mtxId] = false;
    t_mtx.unlock();
    rk_sema_close(&sem);
}

u64 Mutex::getIdx() {
    static u64 idx = 0;
    t_mtx.lock();
    for (u64 i = 0; i < idx; i++) {
        if (!bitmap[i]) {
            bitmap[i] = true;
            t_mtx.unlock();
            return i;
        }
    }
    idx++;
    const u64 ret = idx - 1;
    bitmap[ret] = true;
    t_mtx.unlock();
    return ret;
}

ThreadPool::ThreadPool(const u_count_t nThreads): idx(getIdx()),
                                                  threads(nThreads) {
    mtxContext.threadCount = nThreads;
    mtxContext.waitRefers = 0;
    mtxContext.reduce = -1;
    mtxContext.termAll = false;
    for (u_count_t i = 0; i < nThreads; i++) {
        threads[i] = new ThreadContext;
        initThread(*threads[i], i);
    }
    rk_sema_init(&poolTermSem, 0);
    rk_sema_init(&reduceWaitSem, 0);
}

ThreadPool::~ThreadPool() {
    lockAll.lock();
    mtxContext.termAll = true;
    for (u_count_t i = 0; i < threads.size(); i++) {
        if (status(i) != Empty) {
            destroyThreadLocked(i);
        }
    }
    lockAll.unlock();
    W_SEM(poolTermSem);
    for (u_count_t i = 0; i < threads.size(); i++) {
        delete threads[i];
        C_SEM(threads[i]->semaStart);
        C_SEM(threads[i]->semaFinish);
    }
    C_SEM(reduceWaitSem);
    C_SEM(poolTermSem);
}

ThreadPool::u_count_t ThreadPool::addThread(void *(fun)(void *), void *param, const bool wait) {
    lockAll.lock();
    if (mtxContext.termAll) {
        lockAll.unlock();
        return ~static_cast<u_count_t>(0x0);
    }
    for (u_count_t i = 0; i < threads.size(); i++) {
        if (status(i) == Idly) {
            function(i) = fun;
            paramReturn(i) = param;
            this->wait(i) = wait;
            status(i) = Working;
            P_SEM_S(*threads[i]);
            lockAll.unlock();
            return i;
        }
    }
    for (u_count_t i = 0; i < threads.size(); i++) {
        if (status(i) == Empty) {
            mtxContext.threadCount++;
            function(i) = fun;
            paramReturn(i) = param;
            this->wait(i) = wait;
            status(i) = Working;
            startThread(i);
            P_SEM_S(*threads[i]);
            lockAll.unlock();
            return i;
        }
    }
    const u_count_t last = threads.size();
    // threads.push_back(ThreadContext());
    threads.push_back(new ThreadContext);
    initThread(*threads[last], last);
    mtxContext.threadCount++;
    function(last) = fun;
    paramReturn(last) = param;
    this->wait(last) = wait;
    status(last) = Working;
    P_SEM_S(*threads[last]);
    lockAll.unlock();
    return last;
}

void *ThreadPool::waitThread(const thread_descriptor_t index) {
    if (index >= threads.size()) { return nullptr; }
    lockAll.lock();
    lock(index);
    if (!wait(index)) {
        unlock(index);
        lockAll.unlock();
        return nullptr;
    }
    void *ret;
    if (status(index) & (Idly | Empty)) {
        unlock(index);
        lockAll.unlock();
        return nullptr;
    }
    if (status(index) == Returning) {
        W_SEM_F(*threads[index]);
        status(index) = Idly;
        ret = paramReturn(index);
        unlock(index);
        lockAll.unlock();
        return ret;
    }
    refers(index)++;
    unlock(index);
    lockAll.unlock();
    W_SEM_F(*threads[index]);
    ret = paramReturn(index);
    return ret;
}

bool ThreadPool::destroyThread(const thread_descriptor_t index, const bool onlyIdly) {
    if (index >= threads.size()) { return false; }
    lockAll.lock();
    lock(index);
    const bool ret = destroyThreadLocked(index, onlyIdly);
    unlock(index);
    lockAll.unlock();
    return ret;
}

bool ThreadPool::destroyThreadLocked(const thread_descriptor_t index, const bool onlyIdly) const {
    if (index >= threads.size()) { return false; }
    if (status(index) == Empty) {
        return false;
    }
    if (onlyIdly && status(index) != Idly) {
        return false;
    }
    terminate(index) = true;
    if (status(index) == Returning) {
        W_SEM_F(*threads[index]);
        status(index) = Idly;
    } else if (status(index) == Working) {
        wait(index) = false;
    }
    P_SEM_S(*threads[index]);
    return true;
}

void ThreadPool::reduceTo(const s_count_t tar, const bool force) {
    lockAll.lock();
    if (tar < 0 || tar >= mtxContext.threadCount) {
        lockAll.unlock();
        return;
    }
    mtxContext.reduce = tar;
    u_count_t deduct = mtxContext.threadCount - tar;
    for (u_count_t i = 0; i < threads.size(); i++) {
        if (destroyThreadLocked(i, !force)) {
            deduct--;
            if (deduct == 0) {
                break;
            }
        }
    }
    lockAll.unlock();
}

void ThreadPool::waitReduce() {
    lockAll.lock();
    if (mtxContext.reduce < 0 || mtxContext.reduce >= mtxContext.threadCount) {
        lockAll.unlock();
        return;
    }
    u_count_t deduct = mtxContext.threadCount - mtxContext.reduce;
    for (u_count_t i = 0; i < threads.size(); i++) {
        if (destroyThreadLocked(i, true)) {
            deduct--;
            if (deduct == 0) {
                break;
            }
        }
    }
    mtxContext.waitRefers++;
    lockAll.unlock();
    W_SEM(reduceWaitSem);
}

ThreadPool::u_count_t ThreadPool::getNumThreads() const {
    return mtxContext.threadCount;
}

ThreadPool::u_count_t ThreadPool::getIdlyThreads() {
    u_count_t ret = 0;
    lockAll.lock();
    for (u_count_t i = 0; i < threads.size(); i++) {
        if (status(i) == Idly) {
            ret++;
        }
    }
    lockAll.unlock();
    return ret;
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
        pool.lockAll.lock();
        pMtx.mtx.lock();
        tMtx.mtx.lock();
        if (pMtx.termAll
            || tMtx.forceTerminate
            || (tMtx.func != nullptr && pMtx.reduce != -1 && pMtx.reduce < pMtx.threadCount)) {
            resetThread(tMtx);
            // Do terminate
        DO_TERMINATE:
            tMtx.status = Empty;
            pMtx.threadCount--;
            // If reduceWaited
            if (pMtx.reduce != -1 && pMtx.reduce >= pMtx.threadCount) {
                while (pMtx.waitRefers > 0) {
                    P_SEM(pool.reduceWaitSem);
                    pMtx.waitRefers--;
                }
                pMtx.waitRefers = 0, pMtx.reduce = -1;
            }
            // If termAll
            if (pMtx.threadCount == 0 && pMtx.termAll) {
                tMtx.mtx.unlock();
                pMtx.mtx.unlock();
                pool.lockAll.unlock();
                P_SEM(pool.poolTermSem);
                return nullptr;
            }
            tMtx.mtx.unlock();
            pMtx.mtx.unlock();
            pool.lockAll.unlock();
            return nullptr;
        }
        // tMtx.status = Working;
        tMtx.mtx.unlock();
        pMtx.mtx.unlock();
        pool.lockAll.unlock();
        tMtx.param_return = tMtx.func(tMtx.param_return);
        pool.lockAll.lock();
        tMtx.mtx.lock();
        if (tMtx.refers > 0) {
            tMtx.status = Idly;
            while (tMtx.refers > 0) {
                P_SEM_F(thread);
                tMtx.refers--;
            }
            tMtx.refers = 0;
        } else if (tMtx.waitNeed) {
            tMtx.status = Returning;
            P_SEM_F(thread);
        } else {
            tMtx.status = Idly;
        }
        resetThread(tMtx);
        pMtx.mtx.lock();
        if (tMtx.forceTerminate || (pMtx.reduce != -1 && pMtx.reduce < pMtx.threadCount)) {
            goto DO_TERMINATE;
        }
        pMtx.mtx.unlock();
        tMtx.mtx.unlock();
        pool.lockAll.unlock();
    }
}

void ThreadPool::startThread(const u_count_t id) const {
    if (status(id) != Empty) { return; }
    status(id) = Idly;
    pthread_create(&threads[id]->thread, nullptr, threadFunc, threads[id]);
    pthread_detach(threads[id]->thread);
}

void ThreadPool::initThread(ThreadContext &th, const u_count_t id) {
    th.id = id;
    th.pool = this;
    rk_sema_init(&th.semaStart, 0);
    rk_sema_init(&th.semaFinish, 0);
    th.mtxContext.func = nullptr;
    th.mtxContext.param_return = nullptr;
    th.mtxContext.refers = 0;
    th.mtxContext.status = Empty;
    th.mtxContext.forceTerminate = false;
    th.mtxContext.waitNeed = true;
    startThread(id);
}

void ThreadPool::resetThread(ThreadMtx &tMtx) {
    tMtx.func = nullptr;
    tMtx.refers = 0;
    // tMtx.param_return = nullptr;
    tMtx.forceTerminate = false;
    tMtx.waitNeed = true;
}

u8 &ThreadPool::status(const u_count_t id) const {
    return threads[id]->mtxContext.status;
}

bool &ThreadPool::terminate(const u_count_t id) const {
    return threads[id]->mtxContext.forceTerminate;
}

bool &ThreadPool::wait(const u_count_t id) const {
    return threads[id]->mtxContext.waitNeed;
}

ThreadPool::funcHandler &ThreadPool::function(const u_count_t id) const {
    return threads[id]->mtxContext.func;
}

void ThreadPool::lock(const u_count_t id) const {
    threads[id]->mtxContext.mtx.lock();
}

void ThreadPool::unlock(const u_count_t id) const {
    threads[id]->mtxContext.mtx.unlock();
}

void *&ThreadPool::paramReturn(const u_count_t id) const {
    return threads[id]->mtxContext.param_return;
}

ThreadPool::u_count_t &ThreadPool::refers(const u_count_t id) const {
    return threads[id]->mtxContext.refers;
}
