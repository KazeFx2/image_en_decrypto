//
// Created by Fx Kaze on 25-1-3.
//

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "private/types.h"
#include "rk_sema.h"

class Semaphore {
public:
    Semaphore(u32 init = 0);

    ~Semaphore();

    Semaphore(const Semaphore &other) = delete;

    Semaphore &operator=(const Semaphore &other) = delete;

    Semaphore(Semaphore &&other) = delete;

    Semaphore &operator=(Semaphore &&other) = delete;

    void wait();

    void post();

private:
    rk_sema sem;
};

#endif //SEMAPHORE_H
