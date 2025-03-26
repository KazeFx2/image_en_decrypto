//
// Created by Fx Kaze on 25-1-3.
//

#ifndef MUTEX_H
#define MUTEX_H

#include "types.h"
#include "Semaphore.h"

class Mutex {
public:
    Mutex();

    ~Mutex();

    void lock();

    void unlock();

    Mutex(const Mutex &other) = delete;

    Mutex &operator=(const Mutex &other) = delete;

    Mutex(Mutex &&other);

    Mutex &operator=(Mutex &&other);

private:
#ifdef __DEBUG
    u8 ct;
#endif
    u64 mtxId;
    Semaphore sem;

    // static FastBitmap bitmap;

    u64 static get_putIdx(bool isRet = false, u64 oldIdx = 0);
};

#endif //MUTEX_H
