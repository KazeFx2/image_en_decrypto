//
// Created by Fx Kaze on 24-12-30.
//

#ifndef UTIL_H
#define UTIL_H

#include "includes.h"

f64 PLCM(__IN const f64 &prevCondition, __IN const f64 &controlCondition);

void ConfusionFunc(__IN u32 row, __IN u32 col, __IN const cv::Size &size, __IN u32 confusionSeed, __OUT u32 &newRow,
                   __OUT u32 &newCol);

#endif //UTIL_H
