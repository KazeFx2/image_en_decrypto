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
    using task_descriptor_t = u_count_t;

#define THREAD_FAILED (~static_cast<u_count_t>(0x0))
#define IS_THREAD_FAILED(thread_id) (!(~(thread_id)))

    typedef void *(*funcHandler)(void *);

    typedef enum {
        Idly = 0x1,
        Working = 0x2,
        Empty = 0x4,
        // Returning = 0x8,
    } ThreadStatus;

    typedef struct {
        funcHandler func;
        void *param_return;
        bool wait;
        volatile u_count_t refers;
        task_descriptor_t threadId;
        u32 uniqueId;
        bool finished;
        Semaphore semaFinish;
        Mutex mtx;
    } TaskInfo;

    typedef struct {
        volatile u8 status;
        volatile bool forceTerminate;
        // volatile bool waitNeed;
        TaskInfo *volatile task;
        // volatile funcHandler func;
        // void *param_return;
        Mutex mtx;
    } ThreadMtx;

    typedef struct {
        ThreadPool *pool;
        u_count_t id;
        Semaphore semaStart;
        pthread_t thread;
        ThreadMtx mtxContext;
    } ThreadContext;

    typedef struct {
        volatile u_count_t threadCount;
        volatile u_count_t maxCount;
        volatile u_count_t idlyCount;
        volatile u_count_t waitReduceRefers;
        volatile u_count_t waitFinishRefers;
        volatile u_count_t waitIdlyRefers;
        volatile s_count_t reduce;
        std::vector<TaskInfo *> taskArray;
        volatile bool termAll;
        Mutex mtx;
    } PoolMtx;

    template<typename R>
    class taskDescriptor {
    public:
        typedef struct {
            const void *exceptFunc;
            const void *thenFunc;
            bool wait;
        } ParamType;

        typedef struct {
            R ret;
        } returnType;

        taskDescriptor(ThreadPool *pool, const funcHandler func, void *param): pool(pool), func(func),
                                                                               param(static_cast<ParamType *>(param)),
                                                                               descriptor(THREAD_FAILED), taskUnique(0),
                                                                               task(nullptr) {
        }

        R wait() {
            if (descriptor == THREAD_FAILED) {
                throw std::runtime_error("threadDescriptor::wait: descriptor is invalid");
            }
            if constexpr (std::is_same_v<R, void>) {
                pool->waitThread(descriptor);
                return nullptr;
            } else {
                auto *tmp = static_cast<returnType *>(pool->waitThread(descriptor));
                auto &&ret = tmp->ret;
                delete tmp;
                return ret;
            }
        }

        taskDescriptor &setWait() {
            param->wait = true;
            return *this;
        }

        taskDescriptor &setNoWait() {
            param->wait = false;
            return *this;
        }

        template<typename Func>
        taskDescriptor &except(const Func &&exceptFunc) {
            if constexpr (std::is_same_v<R, void>) {
                using FuncType = const std::function<void(std::exception &)>;
                if (param->exceptFunc != nullptr) {
                    delete static_cast<FuncType *>(param->exceptFunc);
                }
                param->exceptFunc = new FuncType(exceptFunc);
            } else {
                using FuncType = const std::function<void(std::exception &, R &)>;
                if (param->exceptFunc != nullptr) {
                    delete static_cast<FuncType *>(param->exceptFunc);
                }
                param->exceptFunc = new FuncType(exceptFunc);
            }
            return *this;
        }

        template<typename Func>
        taskDescriptor &then(const Func &&thenFunc) {
            if constexpr (std::is_same_v<R, void>) {
                if (param->thenFunc != nullptr) {
                    delete static_cast<const std::function<void()> *>(param->thenFunc);
                }
                param->thenFunc = new std::function<void()>(thenFunc);
            } else {
                if (param->thenFunc != nullptr) {
                    delete static_cast<const std::function<void(R)> *>(param->thenFunc);
                }
                param->thenFunc = new std::function<void(R)>(thenFunc);
            }
            param->wait = false;
            return *this;
        }

        taskDescriptor &start() {
            descriptor = pool->addThread(func, param, param->wait, &task);
            if (descriptor == THREAD_FAILED) {
                delete param;
                return *this;
            }
            taskUnique = task->uniqueId;
            return *this;
        }

        ~taskDescriptor() {
        }

    private:
        ThreadPool *pool;
        funcHandler func;
        ParamType *param;
        task_descriptor_t descriptor;
        u32 taskUnique;
        TaskInfo *task;
    };

    explicit ThreadPool(u_count_t nThreads);

    ThreadPool(const ThreadPool &) = delete;

    ~ThreadPool();

    task_descriptor_t addThread(funcHandler func, void *param, bool wait = true,
                                TaskInfo **taskOut = nullptr);

    /* WARN: param T&& and return T& are not supported */
    template<typename Func, typename... Args>
    taskDescriptor<std::result_of_t<Func(Args...)> > addThreadEX(Func &&f, Args &&... args) {
        using ReturnType = std::result_of_t<Func(Args...)>;
        using BindType = decltype(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
        typedef struct {
            const void *exceptFunc;
            const void *thenFunc;
            bool wait;
            BindType func;
        } ParamType;
        auto *param = new ParamType{
            nullptr, nullptr, true, BindType(std::forward<Func>(f), std::forward<Args>(args)...)
        };
        if constexpr (std::is_same_v<ReturnType, void>) {
            return taskDescriptor<void>(this, [](void *params) -> void *{
                auto *innerParam = static_cast<ParamType *>(params);
                const auto *exceptFunc = static_cast<const std::function<void
                    (std::exception &)> *>(innerParam->exceptFunc);
                const auto *thenFunc = static_cast<const std::function<void()> *>(innerParam->
                    thenFunc);
                if (exceptFunc) {
                    try {
                        innerParam->func();
                    } catch (std::exception &e) {
                        (*exceptFunc)(e);
                    }
                    delete exceptFunc;
                } else
                    innerParam->func();
                if (thenFunc) {
                    (*thenFunc)();
                    delete thenFunc;
                }
                delete innerParam;
                return nullptr;
            }, param);
        } else {
            return taskDescriptor<ReturnType>(this, [](void *params) -> void *{
                auto *innerParam = static_cast<ParamType *>(params);
                const auto *exceptFunc = static_cast<const std::function<void
                    (std::exception &, ReturnType &)> *>(innerParam->exceptFunc);
                const auto *thenFunc = static_cast<const std::function<void
                    (ReturnType)> *>(
                    innerParam->thenFunc);
                auto &wait = innerParam->wait;
                typedef struct {
                    ReturnType ret;
                } returnType;
                returnType *ret = nullptr;
                if (exceptFunc) {
                    try {
                        ret = new returnType{
                            innerParam->func()
                        };
                    } catch (std::exception &e) {
                        ret = new returnType{
                            *(new std::decay_t<ReturnType>),
                        };
                        (*exceptFunc)(e, ret->ret);
                    }
                    delete exceptFunc;
                } else {
                    ret = new returnType{
                        innerParam->func(),
                    };
                }
                if (thenFunc && ret) {
                    (*thenFunc)(ret->ret);
                    delete thenFunc;
                    delete ret;
                    delete innerParam;
                    return nullptr;
                }
                if (!wait)
                    delete ret, ret = nullptr;
                delete innerParam;
                return ret;
            }, param);
        }
    }

    bool isDescriptorAvailable(task_descriptor_t descriptor);

    void *waitThread(task_descriptor_t index);

    bool destroyThread(u_count_t index, bool onlyIdly = false);

    void reduceTo(s_count_t tar, bool force = false);

    void setMax(u_count_t max);

    void waitReduce();

    void waitFinish();

    u_count_t getNumThreads() const;

    u_count_t getIdlyThreads() const;

    u_count_t getMaxThreads() const;

private:
    volatile u_count_t idx;
    FastBitmap threadIdly, threadEmpty, waitingTasks, emptyTasks;
    Semaphore poolTermSem, reduceWaitSem, idlyWaitSem, finishWaitSem;
    PoolMtx mtxContext;
    RWLock lockAll;

    std::vector<ThreadContext *> threads;

    u_count_t addTaskLocked(funcHandler func, void *param, bool wait);

    // returns `true` if successfully added a new task to the given thread with id `current_id`
    // else `false`
    bool addThreadFromWaitingArrayLocked(u_count_t current_id);

    void startThread(u_count_t id);

    void initThread(ThreadContext &th, u_count_t id);

    volatile u8 &status(u_count_t id) const;

    volatile bool &terminate(u_count_t id) const;

    void lockThread(u_count_t id) const;

    void unlockThread(u_count_t id) const;

    void lockTask(task_descriptor_t id) const;

    void unlockTask(task_descriptor_t id) const;

    static void resetThread(ThreadMtx &tMtx);

    static u_count_t getIdx();

    static void *threadFunc(void *arg);

    bool destroyThreadLocked(u_count_t index, bool onlyIdly = false) const;

    // check and notify if all threads/tasks were finished
    void wakeFinishSem();
};


#endif //THREAD_POOL_H
