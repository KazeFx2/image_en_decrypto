//
// Created by Fx Kaze on 25-1-3.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <functional>
#include <stdexcept>

#include "CoreThreadPool_private.h"

class ThreadPool {
    ThreadPool_private _private;

public:
    using u_count_t = u32;
    using s_count_t = i32;
    using task_descriptor_t = u_count_t;

#define THREAD_FAILED (~static_cast<u_count_t>(0x0))
#define IS_THREAD_FAILED(thread_id) (!(~(thread_id)))

    typedef void *(*funcHandler)(void *);

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

        taskDescriptor(): pool(nullptr), func(nullptr), param(nullptr), descriptor(false), taskUnique(0) {
        }

        taskDescriptor(ThreadPool *pool, const funcHandler func, void *param): pool(pool), func(func),
                                                                               param(static_cast<ParamType *>(param)),
                                                                               descriptor(THREAD_FAILED),
                                                                               taskUnique(0) {
        }

        R wait() {
            if (descriptor == THREAD_FAILED) {
                throw std::runtime_error("threadDescriptor::wait: descriptor is invalid");
            }
            if constexpr (std::is_same_v<R, void>) {
                pool->waitThread(descriptor);
                return;
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
            descriptor = pool->addThread(func, param, param->wait, &taskUnique);
            if (descriptor == THREAD_FAILED) {
                delete param;
                return *this;
            }
            return *this;
        }

        ~taskDescriptor() = default;

    private:
        ThreadPool *pool;
        funcHandler func;
        ParamType *param;
        task_descriptor_t descriptor;
        u32 taskUnique;
    };

    explicit ThreadPool(u_count_t nThreads = 8);

    ThreadPool(const ThreadPool &) = delete;

    ~ThreadPool();

    virtual task_descriptor_t addThread(funcHandler func, void *param, bool wait = true,
                                u32 *taskUnique = nullptr);

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

    virtual bool isDescriptorAvailable(task_descriptor_t descriptor);

    virtual  void *waitThread(task_descriptor_t index);

    virtual bool destroyThread(u_count_t index, bool onlyIdly = false);

    virtual void reduceTo(s_count_t tar, bool force = false);

    virtual void setMax(u_count_t max);

    virtual void waitReduce();

    virtual void waitFinish();

    virtual u_count_t getNumThreads() const;

    virtual u_count_t getIdlyThreads() const;

    virtual u_count_t getMaxThreads() const;
};


#endif //THREAD_POOL_H
