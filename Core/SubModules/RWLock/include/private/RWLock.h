//
// Created by Fx Kaze on 25-1-3.
//

#ifndef RW_LOCK_H
#define RW_LOCK_H

#include "types.h"
#include "Mutex.h"
#include "Semaphore.h"

class RWLock {
public:
    class SubLock {
    public:
        SubLock(RWLock *p, void (RWLock::*lock)(), void (RWLock::*unlock)());

        ~SubLock();

        void lock() const;

        void unlock() const;

    private:
        RWLock *parent;

        void (RWLock::*lockHandler)();

        void (RWLock::*unlockHandler)();
    };

    RWLock(u8 Strategy = WriterFist);

    ~RWLock();

    typedef enum {
        WriterFist = 0x0,
        ReaderFist = 0x1
    } RW_Strategy;

    RWLock(const RWLock &other) = delete;

    RWLock &operator=(const RWLock &other) = delete;

    RWLock(RWLock &&other);

    RWLock &operator=(RWLock &&other);

    SubLock reader();

    SubLock writer();

    void ReaderLock();

    void ReaderUnlock();

    void WriterLock();

    void WriterUnlock();

private:
    using u_count_t = u32;
    u8 strategy;
    volatile u_count_t rCount, wCount, rWait, wWait;
    // std::atomic_uint32_t rCount, wCount, rWait, wWait;
    Mutex mtx;
    Semaphore ReaderSem, WriterSem;
};

#endif //RW_LOCK_H
