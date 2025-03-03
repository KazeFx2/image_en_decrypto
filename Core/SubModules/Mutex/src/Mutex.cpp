//
// Created by Fx Kaze on 25-1-3.
//

#include "Mutex.h"

// FastBitmap Mutex::bitmap;

Mutex::Mutex(): sem(static_cast<u32>(1))
{
    mtxId = get_putIdx();
#ifdef __DEBUG
        ct = 1;
        printf("[mutex]id: %zu, op: init, value: %d, addr: %p\n", mtxId, ct, this);
#endif
}

Mutex::~Mutex()
{
    if (mtxId + 1 == 0) return;
    get_putIdx(true, mtxId);
}

Mutex::Mutex(Mutex&& other): sem(std::move(other.sem))
{
#ifdef __DEBUG
    ct = other.ct;
#endif
    mtxId = other.mtxId;
    other.mtxId = 0;
    other.mtxId--;
}


u64 Mutex::get_putIdx(const bool isRet, const u64 oldIdx)
{
    static u64 idx = 0;
    static Semaphore semaphore(1);
    static FastBitmap bitmap;
    semaphore.wait();
    if (isRet == true)
    {
        bitmap[oldIdx] = false;
        semaphore.post();
        return oldIdx;
    }
    auto id = bitmap.findNextFalse(0, idx);
    if (id != BITMAP_NOT_FOUND)
    {
        bitmap[id] = true;
        semaphore.post();
        return id;
    }
    idx++;
    const u64 ret = idx - 1;
    bitmap[ret] = true;
    semaphore.post();
    return ret;
}

void Mutex::lock()
{
    sem.wait();
#ifdef __DEBUG
        ct--;
        printf("[mutex]id: %zu, op: lock, value: %d, addr: %p\n", mtxId, ct, this);
#endif
}

void Mutex::unlock()
{
#ifdef __DEBUG
        ct++;
        printf("[mutex]id: %zu, op: unlock, value: %d, addr: %p\n", mtxId, ct, this);
#endif
    sem.post();
}
