//
// Created by Kaze Fx on 2023/11/23.
//

#ifndef CLIST_H
#define CLIST_H

#include "c_types.h"

#define get_first(list) ((node_t *)(((node_t *)(list))->next))

#define get_last(list) ((node_t *)op_ptr(find_node(list, NULL), -offsetof(node_t, next)))

#define get_data(node) ((void *)(((node_t *)(node)) + 1))

#define FOREACH(type, i, head) for (type *i = (type *)((node_t *)(head))->next; i; i = (type *)((node_t *)i)->next)

#define ASC true
#define DESC false

typedef enum list_type_e {
    CLIST_BASIC
} list_type_t;

typedef struct node_s {
    size_t type;
    struct node_s *prev;
    struct node_s *next;
} node_t;

typedef struct list_s {
    node_t node;

    void (*on_init)(node_t *);

    void (*on_add)(node_t *);

    void (*on_remove)(node_t *);
} *list_t;

typedef int (*compare_func)(node_t *, node_t *);

list_t init_list(size_t type, void (*on_init)(node_t *), void (*on_add)(node_t *),
                 void (*on_remove)(node_t *));

list_t init_list_default();

node_t **find_node(list_t _head, const void *target);

node_t **find_node_by_index(list_t head, size_t index);

node_t *find_p_node_by_index(list_t head, size_t index);

list_t find_head(node_t *node);

node_t *next_n(node_t *node, size_t next_n);

bool push_end(list_t head, const void *data, size_t data_size);

bool push_exist_end(list_t head, node_t *node);

bool push_begin(list_t head, const void *data, size_t data_size);

bool push_exist_begin(list_t head, node_t *node);

bool push_node_before(list_t _head, void *target, const void *data, size_t data_size);

bool push_node_before2(list_t head, void **target, const void *data, size_t data_size);

bool push_exist_node_before(list_t _head, void *target, node_t *node);

bool push_exist_node_before2(list_t head, void **target, node_t *node);

bool push_exist_compared_before(list_t _head, node_t *node, compare_func func, bool asc);

bool push_compared_before(list_t _head, const void *data, size_t data_size, compare_func func, bool asc);

bool remove_node(node_t *node, bool ref);

bool remove_by_index(list_t head, size_t index, bool ref);

bool push_into(list_t head, size_t index, const void *data, size_t data_size);

bool push_exist_into(list_t head, size_t index, node_t *node);

node_t *pop_from(list_t head, size_t index);

node_t *pop_begin(list_t head);

node_t *pop_end(list_t head);

bool swap_nodes(node_t *a, node_t *b);

bool swap_nodes_by_index(list_t head, size_t a, size_t b);

bool move_from_to(list_t head, size_t from, size_t to);

bool remove_begin(list_t head);

bool remove_from(list_t head, size_t index);

bool remove_end(list_t head);

bool unref_node(list_t head, node_t *node);

bool clear_list(list_t head);

bool destroy_list(list_t head);

size_t list_size(list_t head);

list_t sort_by(list_t old, compare_func func, size_t data_size, bool asc);

list_t array_to_list(const void *data, size_t data_size, size_t n_elem);

#ifdef __cplusplus
#include "undef.h"
#endif

#endif //CLIST_H
