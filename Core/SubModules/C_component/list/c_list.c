//
// Created by Kaze Fx on 2023/11/23.666
//

#define C_SOURCE

#include "c_list.h"
#include "setmem.h"

#include <string.h>

#ifdef _DEBUG

#include "stdio.h"

#endif

#define SWAP(type, a, b) {type tmp = a; a = b; b = tmp;}

#define SET_NODE(node) {if ((node)->prev) (node)->prev->next = (node); if((node)->next) (node)->next->prev = (node);}

#define CHECK_TYPE(head, node) (((node_t *)(head))->type == ((node_t *)(node))->type)

#define SET_TYPE(head, node) (((node_t *)(node))->type = ((node_t *)(head))->type)

#define true 1
#define false 0

bool check_node(list_t head, node_t **node) {
    if (!node)
        return false;
    if (!*node)
        return true;
    if (!CHECK_TYPE(head, *node))
        return false;
    if (!*node || *node == (node_t *) head)
        return false;
    return true;
}

list_t init_list(const size_t type, void (*on_init)(node_t *), void (*on_add)(node_t *), void (*on_remove)(node_t *)) {
    list_t head = __malloc(sizeof(struct list_s));
    if (!head)
        return NULL;
    head->node.prev = head->node.next = NULL;
    head->node.type = type;
    head->on_init = on_init;
    head->on_add = on_add;
    head->on_remove = on_remove;
    return head;
}

list_t init_list_default() {
    return init_list(CLIST_BASIC, NULL, NULL, NULL);
}

node_t **find_node(list_t _head, const void *target) {
    if (!_head)
        return NULL;
    if (!CHECK_TYPE(_head, _head))
        return NULL;
    node_t **head = (node_t **) &_head;
    head = &((*head)->next);
    while (*head != target && *head)
        head = &((*head)->next);
    if (*head == target)
        return head;
    return NULL;
}

node_t **find_node_by_index(list_t _head, size_t index) {
    if (!_head)
        return NULL;
    list_t *head = &_head;
    head = (list_t *) &(*head)->node.next;
    if (index == 0)
        return (node_t **) head;
    while (*head && index != 0)
        head = (list_t *) &(*head)->node.next, index--;
    if (index == 0)
        return (node_t **) head;
    return NULL;
}

node_t *find_p_node_by_index(list_t head, size_t index) {
    node_t **ret = find_node_by_index(head, index);
    if (!check_node(head, ret))
        return NULL;
    return *ret;
}

list_t find_head(node_t *node) {
    if (!node)
        return NULL;
    while (node->prev)
        node = node->prev;
    return (list_t) node;
}

node_t *next_n(node_t *node, size_t next_n) {
    while (node && next_n--)
        node = node->next;
    return node;
}

bool push_node_before(list_t _head, void *target, const void *data, size_t data_size) {
    node_t **head = find_node(_head, target);
    return push_node_before2(_head, (void **) head, data, data_size);
}

bool push_node_before2(list_t _head, void **_target, const void *data, size_t data_size) {
    node_t **head = (node_t **) _target;
    if (!head)
        return false;
    node_t *target = *head;
    *head = __malloc(data_size + sizeof(node_t));
    if (!*head) {
        *head = target;
        return false;
    }
    SET_TYPE(_head, *head);
    (*head)->next = target;
    (*head)->prev = op_ptr(head, -offset_of(node_t, next));
    if (target)
        target->prev = *head;
    if (!data)
        return false;
    memcpy(get_data((*head)), data, data_size);
    if (_head->on_init) {
        _head->on_init(*head);
    }
    return true;
}

bool push_exist_node_before(list_t _head, void *target, node_t *node) {
    node_t **head = find_node(_head, target);
    return push_exist_node_before2(_head, (void **) head, node);
}

bool push_exist_node_before2(list_t _head, void **_target, node_t *node) {
    node_t **head = (node_t **) _target;
    if (!head)
        return false;
    node_t *target = *head;
    if (!node)
        return false;
    *head = node;
    if (!*head) {
        *head = target;
        return false;
    }
    SET_TYPE(_head, *head);
    if (_head->on_add) {
        _head->on_add(*head);
    }
    (*head)->next = target;
    (*head)->prev = op_ptr(head, -offset_of(node_t, next));
    if (target)
        target->prev = *head;
    return true;
}

bool push_exist_compared_before(list_t _head, node_t *node, compare_func func, bool asc) {
    FOREACH(node_t, i, _head) {
        if (asc) {
            if (func(node, i) < 0) {
                return push_exist_node_before(_head, i, node);
            }
        } else if (func(node, i) > 0) {
            return push_exist_node_before(_head, i, node);
        }
    }
    return push_exist_node_before(_head, NULL, node);
}

bool push_compared_before(list_t _head, const void *data, size_t data_size, compare_func func, bool asc) {
    node_t *new = (node_t *) __malloc(sizeof(node_t) + data_size);
    if (!new)
        return false;
    memcpy(get_data(new), data, data_size);
    if (!push_exist_compared_before(_head, new, func, asc)) {
        __free(new);
        return false;
    }
    return true;
}

bool push_end(list_t head, const void *data, size_t data_size) {
    return push_node_before(head, NULL, data, data_size);
}

bool push_exist_end(list_t head, node_t *node) {
    return push_exist_node_before(head, NULL, node);
}

bool push_begin(list_t head, const void *data, size_t data_size) {
    if (!head)
        return false;
    return push_node_before(head, head->node.next, data, data_size);
}

bool push_exist_begin(list_t head, node_t *node) {
    if (!head)
        return false;
    return push_exist_node_before(head, head->node.next, node);
}

bool remove_node(node_t *node, bool ref) {
    list_t head = find_head(node);
    if (!head)
        return false;
    if (!node->prev)
        return false;
    if (node->next)
        node->next->prev = node->prev;
    node->prev->next = node->next;
    if (!ref && head->on_remove) {
        head->on_remove(node);
    }
    return true;
}

bool remove_by_index(list_t head, size_t index, bool ref) {
    node_t **ret = find_node_by_index(head, index);
    if (!ret || *ret == (node_t *) head)
        return false;
    node_t *node = *ret;
    if (!node)
        return false;
    return remove_node(node, ref);
}

bool push_into(list_t head, size_t index, const void *data, size_t data_size) {
    return push_node_before2(head, (void **) find_node_by_index(head, index), data, data_size);
}

bool push_exist_into(list_t head, size_t index, node_t *node) {
    return push_exist_node_before2(head, (void **) find_node_by_index(head, index), node);
}

node_t *pop_from(list_t head, size_t index) {
    node_t **node = find_node_by_index(head, index);
    if (!node)
        return NULL;
    node_t *ret = *node;
    remove_node(ret, true);
    return ret;
}

node_t *pop_begin(list_t head) {
    return pop_from(head, 0);
}

node_t *pop_end(list_t head) {
    node_t *del = op_ptr(find_node(head, NULL), -offset_of(node_t, next));
    remove_node(del, true);
    return del;
}

bool swap_nodes(node_t *a, node_t *b) {
    if (!a || !b || !a->prev || !b->prev)
        return false;
    SWAP(node_t *, a->next, b->next)
    SWAP(node_t *, a->prev, b->prev)
    SET_NODE(a)
    SET_NODE(b)
    return true;
}

bool swap_nodes_by_index(list_t head, size_t a, size_t b) {
    node_t *na = find_p_node_by_index(head, a), *nb = find_p_node_by_index(head, b);
    if (!na || !nb)
        return false;
    return swap_nodes(na, nb);
}

bool move_from_to(list_t head, size_t from, size_t to) {
    if (from == to)
        return true;
    node_t **a = find_node_by_index(head, from);
    node_t **b = find_node_by_index(head, to < from ? to : to + 1);
    if (!a || !*a || !b)
        return false;
    (*a)->prev->next = (*a)->next;
    if ((*a)->next)
        (*a)->next->prev = (*a)->prev;
    return push_exist_node_before2(head, (void **) b, *a);
}

bool remove_from(list_t head, size_t index) {
    node_t **node = find_node_by_index(head, index);
    if (!node)
        return false;
    node_t *ret = *node;
    return remove_node(ret, false);
}

bool remove_begin(list_t head) {
    return remove_from(head, 0);
}

bool remove_end(list_t head) {
    node_t *del = op_ptr(find_node(head, NULL), -offset_of(node_t, next));
    return remove_node(del, false);
}

bool unref_node(list_t head, node_t *node) {
    if (!node)
        return false;
    if (!head)
        return false;
    if (head->on_remove)
        head->on_remove(node);
    return true;
}

bool clear_list(list_t head) {
    if (!head)
        return false;
    while (head->node.next) {
        void *n = head->node.next;
        remove_node(head->node.next, false);
        __free(n);
    }
    return true;
}

bool destroy_list(list_t head) {
    if (!clear_list(head))
        return false;
    __free(head);
    return true;
}

size_t list_size(list_t head) {
    size_t len = 0;
    while (head->node.next)
        head = (list_t) head->node.next, len++;
    return len;
}

list_t sort_by(list_t old, compare_func func, size_t data_size, bool asc) {
    if (!old)
        return NULL;
    if (old->node.type != CLIST_BASIC)
        return NULL;
    list_t new = init_list_default();
    if (!new)
        return NULL;
    FOREACH(node_t, i, old) {
        if (!push_compared_before(new, get_data(i), data_size, func, asc))
            goto err;
    }
    return new;
err:
    destroy_list(new);
    return NULL;
}

list_t array_to_list(const void *data, size_t data_size, size_t n_elem) {
    list_t ret = init_list_default();
    size_t i = 0;
    if (!ret) return NULL;
    if (n_elem) {
        for (i = 0; i < n_elem; i++) {
            if (push_end(
                    ret, data, data_size
                ) != true)
                goto clean_up;
            data = op_ptr(data, +data_size);
        }
    }
    return ret;
clean_up:
    destroy_list(ret);
    return NULL;
}
