//
// Created by Fx Kaze on 25-1-3.
//

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "CoreSemaphore_private.h"

class Semaphore {
    Semaphore_private _private;

public:
    Semaphore(u32 init = 0);

    ~Semaphore();

    Semaphore(const Semaphore &other) = delete;

    Semaphore &operator=(const Semaphore &other) = delete;

    Semaphore(Semaphore &&other);

    Semaphore &operator=(Semaphore &&other);

    void wait();

    void post();

    i32 value() const;
};

#endif //SEMAPHORE_H
