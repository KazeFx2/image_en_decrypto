//
// Created by Fx Kaze on 25-3-21.
//
#include "split_line_module.h"
#include "setmem.h"
#include "quote_match/quote_match.h"
#include "symbol_match/symbol_match.h"

split_line_module_t split_line_modules[] = {
    QUOTE_MATCH_MODULE,
    SYMBOL_MATCH_MODULE
};

void **init_split_line_data(parser_conf_t *conf) {
    size_t n_modules = sizeof(split_line_modules) / sizeof(split_line_module_t), i = 0;
    void **data = (void **) __malloc(n_modules * sizeof(void *));

    for (i = 0; i < n_modules; i++) {
        if (split_line_modules[i].init) {
            data[i] = split_line_modules[i].init(conf);
        } else
            data[i] = NULL;
    }
    return data;
}

int split_line(
    const char *file,
    list_t string_list,
    size_t *n_word,
    const char *line,
    list_t line_num_list,
    size_t total_length,
    parser_conf_t *conf,
    void **data
) {
    size_t n_modules = sizeof(split_line_modules) / sizeof(split_line_module_t), i = 0, start_offset = 0, current_offset
            = 0, new_offset = 0;

    if (!data) {
        return EOF;
    }
    while (start_offset <= total_length && current_offset <= total_length) {
        for (i = 0; i < n_modules; i++) {
            if (split_line_modules[i].solve) {
                new_offset = split_line_modules[i].solve(
                    file,
                    line,
                    start_offset,
                    current_offset,
                    total_length,
                    string_list,
                    n_word,
                    line_num_list,
                    conf,
                    data[i]
                );
                if (new_offset > current_offset) {
                    current_offset = new_offset;
                    start_offset = current_offset;
                    break;
                }
            }
        }
        if (i >= n_modules)
            current_offset++;
    }
    return 0;
}

void destroy_split_line_data(parser_conf_t *conf, void **data) {
    size_t n_modules = sizeof(split_line_modules) / sizeof(split_line_module_t), i = 0;

    if (!data) return;
    for (i = 0; i < n_modules; i++) {
        if (data[i] && split_line_modules[i].destroy) {
            split_line_modules[i].destroy(conf, data[i]);
        }
    }
    __free(data);
}
