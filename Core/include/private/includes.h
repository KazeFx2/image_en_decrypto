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

typedef struct {
    u32 width;
    u32 height;
} ImageSize;

typedef struct {
    f64 initCondition;
    f64 ctrlCondition;
} ParamGroup;

typedef struct {
    u8 byteReserve;
    u32 iterations;
    u32 confusionIterations;
    u32 diffusionIterations;
} ParamControl;

typedef struct {
    u32 confusionSeed;
    ParamGroup gParam1;
    ParamGroup gParam2;
} Keys;

#define ORIGINAL_SIZE ImageSize{0, 0}
#define RANDOM_KEYS Keys{Rand16(), GenDoubleFloatFrom1To0Random(), GenDoubleFloatFrom1To0Random()}

#endif //INCLUDES_H
