//
// Created by Fx Kaze on 25-1-3.
//

#include "Semaphore.h"
#include "Bitmap.h"
#include <vector>

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
    if (!sem) return;
    while (ct > 0) {
        wait();
    }
    get_putIdx(0, sem, true, idx);
}

Semaphore::Semaphore(Semaphore &&other) {
    sem = other.sem;
    other.sem = nullptr;
    idx = other.idx;
    ct = other.ct.load();
}

Semaphore &Semaphore::operator=(Semaphore &&other) {
    this->~Semaphore();
    sem = other.sem;
    other.sem = nullptr;
    idx = other.idx;
    ct = other.ct.load();
    return *this;
}

u64 Semaphore::get_putIdx(u32 init, rk_sema *&out, const bool isRet, const u64 oldIdx) {
    static u64 idx = 0;
    static tSem semaphore(1);
    static std::vector<rk_sema *> semaVec;
    static FastBitmap bitmap;
    semaphore.wait();
    auto ptr = semaVec;
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
    semaphore.post();
    out = semaVec[ret];
    return ret;
}


void Semaphore::wait() {
    --ct;
    rk_sema_wait(sem);
}

void Semaphore::post() {
    ++ct;
    rk_sema_post(sem);
}

i32 Semaphore::value() const {
    return ct;
}
