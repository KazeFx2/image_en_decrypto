//
// Created by Fx Kaze on 25-1-17.
//

#ifndef CONDITION_VARIABLE_H
#define CONDITION_VARIABLE_H
#include "../../Semaphore/include/Semaphore.h"

class ConditionVariable {
public:
    ConditionVariable();

    ~ConditionVariable();

    ConditionVariable(const ConditionVariable &other) = delete;

    ConditionVariable &operator=(const ConditionVariable &other) = delete;

    ConditionVariable(ConditionVariable &&other) = delete;

    ConditionVariable &operator=(ConditionVariable &&other) = delete;

    void wait();

    void notify_one();

    void notify_all();

private:
    Semaphore sema;
};

#endif //CONDITION_VARIABLE_H
