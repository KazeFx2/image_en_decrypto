//
// Created by Kaze Fx on 2023/11/25.
//

#ifndef C_COMPONENT_TYPES_H
#define C_COMPONENT_TYPES_H

#include "stddef.h"

#define bool char
typedef unsigned int uint;
typedef unsigned int bm_pid_t;

#define true 1
#define false 0

#define op_ptr(ptr, op) ((void *)(((uintptr_t)ptr) op))
#define offset_of(type, field) ((uintptr_t)&(((type *)NULL)->field))

#define min(a, b) (((a) < (b)) ? (a) : (b))

#endif //C_COMPONENT_TYPES_H
