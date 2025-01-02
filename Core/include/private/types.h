//
// Created by Fx Kaze on 24-12-31.
//

#ifndef TYPES_H
#define TYPES_H

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

struct rk_sema {
#ifdef __APPLE__
    dispatch_semaphore_t sem;
#else
    sem_t                   sem;
#endif
};


static inline void
rk_sema_init(struct rk_sema *s, uint32_t value) {
#ifdef __APPLE__
    dispatch_semaphore_t *sem = &s->sem;

    *sem = dispatch_semaphore_create(value);
#else
    sem_init(&s->sem, 0, value);
#endif
}

static inline void
rk_sema_wait(struct rk_sema *s) {
#ifdef __APPLE__
    dispatch_semaphore_wait(s->sem, DISPATCH_TIME_FOREVER);
#else
    int r;

    do {
        r = sem_wait(&s->sem);
    } while (r == -1 && errno == EINTR);
#endif
}

static inline void
rk_sema_post(struct rk_sema *s) {
#ifdef __APPLE__
    dispatch_semaphore_signal(s->sem);
#else
    sem_post(&s->sem);
#endif
}

static inline void
rk_sema_close(struct rk_sema *s) {
#ifdef __APPLE__
    // nothing...
#else
    sem_post(&s->sem);
#endif
}

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef uintptr_t uptr;

#endif //TYPES_H
