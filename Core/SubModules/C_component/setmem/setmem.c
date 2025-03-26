//
// Created by Kaze Fx on 2023/11/25.
//

#include <setmem.h>
#include <string.h>
#include <stdlib.h>

static void *t_malloc(const size_t size) {
    if (size == 0) {
        return NULL;
    }
    size_t *ptr = (size_t *) malloc(size + sizeof(size_t));
    if (!ptr) {
        fprintf(stderr, "Malloc failed, size: '%lu'\n", size);
        return NULL;
    }
    *ptr = size;
    return ptr + 1;
}

static void t_free(void *ptr) {
    if (!ptr) return;
    free(((size_t *) ptr) - 1);
}

static void *t_realloc(void *ptr, const size_t size) {
    if (!ptr) { return NULL; }
    size_t *ptr_size = (((size_t *) ptr) - 1), cpy_size = 0;
    void *new_ptr = __malloc(size);
    if (!new_ptr) {
        return NULL;
    }
    cpy_size = *ptr_size < size ? *ptr_size : size;
    memcpy(new_ptr, ptr, cpy_size);
    free(ptr_size);
    return new_ptr;
}

void *(*__malloc)(size_t) = t_malloc;

void (*__free)(void *) = t_free;

void *(*__realloc)(void *, size_t) = t_realloc;

void set_malloc(void *(*_malloc)(size_t)) {
    __malloc = _malloc;
}

void set_free(void (*_free)(void *)) {
    __free = _free;
}

void set_realloc(void *(*_realloc)(void *, size_t)) {
    __realloc = _realloc;
}

void *(*get_malloc())(size_t) {
    return __malloc;
}

void (*get_free())(void *) {
    return __free;
}

void *(*get_realloc())(void *, size_t) {
    return __realloc;
}
