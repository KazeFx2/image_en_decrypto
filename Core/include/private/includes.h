//
// Created by Fx Kaze on 24-12-30.
//

#ifndef INCLUDES_H
#define INCLUDES_H

#include <opencv2/opencv.hpp>
#include "types.h"

#define __IN
#define __OUT
#define __IN_OUT

typedef struct ImageSize_s {
    u32 width;
    u32 height;
} ImageSize;

typedef struct Keys_s {
    u32 confusionSeed;
    u32 iterations;
    double initCondition;
    double controlCondition;
} Keys;

#define ORIGINAL_SIZE ImageSize{0, 0}
#define RANDOM_KEYS Keys{Rand16(), 8, GenDoubleFloatFrom1To0Random(), GenDoubleFloatFrom1To0Random()}

#endif //INCLUDES_H
