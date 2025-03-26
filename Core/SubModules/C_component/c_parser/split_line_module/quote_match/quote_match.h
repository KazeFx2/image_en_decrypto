//
// Created by Fx Kaze on 25-3-20.
//

#ifndef C_QUOTE_MATCH_H
#define C_QUOTE_MATCH_H

#include "split_line_module/split_line_types.h"
#include "c_string_ref_list/string_ref_list.h"
#include "split_line_module/split_line_module.h"

#define QUOTE_MATCH_MODULE\
    {\
        init_quote_match_module,\
        quote_match_module_func,\
        destroy_quote_match_module\
    }

typedef struct quote_match_module_s {
    /// init module data
    init_split_line_module_func init;
    /// destroy module data
    destroy_split_line_module_func destroy;
    /// match start symbol
    string_match_func start_match;
    /// match end symbol
    string_match_func end_match;
    /// match escape symbol
    string_match_func escape_match;
    /// whether escape end symbol
    bool escape;
} quote_match_module_t;

extern quote_match_module_t quote_match_modules[];

typedef struct quote_sample_match_s {
    string_t start_str;
    string_t end_str;
    string_t escape_str;
    bool escape;
} quote_sample_match_t;

typedef struct quote_line_endless_match_s {
    string_t start_str;
} quote_line_endless_match_t;

void *init_quote_match_module(struct parser_conf_s *conf);

size_t quote_match_module_func(
    const char *file,
    const char *line,
    size_t start_offset,
    size_t current_offset,
    size_t total_length,
    list_t str_list,
    size_t *n_word,
    list_t line_num_list,
    struct parser_conf_s *conf,
    void *data);

void destroy_quote_match_module(struct parser_conf_s *, void *data);

#endif //C_QUOTE_MATCH_H
