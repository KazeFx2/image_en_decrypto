//
// Created by Kaze Fx on 2023/11/25.
//

#ifndef SETMEM_H
#define SETMEM_H

#include <stdio.h>

extern void *(*__malloc)(size_t);

extern void (*__free)(void *);

/// malloc new buffer, memcpy, then free old (if success)
extern void *(*__realloc)(void *, size_t);

void set_malloc(void *(*_malloc)(size_t));

void set_free(void (*_free)(void *));

void set_realloc(void *(*_realloc)(void *, size_t));

void *(*get_malloc())(size_t);

void (*get_free())(void *);

void *(*get_realloc())(void *, size_t);

#endif //SETMEM_H
