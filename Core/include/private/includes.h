//
// Created by Fx Kaze on 24-12-30.
//

#ifndef INCLUDES_H
#define INCLUDES_H

#include <opencv2/opencv.hpp>
#include "types.h"
#include "Semaphore.h"

// #define __DEBUG

#define __IN
#define __OUT
#define __IN_OUT

// typedef struct {
//     u32 width;
//     u32 height;
// } ImageSize;

typedef struct {
    f64 initCondition;
    f64 ctrlCondition;
} ParamGroup;

typedef struct {
    u8 byteReserve;
    u32 preIterations;
    u32 confusionIterations;
    u32 diffusionConfusionIterations;
    u32 nThread;
} ParamControl;

typedef struct {
    u32 confusionSeed;
    ParamGroup gParam1;
    ParamGroup gParam2;
} Keys;

typedef struct {
    __OUT cv::Mat **dst;
    __IN cv::Mat **src;
    __IN cv::Size *size;
    __IN size_t threadId;
    __IN u32 iterations;
    __IN Keys keys;
    __IN const ParamControl *config;
    __IN Semaphore Start;
    __IN Semaphore Finish;
} threadParams;

typedef struct {
    __OUT u8 *byteSeq;
    __OUT u8 *diffusionSeedArray;
} threadReturn;

typedef struct {
    threadParams params;
    const threadReturn *ret;
} threadParamsWithKey;

#define DOLOOP \
{\
for (u32 j = 0; j < Config.nThread; j++)\
    params[j].Start.post();\
for (u32 j = 0; j < Config.nThread; j++)\
    params[j].Finish.wait();\
}

#define DOLOOP_KEY \
{\
for (u32 j = 0; j < Config.nThread; j++)\
    params[j].params.Start.post();\
for (u32 j = 0; j < Config.nThread; j++)\
    params[j].params.Finish.wait();\
}

#define ORIGINAL_SIZE cv::Size{0, 0}
#define RANDOM_KEYS Keys{Rand16(), GenDoubleFloatFrom1To0Random(), GenDoubleFloatFrom1To0Random() / 2, GenDoubleFloatFrom1To0Random(), GenDoubleFloatFrom1To0Random() / 2}
#define DEFAULT_CONFIG ParamControl{6, 200, 3, 5, 8}

#endif //INCLUDES_H
