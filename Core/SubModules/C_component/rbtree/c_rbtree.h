//
// Created by Kaze Fx on 2023/11/25.
//

#ifndef C_RBTREE_H
#define C_RBTREE_H

#include <stdio.h>
#include "c_types.h"

typedef enum {
    RB_Red = 0,
    RB_Black,
    RB_Head
} tree_color_t;

typedef struct rbtree_node_s {
    tree_color_t color;
    struct rbtree_node_s *left, *right, *parent;
    uintptr_t comparable;
} rbtree_node_t, *rbtree_t;

rbtree_t init_rbtree();

bool rbtree_add_node(rbtree_t head, uintptr_t comparable_value, void *data, size_t data_size);

bool rbtree_add_exist_node(rbtree_t head, uintptr_t comparable_value, rbtree_node_t *node);

bool rbtree_delete_node(rbtree_t head, rbtree_node_t *target);

rbtree_node_t *rbtree_get_node(rbtree_t head, uintptr_t key);

bool rbtree_is_correct(rbtree_t head, uint depth_now, bool forceBlack);

#ifdef __cplusplus
#include "undef.h"
#endif

#endif //C_RBTREE_H
