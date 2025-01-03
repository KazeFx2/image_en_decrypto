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
    u32 preIterations;
    u32 confusionIterations;
    u32 diffusionConfusionIterations;
} ParamControl;

typedef struct {
    u32 confusionSeed;
    ParamGroup gParam1;
    ParamGroup gParam2;
} Keys;

#define ORIGINAL_SIZE ImageSize{0, 0}
#define RANDOM_KEYS Keys{Rand16(), GenDoubleFloatFrom1To0Random(), GenDoubleFloatFrom1To0Random() / 2, GenDoubleFloatFrom1To0Random(), GenDoubleFloatFrom1To0Random() / 2}
#define DEFAULT_CONFIG ParamControl{6, 200, 3, 5}

#endif //INCLUDES_H
