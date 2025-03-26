//
// Created by Fx Kaze on 25-3-20.
//

#include <string.h>
#include "string_ref_list.h"
#include "split_line_module/split_line_module.h"
#include "setmem.h"

int offset_to_row_col(
    size_t offset,
    list_t line_num_list,
    size_t *row, size_t *col
) {
    line_num_t *line_num = NULL;

    *row = *col = 0;
    if (!line_num_list) { return EOF; }
    FOREACH(node_t, i, line_num_list) {
        line_num = (line_num_t *) get_data(i);
        if (offset > line_num->line_length)
            offset -= line_num->line_length;
        else if (offset == line_num->line_length) {
            *row = line_num->line_num + 1;
            *col = 0;
            return 0;
        } else {
            *row = line_num->line_num;
            *col = offset;
            return 0;
        }
    }
    return EOF;
}

int stick_push_to_list(
    const char *line,
    size_t len,
    size_t row, size_t col,
    list_t str_list
) {
    pos_string_t *prev_data = NULL;

    prev_data = *(pos_string_t **) get_data(get_last(str_list));
    if (prev_data->string.str[0] == '\0')
        prev_data->position.row = row, prev_data->position.col = col;
    string_n_cat(&prev_data->string, line, len);
    return 0;
}

int push_to_list(
    const char *line,
    size_t len,
    size_t row,
    size_t col,
    list_t str_list,
    size_t *n_word,
    bool replace_empty
) {
    pos_string_t *node_data;
    pos_string_t *prev_data = NULL;

    if (replace_empty) {
        prev_data = *(pos_string_t **) get_data(get_last(str_list));
        if (prev_data->string.str[0] == '\0') {
            free_string(&prev_data->string);
            prev_data->string = mk_string(line, len);
            return 0;
        }
    }
    node_data = (pos_string_t *) __malloc(sizeof(pos_string_t));
    if (!node_data) { return EOF; }
    node_data->position.row = row;
    node_data->position.col = col;
    node_data->string = mk_string(line, len);
    if (node_data->string.str == NULL) {
        return EOF;
    }
    if (push_end(
            str_list,
            &node_data,
            sizeof(pos_string_t *)
        ) == false)
        return EOF;
    (*n_word)++;
    return 0;
}

int push_n_char_to_list(
    const char *line,
    size_t offset,
    list_t str_list,
    size_t *n_word,
    list_t line_num_list,
    size_t n,
    bool allow_zero,
    bool stick_prev) {
    pos_string_t *node_data;

    if (!allow_zero && n == 0) return 0;
    if (stick_prev) {
        node_data = *(pos_string_t **) get_data(get_last(str_list));
        return string_n_cat(&node_data->string, line + offset, n);
    }
    node_data = (pos_string_t *) __malloc(sizeof(pos_string_t));
    if (!node_data) { return EOF; }
    if (offset_to_row_col(offset, line_num_list, &node_data->position.row, &node_data->position.col))
        return EOF;
    node_data->string = mk_string(line + offset, n);
    if (node_data->string.str == NULL) {
        return EOF;
    }
    if (push_end(
            str_list,
            &node_data,
            sizeof(pos_string_t *)
        ) == false)
        return EOF;
    (*n_word)++;
    return 0;
}

static void init_string(node_t *node) {
    if (!node) return;
    string_node_t *str_node = (string_node_t *) node;
    str_node->data->ref = 1;
}

static void add_string(node_t *node) {
    if (!node) return;
    string_node_t *str_node = (string_node_t *) node;
    str_node->data->ref++;
}

static void remove_string(node_t *node) {
    if (!node) return;
    string_node_t *str_node = (string_node_t *) node;
    str_node->data->ref--;
    if (str_node->data->ref == 0) {
        free_string(&str_node->data->string);
        __free(str_node->data);
        // node will be freed by the list function
    }
}

string_t mk_string(const char *str, size_t len) {
    string_t ret;
    ret.type = ALLOC;
    ret.str = (char *) __malloc(len + 1);
    memcpy((void *)ret.str, str, len);
    ((char *) ret.str)[len] = '\0';
    ret.len = len;
    return ret;
}

void free_string(string_t *str) {
    if (str->type == ALLOC) {
        __free((void *) str->str);
    }
}

list_t init_c_string_ref_list() {
    return init_list(
        CLIST_WORD_REF_LIST,
        init_string,
        add_string,
        remove_string
    );
}

int string_n_cat(string_t *str1, const char *str2, size_t len) {
    char *new_str = (char *) __realloc((void *) str1->str, str1->len + len + 1);
    if (!new_str) return EOF;
    str1->str = new_str;
    memcpy(new_str + str1->len, str2, len);
    new_str[str1->len + len] = '\0';
    str1->len += len;
    return 0;
}

int c_string_n_cmp(const char *str1, const char *str2, size_t len) {
    while (len--) {
        if (*str1 < *str2) return -1;
        if (*str1 > *str2) return 1;
        str1++, str2++;
    }
    return 0;
}

int string_n_cmp(string_t str1, string_t str2, size_t len) {
    const char *p1 = NULL, *p2 = NULL;

    p1 = str1.str, p2 = str2.str;
    while (len--) {
        if (*p1 < *p2) return -1;
        if (*p1 > *p2) return 1;
        p1++, p2++;
    }
    return 0;
}

int string_cmp(string_t str1, string_t str2) {
    int ret = string_n_cmp(str1, str2, min(str1.len, str2.len));
    if (ret == 0) {
        if (str1.len < str2.len) {
            return -1;
        }
        if (str1.len > str2.len) {
            return 1;
        }
        return 0;
    }
    return ret;
}

int c_string_cmp(const char *str1, string_t str2) {
    return c_string_n_cmp(str1, str2.str, str2.len);
}
