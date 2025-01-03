//
// Created by Fx Kaze on 25-1-3.
//

#include "Semaphore.h"

Semaphore::Semaphore(const u32 init) {
    rk_sema_init(&sem, init);
}

Semaphore::~Semaphore() {
    rk_sema_close(&sem);
}

void Semaphore::wait() {
    rk_sema_wait(&sem);
}

void Semaphore::post() {
    rk_sema_post(&sem);
}
