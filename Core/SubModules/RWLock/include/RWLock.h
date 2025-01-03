//
// Created by Fx Kaze on 25-1-3.
//

#ifndef RW_LOCK_H
#define RW_LOCK_H

#include "private/types.h"
#include "Mutex.h"

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

    RWLock(const u8 Strategy = WriterFist);

    ~RWLock();

    typedef enum {
        WriterFist = 0x0,
        ReaderFist = 0x1
    } RW_Strategy;

    RWLock(const RWLock &other) = delete;

    RWLock &operator=(const RWLock &other) = delete;

    RWLock(RWLock &&other) = delete;

    RWLock &operator=(RWLock &&other) = delete;

    SubLock reader();

    SubLock writer();

    void ReaderLock();

    void ReaderUnlock();

    void WriterLock();

    void WriterUnlock();

private:
    using u_count_t = u32;
    u8 strategy;
    u_count_t rCount, wCount, rWait, wWait;
    Mutex mtx;
    Semaphore ReaderSem, WriterSem;
};

#endif //RW_LOCK_H
