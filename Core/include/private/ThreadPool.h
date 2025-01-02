//
// Created by Fx Kaze on 24-12-30.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <vector>
#include "types.h"

class FastBitmap {
public:
    class element {
    public:
        element(uptr &data, const u8 offset) : data(&data), offset(offset) {
        };

        element(const element &other) = delete;

        ~element() = default;

        operator bool() const {
            return static_cast<bool>(*data & 0x1 << offset);
        }

        bool operator=(const bool val) const {
            if (val) {
                *data |= 0x1 << offset;
            } else {
                *data &= ~(0x1 << offset);
            }
            return val;
        }

    private:
        uptr *data;
        u8 offset;
    };

    using u_size_t = u64;

    FastBitmap(const u_size_t initBits = 128): bitmap(
        initBits / (8 * sizeof(uptr)) + (initBits % (8 * sizeof(uptr))
                                             ? 1
                                             : 0),
        0x0) {
        max = bitmap.size() * 8 * sizeof(uptr);
    }

    ~FastBitmap() = default;

    FastBitmap(const FastBitmap &other) = default;

    FastBitmap &operator=(const FastBitmap &other) = default;

    element operator[](u_size_t index) {
        u_size_t idx = index / (8 * sizeof(uptr));
        u_size_t offset = index % (8 * sizeof(uptr));
        if (index >= max) {
            const u_size_t nNew = idx - bitmap.size() + 1;
            bitmap.insert(bitmap.end(), nNew, 0x0);
            max += nNew * 8 * sizeof(uptr);
        }
        return element(bitmap[idx], offset);
    }

private:
    u_size_t max;
    std::vector<uptr> bitmap;
};

class Mutex {
public:
    Mutex() {
        mtxId = getIdx();
        rk_sema_init(&sem, 1);
        // ct = 1;
        // printf("[mutex]id: %zu, op: init, value: %d, addr: %p\n", mtxId, ct, this);
    }

    ~Mutex();

    void lock() {
        rk_sema_wait(&sem);
        // ct--;
        // printf("[mutex]id: %zu, op: lock, value: %d, addr: %p\n", mtxId, ct, this);
    }

    void unlock() {
        // ct++;
        // printf("[mutex]id: %zu, op: unlock, value: %d, addr: %p\n", mtxId, ct, this);
        rk_sema_post(&sem);
    }

    Mutex(const Mutex &other) = delete;

private:
    // u8 ct;
    u64 mtxId;
    rk_sema sem;

    static FastBitmap bitmap;

    u64 static getIdx();
};

// using namespace std;

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
        rk_sema semaStart;
        rk_sema semaFinish;
        pthread_t thread;
        ThreadMtx mtxContext;
    } ThreadContext;

    typedef struct {
        u_count_t threadCount;
        u_count_t waitRefers;
        s_count_t reduce;
        bool termAll;
        Mutex mtx;
    } PoolMtx;

    u_count_t idx;
    rk_sema poolTermSem, reduceWaitSem;
    PoolMtx mtxContext;
    Mutex lockAll;

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

    bool destroyThreadLocked(thread_descriptor_t index, bool onlyIdly = false) const;
};

#endif //THREAD_POOL_H
