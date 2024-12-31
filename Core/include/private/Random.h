//
// Created by Fx Kaze on 24-12-30.
//

#ifndef RANDOM_H
#define RANDOM_H

#include <random>

uint64_t Rand64();
uint32_t Rand32();
uint16_t Rand16();
uint8_t Rand8();

double GenDoubleFloatFrom0To1(uint64_t Val52Bits);

double GenDoubleFloatFrom1To0Random();

#endif //RANDOM_H
