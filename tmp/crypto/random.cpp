//
// Created by Fx Kaze on 25-3-27.
//
#include "private/crypto/random.h"

using namespace std;
random_device rd;
// default_random_engine engine(rd());
mt19937 engine(rd());

uniform_int_distribution<u64> dist64(0, UINT64_MAX);
uniform_int_distribution<u32> dist32(0, UINT32_MAX);
uniform_int_distribution<u16> dist16(0, UINT16_MAX);
#ifndef _WIN32
uniform_int_distribution<u8> dist8(0, UINT8_MAX);
#else
uniform_int_distribution<u16> dist8(0, UINT8_MAX);
#endif

static u64 __Rand64() {
    return dist64(engine);
}

static u32 __Rand32() {
    return dist32(engine);
}

static u16 __Rand16() {
    return dist16(engine);
}

static u8 __Rand8() {
    return dist8(engine);
}

u64 (*Rand64)() = __Rand64;

u32 (*Rand32)() = __Rand32;

u16 (*Rand16)() = __Rand16;

u8 (*Rand8)() = __Rand8;

f64 GenDoubleFloatFrom0To1(const u64 Val52Bits) {
    // sign: 1;
    // level code: 11
    u64 hex = static_cast<u64>(0b001111111110) << 52 | Val52Bits & 0xfffffffffffff;
    return reinterpret_cast<double *>(&hex)[0];
}

f64 GenDoubleFloatFrom1To0Random() {
    return GenDoubleFloatFrom0To1(Rand64());
}

