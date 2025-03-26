//
// Created by Fx Kaze on 25-3-21.
//

#ifndef SPLIT_LINE_TYPES_H
#define SPLIT_LINE_TYPES_H

#include "c_types.h"
#include "c_list.h"

struct parser_conf_s;

/// `return` if matched the length of key word will be returned, else 0
typedef size_t (*string_match_func)(
    const char *,
    size_t offset,
    size_t total_length,
    bool *push_prev,
    bool *push_word,
    void *data
);

typedef void *(*init_split_line_module_func)(struct parser_conf_s *);

typedef size_t (*split_line_module_func)(
    /// file name,
    const char *,
    /// raw string
    const char *,
    /// start offset
    size_t,
    /// current offset
    size_t,
    /// line total length
    size_t,
    /// word_list
    list_t,
    /// n_word
    size_t *,
    /// line_num_list
    list_t,
    /// parser_conf
    struct parser_conf_s *,
    /// private_data
    void *
);

typedef void (*destroy_split_line_module_func)(struct parser_conf_s *, void *);

#endif //SPLIT_LINE_TYPES_H
