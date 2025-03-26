//
// Created by Fx Kaze on 25-1-3.
//

#ifndef MUTEX_H
#define MUTEX_H

#include "CoreMutex_private.h"

class Mutex {
    Mutex_private _private;

public:
    Mutex();

    ~Mutex();

    void lock();

    void unlock();

    Mutex(const Mutex &other) = delete;

    Mutex &operator=(const Mutex &other) = delete;

    Mutex(Mutex &&other);

    Mutex &operator=(Mutex &&other);
};

#endif //MUTEX_H
