//
// Created by Fx Kaze on 25-3-20.
//

#ifndef C_STRING_REF_LIST_H
#define C_STRING_REF_LIST_H

#define CLIST_WORD_REF_LIST 2

#include "c_list.h"

typedef struct position_s {
    size_t row;
    size_t col;
} position_t;

typedef enum string_type_e {
    CONST,
    ALLOC
} string_type_t;

typedef struct string_s {
    const char *str;
    size_t len;
    string_type_t type;
} string_t;

typedef struct pos_string_s {
    string_t string;
    position_t position;
    size_t ref;
} pos_string_t;

typedef struct string_node_s {
    node_t node;
    pos_string_t *data;
} string_node_t;

static int offset_to_row_col(
    size_t offset,
    list_t line_num_list,
    size_t *row, size_t *col
);

int stick_push_to_list(
    const char *line,
    size_t len,
    size_t row, size_t col,
    list_t str_list
);

int push_to_list(
    const char *line,
    size_t len,
    size_t row,
    size_t col,
    list_t str_list,
    size_t *n_word,
    bool replace_empty
);

int push_n_char_to_list(
    const char *line,
    size_t offset,
    list_t str_list,
    size_t *n_word,
    list_t line_num_list,
    size_t n,
    bool allow_zero,
    bool stick_prev);

string_t mk_string(const char *str, size_t len);

void free_string(string_t *str);

#define MK_STRING(str) {(str), (sizeof(str) - 1), CONST}

list_t init_c_string_ref_list();

int string_n_cat(string_t *str1, const char *str2, size_t len);

int c_string_n_cmp(const char *str1, const char *str2, size_t len);

int string_n_cmp(string_t str1, string_t str2, size_t len);

int string_cmp(string_t str1, string_t str2);

int c_string_cmp(const char *str1, string_t str2);

#endif //C_STRING_REF_LIST_H
