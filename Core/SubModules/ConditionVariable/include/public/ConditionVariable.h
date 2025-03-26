//
// Created by Fx Kaze on 25-1-17.
//

#ifndef CONDITION_VARIABLE_H
#define CONDITION_VARIABLE_H

#include "CoreConditionVariable_private.h"

class ConditionVariable {
    ConditionVariable_private _private;

public:
    ConditionVariable();

    ~ConditionVariable();

    ConditionVariable(const ConditionVariable &other) = delete;

    ConditionVariable &operator=(const ConditionVariable &other) = delete;

    ConditionVariable(ConditionVariable &&other);

    ConditionVariable &operator=(ConditionVariable &&other);

    void wait();

    void notify_one();

    void notify_all();
};

#endif //CONDITION_VARIABLE_H
