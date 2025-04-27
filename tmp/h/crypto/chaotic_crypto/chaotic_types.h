//
// Created by Fx Kaze on 25-3-27.
//

#ifndef CHAOTIC_TYPES_H
#define CHAOTIC_TYPES_H

#include "../macro.h"
#include "types.h"
#include "Semaphore.h"
#include "../random.h"

typedef struct {
    f64 initCondition;
    f64 ctrlCondition;
} ChaoticParamGroup;

#define CHAOTIC_NO_SHAPE 0, 0, 0

typedef struct {
    u8 byteReserve;
    u32 preIterations;
    u32 confusionIterations;
    u32 diffusionConfusionIterations;
    u32 nThread;
} ChaoticControl;

#define CHAOTIC_DEFAULT_CTRL 5, 200, 3, 4, 16

typedef struct {
    u32 confusionSeed;
    ChaoticParamGroup gParam1;
    ChaoticParamGroup gParam2;
} ChaoticKeys;

#define CHAOTIC_RAND_KEY GenDoubleFloatFrom1To0Random(), GenDoubleFloatFrom1To0Random() / 2
#define CHAOTIC_RAND_KEYS Rand32(), {CHAOTIC_RAND_KEY}, {CHAOTIC_RAND_KEY}

typedef struct {
    __OUT u8 **dst;
    __IN u8 **src;
    __IN size_t threadId;
    __IN u32 iterations;
    __IN const ChaoticKeys *keys;
    __IN const ChaoticControl *config;
    __IN Semaphore Start;
    __IN Semaphore Finish;
    __IN bool cuda;
} ChaoticThreadParams;

typedef struct {
    __OUT u8 *byteSeq;
    __OUT u8 *diffusionSeedArray;
} ChaoticThreadReturn;

typedef struct {
    __IN u32 nThread;
    __OUT ChaoticThreadReturn **threadReturn;
} ChaoticReturns;

typedef struct {
    ChaoticThreadParams params;
    const ChaoticThreadReturn *ret;
} ChaoticThreadParamsWithKey;

#endif //CHAOTIC_TYPES_H
