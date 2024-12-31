//
// Created by Fx Kaze on 24-12-30.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(size_t nThreads);

    ThreadPool(const ThreadPool &) = delete;

    ~ThreadPool();

    size_t addThread(void *(function)(void *), void *param, bool wait = true);

    void *waitThread(size_t handle);

    bool destroyThread(size_t handle, bool forceIdly = false);

    void reduceTo(int32_t tar, bool force = false);

    void waitReduce();

    size_t getNumThreads() const;

    size_t getIdlyThreads() const;

    class Semaphore {
    public:
        explicit Semaphore(const int32_t initialCount = 0)
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
        int32_t count;
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
        size_t id;
    } ThreadInfo;

    size_t threadCount;
    size_t idx;
    int32_t reduce;
    std::vector<pthread_t> threads;
    std::vector<Semaphore *> semaphoreStart;
    std::vector<Semaphore *> semaphoreFinish;
    std::vector<bool> terminate;
    std::vector<bool> wait;
    std::vector<void *(*)(void *)> func;
    std::vector<void *> param_return;
    std::vector<uint8_t> status;
    std::vector<uint32_t> refers;
    // When modifying thread status
    std::vector<std::mutex *> mutexes;
    Semaphore *finalSignal, *waitSignal;
    int32_t waitRefer;
    // When modifying thread counter
    std::mutex countMutex;
    bool termAll;

    static size_t getIdx();

    static void *threadFunc(void *arg);

    bool destroyThreadLocked(size_t handle, bool forceIdly = false);
};

#endif //THREAD_POOL_H
