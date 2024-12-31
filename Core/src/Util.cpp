//
// Created by Fx Kaze on 24-12-30.
//

#include "private/Util.h"
#include "private/Random.h"

double PLCM(__IN const double &prevCondition, __IN const double &controlCondition) {
    assert(prevCondition >= 0 && prevCondition <= 1);
    if (prevCondition < controlCondition) {
        return prevCondition / controlCondition;
    }
    if (prevCondition <= 0.5) {
        return (prevCondition  - controlCondition) / (0.5 - controlCondition);
    }
    return PLCM(1 - prevCondition, controlCondition);
}
