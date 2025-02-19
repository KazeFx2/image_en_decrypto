//
// Created by Kaze Fx on 2023/11/25.
//

#ifndef SETMEM_H
#define SETMEM_H

#include <stdio.h>

#define FOREACH(type, i, head) for (type *i = (type *)((list_t)(head))->next; i; i = (type *)((node_t *)i)->next)

extern void *(*__malloc)(size_t);

extern void (*__free)(void *);

void setMalloc(void *(*_malloc)(size_t));

void setFree(void (*_free)(void *));

void *(*getMalloc())

(size_t);

void (*getFree())(void *);

#endif //SETMEM_H
