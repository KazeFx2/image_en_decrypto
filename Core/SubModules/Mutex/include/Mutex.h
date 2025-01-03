//
// Created by Fx Kaze on 25-1-3.
//

#ifndef MUTEX_H
#define MUTEX_H

#include "private/types.h"
#include "Bitmap.h"
#include "Semaphore.h"

class Mutex {
public:
    Mutex();

    ~Mutex();

    void lock();

    void unlock();

    Mutex(const Mutex &other) = delete;

    Mutex &operator=(const Mutex &other) = delete;

    Mutex(Mutex &&other) = delete;

    Mutex &operator=(Mutex &&other) = delete;

private:
#ifdef __DEBUG
    u8 ct;
#endif
    u64 mtxId;
    Semaphore sem;

    static FastBitmap bitmap;

    u64 static getIdx();
};

#endif //MUTEX_H
