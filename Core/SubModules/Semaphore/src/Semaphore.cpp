//
// Created by Fx Kaze on 25-1-3.
//

#include "Semaphore.h"

class tSem {
public:
    tSem(const u32 init) {
        rk_sema_init(&sem, init);
    }

    ~tSem() {
        rk_sema_close(&sem);
    }

    void wait() {
        rk_sema_wait(&sem);
    }

    void post() {
        rk_sema_post(&sem);
    }

private:
    rk_sema sem;
};

Semaphore::Semaphore(const u32 init) {
    idx = get_putIdx(init, sem);
    ct = init;
}

Semaphore::~Semaphore() {
    while (ct > 0) {
        wait();
    }
    get_putIdx(0, sem, true, idx);
}

u64 Semaphore::get_putIdx(u32 init, rk_sema *&out, const bool isRet, const u64 oldIdx) {
    static u64 idx = 0;
    static tSem semaphore(1);
    static std::vector<rk_sema *> semaVec;
    static FastBitmap bitmap;
    semaphore.wait();
    auto ptr = semaVec;
    auto a1 = bitmap[0];
    if (isRet == true) {
        bitmap[oldIdx] = false;
        semaphore.post();
        out = nullptr;
        return oldIdx;
    }
    auto id = bitmap.findNextFalse(0, idx);
    if (id != BITMAP_NOT_FOUND) {
        bitmap[id] = true;
        while (init > 0)
            rk_sema_post(semaVec[id]), --init;
        semaphore.post();
        out = semaVec[id];
        return id;
    }
    idx++;
    const u64 ret = idx - 1;
    bitmap[ret] = true;
    semaVec.push_back(new rk_sema());
    rk_sema_init(semaVec[ret], init);
    auto t = &semaVec[ret];
    semaphore.post();
    out = semaVec[ret];
    return ret;
}


void Semaphore::wait() {
    rk_sema_wait(sem);
    --ct;
}

void Semaphore::post() {
    ++ct;
    rk_sema_post(sem);
}
