//
// Created by Fx Kaze on 24-12-30.
//

#ifndef RANDOM_H
#define RANDOM_H

#include <random>
#include "types.h"

u64 Rand64();

u32 Rand32();

u16 Rand16();

u8 Rand8();

f64 GenDoubleFloatFrom0To1(u64 Val52Bits);

f64 GenDoubleFloatFrom1To0Random();

#endif //RANDOM_H
