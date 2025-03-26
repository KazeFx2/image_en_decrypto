//
// Created by Fx Kaze on 25-1-17.
//

#include "ConditionVariable.h"

#include <algorithm>

ConditionVariable::ConditionVariable(): sema(0) {
}

ConditionVariable::~ConditionVariable() {
}

ConditionVariable::ConditionVariable(ConditionVariable &&other): sema(std::move(other.sema)) {
}

ConditionVariable &ConditionVariable::operator=(ConditionVariable &&other) {
    this->~ConditionVariable();
    sema = std::move(other.sema);
    return *this;
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
