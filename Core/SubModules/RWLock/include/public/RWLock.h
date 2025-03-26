//
// Created by Fx Kaze on 25-1-3.
//

#ifndef RW_LOCK_H
#define RW_LOCK_H

#include "CoreRWLock_private.h"

class RWLock {
    RWLock_private _private;

public:
    class SubLock {
        SubLock_private _private;

    public:
        SubLock(RWLock *p, void (RWLock::*lock)(), void (RWLock::*unlock)());

        ~SubLock();

        void lock() const;

        void unlock() const;
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
};

#endif //RW_LOCK_H
