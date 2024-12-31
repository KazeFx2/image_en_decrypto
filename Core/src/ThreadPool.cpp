//
// Created by Fx Kaze on 24-12-30.
//

#include "private/ThreadPool.h"

// ThreadPool::Semaphore *sem_open(const char *, int, int, const int n) {
//     return new ThreadPool::Semaphore(n);
// }

inline void sem_post(ThreadPool::Semaphore *sem) {
    sem->post();
}

inline void sem_wait(ThreadPool::Semaphore *sem) {
    sem->wait();
}

inline void sem_close(const ThreadPool::Semaphore *sem) {
    delete sem;
}

ThreadPool::ThreadPool(const u_count_t nThreads): threadCount(nThreads), reduce(-1), threads(nThreads),
                                                  semaphoreStart(nThreads), semaphoreFinish(nThreads),
                                                  terminate(nThreads, false), wait(nThreads, true),
                                                  func(nThreads, nullptr), param_return(nThreads, nullptr),
                                                  status(nThreads, Idly), refers(nThreads, 0), mutexes(nThreads),
                                                  waitRefer(0), termAll(false) {
    // char tmp[64];
    idx = getIdx();
    for (u_count_t i = 0; i < nThreads; i++) {
        mutexes[i] = new std::mutex;
        // snprintf(tmp, sizeof(tmp), "%zu_sem_start_%zu", idx, i);
        // semaphoreStart[i] = sem_open(tmp, O_CREAT, S_IRUSR | S_IWUSR, 0);
        // snprintf(tmp, sizeof(tmp), "%zu_sem_finish_%zu", idx, i);
        // semaphoreFinish[i] = sem_open(tmp, O_CREAT, S_IRUSR | S_IWUSR, 0);
        semaphoreStart[i] = new Semaphore(0);
        semaphoreFinish[i] = new Semaphore(0);
        ThreadInfo *info = new ThreadInfo{
            this,
            i
        };
        pthread_create(&threads[i], nullptr, threadFunc, info);
        pthread_detach(threads[i]);
    }
    for (u_count_t i = 0; i < nThreads; i++) {
        sem_wait(semaphoreFinish[i]);
    }
    // snprintf(tmp, sizeof(tmp), "%zu_sem_fin", idx);
    // finalSignal = sem_open(tmp, O_CREAT, S_IRUSR | S_IWUSR, 0);
    // snprintf(tmp, sizeof(tmp), "%zu_sem_wait", idx);
    // waitSignal = sem_open(tmp, O_CREAT, S_IRUSR | S_IWUSR, 0);
    finalSignal = new Semaphore(0);
    waitSignal = new Semaphore(0);
}

ThreadPool::~ThreadPool() {
    termAll = true;
    for (u_count_t i = 0; i < threads.size(); i++) {
        mutexes[i]->lock();
        if (status[i] != Empty) {
            destroyThreadLocked(i);
        }
        mutexes[i]->unlock();
    }
    sem_wait(finalSignal);
    sem_close(finalSignal);
    sem_close(waitSignal);
    for (u_count_t i = 0; i < threads.size(); i++) {
        delete mutexes[i];
    }
}

ThreadPool::u_count_t ThreadPool::addThread(void *(function)(void *), void *param, const bool wait) {
    // char tmp[64];
    ThreadInfo *info;
    for (u_count_t i = 0; i < threads.size(); i++) {
        mutexes[i]->lock();
        if (status[i] == Idly) {
            this->func[i] = function;
            param_return[i] = param;
            status[i] = Working;
            refers[i] = 0;
            this->wait[i] = wait;
            sem_post(semaphoreStart[i]);
            mutexes[i]->unlock();
            return i;
        }
        mutexes[i]->unlock();
    }
    for (u_count_t i = 0; i < threads.size(); i++) {
        mutexes[i]->lock();
        if (status[i] == Empty) {
            terminate[i] = false;
            // snprintf(tmp, sizeof(tmp), "%zu_sem_start_%zu", idx, i);
            // semaphoreStart[i] = sem_open(tmp, O_CREAT, S_IRUSR | S_IWUSR, 0);
            // snprintf(tmp, sizeof(tmp), "%zu_sem_finish_%zu", idx, i);
            // semaphoreFinish[i] = sem_open(tmp, O_CREAT, S_IRUSR | S_IWUSR, 0);
            semaphoreStart[i] = new Semaphore(0);
            semaphoreFinish[i] = new Semaphore(0);
            info = new ThreadInfo{
                this,
                i
            };
            pthread_create(&threads[i], nullptr, threadFunc, info);
            pthread_detach(threads[i]);
            sem_wait(semaphoreFinish[i]);
            countMutex.lock();
            if (threadCount == 0)
                sem_wait(finalSignal);
            threadCount++;
            countMutex.unlock();
            this->func[i] = function;
            param_return[i] = param;
            this->wait[i] = wait;
            status[i] = Working;
            refers[i] = 0;
            sem_post(semaphoreStart[i]);
            mutexes[i]->unlock();
            return i;
        }
        mutexes[i]->unlock();
    }
    const u_count_t last = threads.size();
    // snprintf(tmp, sizeof(tmp), "%zu_sem_start_%zu", idx, last);
    // semaphoreStart.push_back(sem_open(tmp, O_CREAT, S_IRUSR | S_IWUSR, 0));
    // snprintf(tmp, sizeof(tmp), "%zu_sem_finish_%zu", idx, last);
    // semaphoreFinish.push_back(sem_open(tmp, O_CREAT, S_IRUSR | S_IWUSR, 0));
    semaphoreStart.push_back(new Semaphore(0));
    semaphoreFinish.push_back(new Semaphore(0));
    terminate.push_back(false);
    func.push_back(function);
    param_return.push_back(param);
    this->wait.push_back(wait);
    status.push_back(Working);
    refers.push_back(0);
    mutexes.push_back(new std::mutex);
    threads.push_back(nullptr);
    info = new ThreadInfo{
        this,
        last
    };
    pthread_create(&threads[last], nullptr, threadFunc, info);
    pthread_detach(threads[last]);
    sem_wait(semaphoreFinish[last]);
    countMutex.lock();
    if (threadCount == 0)
        sem_wait(finalSignal);
    threadCount++;
    countMutex.unlock();
    sem_post(semaphoreStart[last]);
    return last;
}

void *ThreadPool::waitThread(const handler_t handle) {
    if (handle >= threads.size()) { return nullptr; }
    if (!wait[handle]) { return nullptr; }
    void *ret;
    mutexes[handle]->lock();
    if (status[handle] & (Idly | Empty)) { return nullptr; }
    if (status[handle] == Returning) {
        sem_wait(semaphoreFinish[handle]);
        status[handle] = Idly;
        ret = param_return[handle];
        mutexes[handle]->unlock();
        return ret;
    }
    refers[handle]++;
    mutexes[handle]->unlock();
    sem_wait(semaphoreFinish[handle]);
    ret = param_return[handle];
    return ret;
}

bool ThreadPool::destroyThread(const handler_t handle, const bool forceIdly) {
    if (handle >= threads.size()) { return false; }
    mutexes[handle]->lock();
    const bool ret = destroyThreadLocked(handle, forceIdly);
    mutexes[handle]->unlock();
    return ret;
}

bool ThreadPool::destroyThreadLocked(const handler_t handle, const bool forceIdly) {
    if (handle >= threads.size()) { return false; }
    if (status[handle] == Empty) {
        return false;
    }
    if (forceIdly && status[handle] != Idly) {
        return false;
    }
    terminate[handle] = true;
    if (status[handle] == Returning) {
        sem_wait(semaphoreFinish[handle]);
        status[handle] = Idly;
    } else if (status[handle] == Working) {
        wait[handle] = false;
    }
    sem_post(semaphoreStart[handle]);
    return true;
}

void ThreadPool::reduceTo(const s_count_t tar, const bool force) {
    if (tar < 0 || tar >= threadCount) return;
    reduce = tar;
    s_count_t deduct = 0;
    for (u_count_t i = 0; i < threads.size(); i++) {
        if (status[i] == Idly) {
            if (destroyThread(i, !force)) {
                deduct++;
                if (reduce + deduct >= threadCount) {
                    return;
                }
            }
        }
    }
}

void ThreadPool::waitReduce() {
    countMutex.lock();
    if (reduce < 0 || reduce >= threadCount) {
        countMutex.unlock();
        return;
    }
    u_count_t deduct = threadCount - reduce;
    for (u_count_t i = 0; i < threads.size(); i++) {
        if (status[i] == Idly && terminate[i])
            sem_post(semaphoreStart[i]), deduct--;
    }
    if (deduct > 0)
        for (u_count_t i = 0; i < threads.size(); i++) {
            if (status[i] == Idly && !terminate[i]) {
                terminate[i] = true;
                sem_post(semaphoreStart[i]);
                deduct--;
                if (deduct <= 0)
                    break;
            }
        }
    waitRefer++;
    countMutex.unlock();
    sem_wait(waitSignal);
}

ThreadPool::u_count_t ThreadPool::getNumThreads() const {
    return threadCount;
}

ThreadPool::u_count_t ThreadPool::getIdlyThreads() const {
    u_count_t ret = 0;
    for (u_count_t i = 0; i < threads.size(); i++) {
        if (status[i] == Idly) {
            ret++;
        }
    }
    return ret;
}

ThreadPool::u_count_t ThreadPool::getIdx() {
    static u_count_t idx = 0;
    idx++;
    return idx - 1;
}

void *ThreadPool::threadFunc(void *arg) {
    ThreadInfo info = *static_cast<ThreadInfo *>(arg);
    sem_post(info.pool->semaphoreFinish[info.id]);
    delete static_cast<ThreadInfo *>(arg);
    while (true) {
        sem_wait(info.pool->semaphoreStart[info.id]);
        info.pool->countMutex.lock();
        info.pool->mutexes[info.id]->lock();
        if (!info.pool->termAll && !info.pool->terminate[info.id] && (
                info.pool->reduce == -1 || info.pool->reduce >= info.pool->
                threadCount)) {
            info.pool->mutexes[info.id]->unlock();
            info.pool->countMutex.unlock();
        } else {
            // info.pool->countMutex.lock();
            // info.pool->mutexes[info.id]->lock();
            info.pool->status[info.id] = Empty;
            sem_close(info.pool->semaphoreStart[info.id]);
            sem_close(info.pool->semaphoreFinish[info.id]);
            info.pool->threadCount--;
            if (info.pool->threadCount <= info.pool->reduce && info.pool->reduce != -1) {
                info.pool->reduce = -1;
                while (info.pool->waitRefer--)
                    sem_post(info.pool->waitSignal);
                info.pool->waitRefer = 0;
            }
            info.pool->terminate[info.id] = false;
            if (info.pool->threadCount == 0) {
                sem_post(info.pool->finalSignal);
            }
            info.pool->mutexes[info.id]->unlock();
            info.pool->countMutex.unlock();
            return nullptr;
        }
        void *ret = info.pool->func[info.id](info.pool->param_return[info.id]);
        info.pool->mutexes[info.id]->lock();
        if (!info.pool->wait[info.id]) {
            info.pool->param_return[info.id] = ret;
            info.pool->status[info.id] = Idly;
            while (info.pool->refers[info.id] > 0 && info.pool->refers[info.id]--)
                sem_post(info.pool->semaphoreFinish[info.id]);
        } else {
            info.pool->param_return[info.id] = ret;
            info.pool->status[info.id] = Returning;
            sem_post(info.pool->semaphoreFinish[info.id]);
            if (info.pool->refers[info.id] > 0) {
                while (--info.pool->refers[info.id])
                    sem_post(info.pool->semaphoreFinish[info.id]);
                info.pool->status[info.id] = Idly;
            }
        }
        info.pool->mutexes[info.id]->unlock();
    }
}
