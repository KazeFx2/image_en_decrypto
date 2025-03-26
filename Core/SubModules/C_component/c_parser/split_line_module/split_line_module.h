//
// Created by Fx Kaze on 25-3-21.
//

#ifndef SPLIT_LINE_MODULE_H
#define SPLIT_LINE_MODULE_H

#include "c_parser.h"
#include "split_line_types.h"

typedef struct line_num_s {
    size_t line_num;
    size_t line_length;
} line_num_t;

typedef struct split_line_module_s {
    init_split_line_module_func init;
    split_line_module_func solve;
    destroy_split_line_module_func destroy;
} split_line_module_t;

extern split_line_module_t split_line_modules[];

void **init_split_line_data(struct parser_conf_s *conf);

int split_line(
    const char *file,
    list_t string_list,
    size_t *n_word,
    const char *line,
    list_t line_num_list,
    size_t total_length,
    struct parser_conf_s *conf,
    void **data
);

void destroy_split_line_data(struct parser_conf_s *conf, void **data);

#endif //SPLIT_LINE_MODULE_H
