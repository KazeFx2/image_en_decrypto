//
// Created by Fx Kaze on 25-1-3.
//

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <Bitmap.h>

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
    static u64 get_putIdx(u32 init, rk_sema *&out, bool isRet = false, u64 oldIdx = 0);

    rk_sema *sem;
    volatile u32 idx;
    std::atomic_uint32_t ct;
};

#endif //SEMAPHORE_H
