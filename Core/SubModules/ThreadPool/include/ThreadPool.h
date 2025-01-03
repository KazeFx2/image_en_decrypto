//
// Created by Fx Kaze on 25-1-3.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "private/types.h"
#include "RWLock.h"
#include "pthread.h"
#include <vector>

class ThreadPool {
public:
    using u_count_t = u32;
    using s_count_t = s32;
    using thread_descriptor_t = u_count_t;

    typedef void *(*funcHandler)(void *);

    explicit ThreadPool(u_count_t nThreads);

    ThreadPool(const ThreadPool &) = delete;

    ~ThreadPool();

    thread_descriptor_t addThread(funcHandler fun, void *param, bool wait = true);

    void *waitThread(thread_descriptor_t index);

    bool destroyThread(thread_descriptor_t index, bool onlyIdly = false);

    void reduceTo(s_count_t tar, bool force = false);

    void waitReduce();

    void waitFinish();

    u_count_t getNumThreads() const;

    u_count_t getIdlyThreads();

private:
    typedef enum {
        Idly = 0x1,
        Working = 0x2,
        Returning = 0x4,
        Empty = 0x8,
    } ThreadStatus;

    typedef struct {
        u8 status;
        u_count_t refers;
        bool forceTerminate;
        bool waitNeed;
        funcHandler func;
        void *param_return;
        Mutex mtx;
    } ThreadMtx;

    typedef struct {
        ThreadPool *pool;
        u_count_t id;
        Semaphore semaStart;
        Semaphore semaFinish;
        pthread_t thread;
        ThreadMtx mtxContext;
    } ThreadContext;

    typedef struct {
        u_count_t threadCount;
        u_count_t idlyCount;
        u_count_t waitReduceRefers;
        u_count_t waitFinishRefers;
        s_count_t reduce;
        bool termAll;
        Mutex mtx;
    } PoolMtx;

    u_count_t idx;
    Semaphore poolTermSem, reduceWaitSem, finishWaitSem;
    PoolMtx mtxContext;
    RWLock lockAll;

    std::vector<ThreadContext *> threads;

    void startThread(u_count_t id) const;

    void initThread(ThreadContext &th, u_count_t id);

    u8 &status(u_count_t id) const;

    bool &terminate(u_count_t id) const;

    bool &wait(u_count_t id) const;

    funcHandler &function(u_count_t id) const;

    void lock(u_count_t id) const;

    void unlock(u_count_t id) const;

    void *&paramReturn(u_count_t id) const;

    u_count_t &refers(u_count_t id) const;

    static void resetThread(ThreadMtx &tMtx);

    static u_count_t getIdx();

    static void *threadFunc(void *arg);

    bool destroyThreadLocked(thread_descriptor_t index, bool allLocked = true, bool onlyIdly = false);

    void wakeFinishSem();
};


#endif //THREAD_POOL_H
