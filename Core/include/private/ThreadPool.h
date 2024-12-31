//
// Created by Fx Kaze on 24-12-30.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <vector>
#include "types.h"

class ThreadPool {
public:
    using u_count_t = u32;
    using s_count_t = s32;
    using handler_t = u_count_t;

    explicit ThreadPool(u_count_t nThreads);

    ThreadPool(const ThreadPool &) = delete;

    ~ThreadPool();

    handler_t addThread(void *(function)(void *), void *param, bool wait = true);

    void *waitThread(handler_t handle);

    bool destroyThread(handler_t handle, bool forceIdly = false);

    void reduceTo(s_count_t tar, bool force = false);

    void waitReduce();

    u_count_t getNumThreads() const;

    u_count_t getIdlyThreads() const;

    class Semaphore {
    public:
        explicit Semaphore(const u_count_t initialCount = 0)
            : count(initialCount) {
        }

        void wait() {
            std::unique_lock lock(mtx);
            while (count == 0) {
                cond.wait(lock);
            }
            --count;
        }

        void post() {
            std::lock_guard lock(mtx);
            ++count;
            cond.notify_one();
        }

    private:
        u_count_t count;
        std::mutex mtx;
        std::condition_variable cond;
    };

private:
    typedef enum {
        Idly = 0x1,
        Working = 0x2,
        Returning = 0x4,
        Empty = 0x8,
    } ThreadStatus;

    typedef struct {
        ThreadPool *pool;
        u_count_t id;
    } ThreadInfo;

    u_count_t threadCount;
    u_count_t idx;
    s_count_t reduce;
    std::vector<pthread_t> threads;
    std::vector<Semaphore *> semaphoreStart;
    std::vector<Semaphore *> semaphoreFinish;
    std::vector<bool> terminate;
    std::vector<bool> wait;
    std::vector<void *(*)(void *)> func;
    std::vector<void *> param_return;
    std::vector<u8> status;
    std::vector<u_count_t> refers;
    // When modifying thread status
    std::vector<std::mutex *> mutexes;
    Semaphore *finalSignal, *waitSignal;
    u_count_t waitRefer;
    // When modifying thread counter
    std::mutex countMutex;
    bool termAll;

    static u_count_t getIdx();

    static void *threadFunc(void *arg);

    bool destroyThreadLocked(handler_t handle, bool forceIdly = false);
};

#endif //THREAD_POOL_H
