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

class Semaphore {
public:
    Semaphore(u32 init = 0) {
        rk_sema_init(&sem, init);
    }

    ~Semaphore() {
        rk_sema_close(&sem);
    }

    Semaphore(const Semaphore &other) = delete;

    Semaphore &operator=(const Semaphore &other) = delete;

    Semaphore(Semaphore &&other) = delete;

    Semaphore &operator=(Semaphore &&other) = delete;

    void wait() {
        rk_sema_wait(&sem);
    }

    void post() {
        rk_sema_post(&sem);
    }

private:
    rk_sema sem;
};

class Mutex {
public:
    Mutex() {
        mtxId = getIdx();
        rk_sema_init(&sem, 1);
#ifdef __DEBUG
        ct = 1;
        printf("[mutex]id: %zu, op: init, value: %d, addr: %p\n", mtxId, ct, this);
#endif
    }

    ~Mutex();

    void lock() {
        rk_sema_wait(&sem);
#ifdef __DEBUG
        ct--;
        printf("[mutex]id: %zu, op: lock, value: %d, addr: %p\n", mtxId, ct, this);
#endif
    }

    void unlock() {
#ifdef __DEBUG
        ct++;
        printf("[mutex]id: %zu, op: unlock, value: %d, addr: %p\n", mtxId, ct, this);
#endif
        rk_sema_post(&sem);
    }

    Mutex(const Mutex &other) = delete;

    Mutex &operator=(const Mutex &other) = delete;

    Mutex(Mutex &&other) = delete;

    Mutex &operator=(Mutex &&other) = delete;

private:
#ifdef __DEBUG
    u8 ct;
#endif
    u64 mtxId;
    rk_sema sem;

    static FastBitmap bitmap;

    u64 static getIdx();
};

class RWLock {
public:
    class SubLock {
    public:
        SubLock(RWLock *p, void (RWLock::*lock)(), void (RWLock::*unlock)()): parent(p), lockHandler(lock),
                                                                              unlockHandler(unlock) {
        }

        ~SubLock() = default;

        void lock() const { (parent->*lockHandler)(); }

        void unlock() const { (parent->*unlockHandler)(); }

    private:
        RWLock *parent;

        void (RWLock::*lockHandler)();

        void (RWLock::*unlockHandler)();
    };

    RWLock(const u8 Strategy = WriterFist): strategy(Strategy), rCount(0), wCount(0), rWait(0), wWait(0), ReaderSem(0),
                                            WriterSem(0) {
    }

    ~RWLock() {
    }

    typedef enum {
        WriterFist = 0x0,
        ReaderFist = 0x1
    } RW_Strategy;

    RWLock(const RWLock &other) = delete;

    RWLock &operator=(const RWLock &other) = delete;

    RWLock(RWLock &&other) = delete;

    RWLock &operator=(RWLock &&other) = delete;

    SubLock reader() {
        return SubLock(this, &RWLock::ReaderLock, &RWLock::ReaderUnlock);
    }

    SubLock writer() {
        return SubLock(this, &RWLock::WriterLock, &RWLock::WriterUnlock);
    }

    void ReaderLock() {
        bool onWait = false;
        mtx.lock();
        switch (strategy) {
            case WriterFist:
                if (wCount > 0 || wWait > 0) {
                    rWait++;
                    onWait = true;
                } else
                    rCount++;
                break;
            case ReaderFist:
            default:
                if (wCount > 0) {
                    rWait++;
                    onWait = true;
                } else
                    rCount++;
                break;
        }
        mtx.unlock();
        if (onWait) {
            ReaderSem.wait();
        }
    }

    void ReaderUnlock() {
        mtx.lock();
        rCount--;
        if (rCount == 0 && wWait > 0) {
            wWait--;
            wCount++;
            WriterSem.post();
        }
        mtx.unlock();
    }

    void WriterLock() {
        bool onWait = false;
        mtx.lock();
        switch (strategy) {
            case WriterFist:
                if (rCount > 0 || wCount > 0) {
                    wWait++;
                    onWait = true;
                } else
                    wCount++;
                break;
            case ReaderFist:
            default:
                if (rCount > 0 || wCount > 0 || rWait > 0) {
                    wWait++;
                    onWait = true;
                } else
                    wCount++;
                break;
        }
        mtx.unlock();
        if (onWait) {
            WriterSem.wait();
        }
    }

    void WriterUnlock() {
        mtx.lock();
        wCount--;
        if (wWait > 0) {
            wWait--;
            wCount++;
            WriterSem.post();
        } else if (rWait > 0) {
            while (rWait > 0) {
                rWait--;
                rCount++;
                ReaderSem.post();
            }
        }
        mtx.unlock();
    }

private:
    using u_count_t = u32;
    u8 strategy;
    u_count_t rCount, wCount, rWait, wWait;
    Mutex mtx;
    Semaphore ReaderSem, WriterSem;
};

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
