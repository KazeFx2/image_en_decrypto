//
// Created by Fx Kaze on 25-1-3.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <functional>

#include "private/types.h"
#include "RWLock.h"
#include "pthread.h"
#include <vector>

class ThreadPool {
public:
    using u_count_t = u32;
    using s_count_t = s32;
    using thread_descriptor_t = u_count_t;

    typedef void *(*funcHandler)(void *);

    template<typename R>
    class threadDescriptor {
    public:
        threadDescriptor(ThreadPool *pool, const thread_descriptor_t descriptor): pool(pool), descriptor(descriptor) {
        }

        R wait() {
            if constexpr (std::is_same_v<R, void>) {
                pool->waitThread(descriptor);
                return nullptr;
            } else {
                typedef struct {
                    R &&ret;
                } returnType;
                auto *tmp = static_cast<returnType *>(pool->waitThread(descriptor));
                auto &&ret = tmp->ret;
                delete tmp;
                return ret;
            }
        }

        ~threadDescriptor() {
        }

    private:
        ThreadPool *pool;
        thread_descriptor_t descriptor;
    };

    explicit ThreadPool(u_count_t nThreads);

    ThreadPool(const ThreadPool &) = delete;

    ~ThreadPool();

    thread_descriptor_t addThread(funcHandler func, void *param, bool wait = true);

    /* WARN: T& and T&& are not supported */
    template<typename Func, typename... Args>
    threadDescriptor<std::result_of_t<Func(Args...)> > addThreadEX(Func &&f, bool wait = true, Args &&... args) {
        using ReturnType = std::result_of_t<Func(Args...)>;
        using BindType = decltype(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
        typedef struct {
            BindType func;
        } ParamType;
        auto *param = new BindType(std::forward<Func>(f), std::forward<Args>(args)...);
        auto descriptor = addThread([](void *params) -> void *{
            auto *innerParam = static_cast<ParamType *>(params);
            if constexpr (std::is_void_v<ReturnType>) {
                innerParam->func();
                delete innerParam;
                return nullptr;
            } else {
                typedef struct {
                    ReturnType &&ret;
                } returnType;
                auto *ret = new returnType{
                    innerParam->func()

                };
                delete innerParam;
                return ret;
            }
        }, param, wait);
        if (!~descriptor) {
            delete param;
        }
        return threadDescriptor<std::result_of_t<Func(Args...)> >(this, descriptor);
    }

    void *waitThread(thread_descriptor_t index);

    bool destroyThread(thread_descriptor_t index, bool onlyIdly = false);

    void reduceTo(s_count_t tar, bool force = false);

    void waitReduce();

    void waitFinish();

    u_count_t getNumThreads() const;

    u_count_t getIdlyThreads();

private:
    typedef enum {
        Idly = 0x1,
        Working = 0x2,
        Returning = 0x4,
        Empty = 0x8,
    } ThreadStatus;

    typedef struct {
        volatile u8 status;
        volatile u_count_t refers;
        volatile bool forceTerminate;
        volatile bool waitNeed;
        volatile funcHandler func;
        void *param_return;
        Mutex mtx;
    } ThreadMtx;

    typedef struct {
        ThreadPool *pool;
        u_count_t id;
        Semaphore semaStart;
        Semaphore semaFinish;
        pthread_t thread;
        ThreadMtx mtxContext;
    } ThreadContext;

    typedef struct {
        volatile u_count_t threadCount;
        volatile u_count_t idlyCount;
        volatile u_count_t waitReduceRefers;
        volatile u_count_t waitFinishRefers;
        volatile s_count_t reduce;
        volatile bool termAll;
        Mutex mtx;
    } PoolMtx;

    volatile u_count_t idx;
    Semaphore poolTermSem, reduceWaitSem, finishWaitSem;
    PoolMtx mtxContext;
    RWLock lockAll;

    std::vector<ThreadContext *> threads;

    void startThread(u_count_t id) const;

    void initThread(ThreadContext &th, u_count_t id);

    volatile u8 &status(u_count_t id) const;

    volatile bool &terminate(u_count_t id) const;

    volatile bool &wait(u_count_t id) const;

    volatile funcHandler &function(u_count_t id) const;

    void lock(u_count_t id) const;

    void unlock(u_count_t id) const;

    void *&paramReturn(u_count_t id) const;

    volatile u_count_t &refers(u_count_t id) const;

    static void resetThread(ThreadMtx &tMtx);

    static u_count_t getIdx();

    static void *threadFunc(void *arg);

    bool destroyThreadLocked(thread_descriptor_t index, bool allLocked = true, bool onlyIdly = false);

    void wakeFinishSem();
};


#endif //THREAD_POOL_H
