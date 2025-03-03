//
// Created by Fx Kaze on 25-1-17.
//

#include "ConditionVariable.h"

ConditionVariable::ConditionVariable(): sema(0) {
}

ConditionVariable::~ConditionVariable() {
}

void ConditionVariable::wait() {
    sema.wait();
}

void ConditionVariable::notify_one() {
    sema.post();
}

void ConditionVariable::notify_all() {
    while (sema.value() < 0)
        sema.post();
}
