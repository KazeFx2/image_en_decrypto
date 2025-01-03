//
// Created by Fx Kaze on 25-1-3.
//

#include "Mutex.h"
#include <mutex>

static std::mutex t_mtx;

FastBitmap Mutex::bitmap;

Mutex::Mutex(): sem(static_cast<u32>(1)) {
    mtxId = getIdx();
#ifdef __DEBUG
        ct = 1;
        printf("[mutex]id: %zu, op: init, value: %d, addr: %p\n", mtxId, ct, this);
#endif
}

Mutex::~Mutex() {
    t_mtx.lock();
    bitmap[mtxId] = false;
    t_mtx.unlock();
}

u64 Mutex::getIdx() {
    static u64 idx = 0;
    t_mtx.lock();
    for (u64 i = 0; i < idx; i++) {
        if (!bitmap[i]) {
            bitmap[i] = true;
            t_mtx.unlock();
            return i;
        }
    }
    idx++;
    const u64 ret = idx - 1;
    bitmap[ret] = true;
    t_mtx.unlock();
    return ret;
}

void Mutex::lock() {
    sem.wait();
#ifdef __DEBUG
        ct--;
        printf("[mutex]id: %zu, op: lock, value: %d, addr: %p\n", mtxId, ct, this);
#endif
}

void Mutex::unlock() {
#ifdef __DEBUG
        ct++;
        printf("[mutex]id: %zu, op: unlock, value: %d, addr: %p\n", mtxId, ct, this);
#endif
    sem.post();
}
