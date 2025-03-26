//
// Created by Fx Kaze on 25-1-3.
//

#ifndef RK_SEMA_H
#define RK_SEMA_H

#include <cerrno>
#ifdef __linux__
#include <asm-generic/errno-base.h>
#elif _WIN32
#include <windows.h>
#define MAX_SEM_COUNT 1024
#elif __APPLE__
#include <stdexcept>
#endif

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#define SEMA_TIMEOUT DISPATCH_TIME_FOREVER
// #define SEMA_TIMEOUT (dispatch_time(DISPATCH_TIME_NOW, (int64_t)(10 * NSEC_PER_SEC)))
#elif __linux__
#include <semaphore.h>
#endif

struct rk_sema {
#ifdef __APPLE__
    dispatch_semaphore_t sem;
#elif __linux__
    sem_t sem;
#elif _WIN32
    HANDLE sem;
#endif
};


static inline void
rk_sema_init(struct rk_sema *s, uint32_t value) {
#ifdef __APPLE__
    dispatch_semaphore_t *sem = &s->sem;
    *sem = dispatch_semaphore_create(value);
#elif __linux__
    sem_init(&s->sem, 0, value);
#elif _WIN32
    s->sem = CreateSemaphore(nullptr, value, MAX_SEM_COUNT, nullptr);
#endif
}

static inline void
rk_sema_wait(struct rk_sema *s) {
#ifdef __APPLE__
    if (dispatch_semaphore_wait(s->sem, SEMA_TIMEOUT))
        throw std::runtime_error("semaphore wait timeout");
#elif __linux__
    int r;
    do
    {
        r = sem_wait(&s->sem);
    }
    while (r == -1 && errno == EINTR);
#elif _WIN32
    WaitForSingleObject(s->sem, INFINITE);
#endif
}

static inline void
rk_sema_post(struct rk_sema *s) {
#ifdef __APPLE__
    dispatch_semaphore_signal(s->sem);
#elif __linux__
    sem_post(&s->sem);
#elif _WIN32
    ReleaseSemaphore(s->sem, 1, nullptr);
#endif
}

static inline void
rk_sema_close(struct rk_sema *s) {
#ifdef __APPLE__
    // nothing...
#elif __linux__
    sem_post(&s->sem);
#elif _WIN32
    CloseHandle(s->sem);
#endif
}

#endif //RK_SEMA_H
