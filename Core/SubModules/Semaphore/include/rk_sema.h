//
// Created by Fx Kaze on 25-1-3.
//

#ifndef RK_SEMA_H
#define RK_SEMA_H

#include <cerrno>
#include <asm-generic/errno-base.h>

#include "private/types.h"

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#define SEMA_TIMEOUT DISPATCH_TIME_FOREVER
// #define SEMA_TIMEOUT (dispatch_time(DISPATCH_TIME_NOW, (int64_t)(10 * NSEC_PER_SEC)))
#else
#include <semaphore.h>
#endif

struct rk_sema
{
#ifdef __APPLE__
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
};


static inline void
rk_sema_init(struct rk_sema* s, uint32_t value)
{
#ifdef __APPLE__
    dispatch_semaphore_t *sem = &s->sem;

    *sem = dispatch_semaphore_create(value);
#else
    sem_init(&s->sem, 0, value);
#endif
}

static inline void
rk_sema_wait(struct rk_sema* s)
{
#ifdef __APPLE__
    if (dispatch_semaphore_wait(s->sem, SEMA_TIMEOUT))
        throw std::runtime_error("semaphore wait timeout");
#else
    int r;

    do
    {
        r = sem_wait(&s->sem);
    }
    while (r == -1 && errno == EINTR);
#endif
}

static inline void
rk_sema_post(struct rk_sema* s)
{
#ifdef __APPLE__
    dispatch_semaphore_signal(s->sem);
#else
    sem_post(&s->sem);
#endif
}

static inline void
rk_sema_close(struct rk_sema* s)
{
#ifdef __APPLE__
    // nothing...
#else
    sem_post(&s->sem);
#endif
}

#endif //RK_SEMA_H
