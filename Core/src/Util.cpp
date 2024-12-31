//
// Created by Fx Kaze on 24-12-30.
//

#include "private/Util.h"
#include "private/Random.h"
#include "math.h"

f64 PLCM(__IN const f64 &prevCondition, __IN const f64 &controlCondition) {
    assert(prevCondition >= 0 && prevCondition <= 1);
    if (prevCondition < controlCondition) {
        return prevCondition / controlCondition;
    }
    if (prevCondition <= 0.5) {
        return (prevCondition - controlCondition) / (0.5 - controlCondition);
    }
    return PLCM(1 - prevCondition, controlCondition);
}

void ConfusionFunc(__IN const u32 row, __IN const u32 col, __IN const cv::Size &size, __IN const u32 confusionSeed,
                   __OUT u32 &newRow,
                   __OUT u32 &newCol) {
    newRow = (row + col) % size.height;
    newCol = ((col + static_cast<u32>(round(confusionSeed * sin(2 * M_PI * newRow / size.height)))) % size.width + size.
              width) % size.
             width;
}
