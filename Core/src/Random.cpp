//
// Created by Fx Kaze on 24-12-30.
//

#include "private/Random.h"

using namespace std;
random_device rd;
default_random_engine engine(rd());

uniform_int_distribution<uint64_t> dist64(0, UINT64_MAX);
uniform_int_distribution<uint32_t> dist32(0, UINT32_MAX);
uniform_int_distribution<uint16_t> dist16(0, UINT16_MAX);
uniform_int_distribution<uint8_t> dist8(0, UINT8_MAX);

uint64_t Rand64() {
    return dist64(engine);
}

uint32_t Rand32() {
    return dist32(engine);
}

uint16_t Rand16() {
    return dist16(engine);
}

uint8_t Rand8() {
    return dist8(engine);
}

double GenDoubleFloatFrom0To1(const uint64_t Val52Bits) {
    // sign: 1;
    // level code: 11
    uint64_t hex = static_cast<uint64_t>(0b001111111110) << 52 | Val52Bits & 0xfffffffffffff;
    return reinterpret_cast<double*>(&hex)[0];
}

double GenDoubleFloatFrom1To0Random() {
    return GenDoubleFloatFrom0To1(Rand64());
}
