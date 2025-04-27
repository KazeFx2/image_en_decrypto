//
// Created by Fx Kaze on 24-12-30.
//

#ifndef RANDOM_H
#define RANDOM_H

#include <random>
#include "types.h"

extern u64 (*Rand64)();
extern u32 (*Rand32)();
extern u16 (*Rand16)();
extern u8 (*Rand8)();

f64 GenDoubleFloatFrom0To1(u64 Val52Bits);

f64 GenDoubleFloatFrom1To0Random();

#endif //RANDOM_H
