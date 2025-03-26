//
// Created by Fx Kaze on 25-1-3.
//

#include "RWLock.h"

#include <algorithm>

RWLock::SubLock::SubLock(RWLock *p, void (RWLock::*lock)(), void (RWLock::*unlock)()): parent(p), lockHandler(lock),
    unlockHandler(unlock) {
}

RWLock::SubLock::~SubLock() = default;

void RWLock::SubLock::lock() const {
    (parent->*lockHandler)();
}

void RWLock::SubLock::unlock() const {
    (parent->*unlockHandler)();
}

RWLock::RWLock(const u8 Strategy): strategy(Strategy), rCount(0), wCount(0), rWait(0), wWait(0), ReaderSem(0),
                                   WriterSem(0) {
}

RWLock::~RWLock() {
}

RWLock::RWLock(RWLock &&other): mtx(std::move(other.mtx)), ReaderSem(std::move(other.ReaderSem)),
                                WriterSem(std::move(other.WriterSem)) {
    strategy = other.strategy;
    rCount = other.rCount;
    wCount = other.wCount;
    rWait = other.rWait;
    wWait = other.wWait;
}

RWLock &RWLock::operator=(RWLock &&other) {
    this->~RWLock();
    strategy = other.strategy;
    rCount = other.rCount;
    wCount = other.wCount;
    rWait = other.rWait;
    wWait = other.wWait;
    mtx = std::move(other.mtx);
    ReaderSem = std::move(other.ReaderSem);
    WriterSem = std::move(other.WriterSem);
    return *this;
}

RWLock::SubLock RWLock::reader() {
    return SubLock(this, &RWLock::ReaderLock, &RWLock::ReaderUnlock);
}

RWLock::SubLock RWLock::writer() {
    return SubLock(this, &RWLock::WriterLock, &RWLock::WriterUnlock);
}

void RWLock::ReaderLock() {
    bool onWait = false;
    mtx.lock();
    switch (strategy) {
        case WriterFist:
            if (wCount > 0 || wWait > 0) {
                ++rWait;
                onWait = true;
            } else
                ++rCount;
            break;
        case ReaderFist:
        default:
            if (wCount > 0) {
                ++rWait;
                onWait = true;
            } else
                ++rCount;
            break;
    }
    mtx.unlock();
    if (onWait) {
        ReaderSem.wait();
    }
}

void RWLock::ReaderUnlock() {
    mtx.lock();
    --rCount;
    if (rCount == 0 && wWait > 0) {
        --wWait;
        ++wCount;
        WriterSem.post();
    }
    mtx.unlock();
}

void RWLock::WriterLock() {
    bool onWait = false;
    mtx.lock();
    switch (strategy) {
        case WriterFist:
            if (rCount > 0 || wCount > 0) {
                ++wWait;
                onWait = true;
            } else
                ++wCount;
            break;
        case ReaderFist:
        default:
            if (rCount > 0 || wCount > 0 || rWait > 0) {
                ++wWait;
                onWait = true;
            } else
                ++wCount;
            break;
    }
    mtx.unlock();
    if (onWait) {
        WriterSem.wait();
    }
}

void RWLock::WriterUnlock() {
    mtx.lock();
    --wCount;
    if (wWait > 0) {
        --wWait;
        ++wCount;
        WriterSem.post();
    } else if (rWait > 0) {
        while (rWait > 0) {
            --rWait;
            ++rCount;
            ReaderSem.post();
        }
    }
    mtx.unlock();
}
