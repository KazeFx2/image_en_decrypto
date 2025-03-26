//
// Created by Fx Kaze on 25-3-20.
//

#include "symbol_match.h"
#include "setmem.h"
#include "split_line_module/symbol_match/split_symbol_match/split_symbol_match.h"
#include "split_line_module/symbol_match/key_symbol_match/key_symbol_match.h"

symbol_match_module_t symbol_match_modules[] = {
    SPLIT_SYMBOL_MATCH_MODULE,
    KEY_SYMBOL_MATCH_MODULE
};

typedef struct symbol_match_module_data_s {
    void **data;
} symbol_match_module_data_t;

void *init_symbol_match_module(parser_conf_t *conf) {
    symbol_match_module_data_t *data = NULL;
    size_t n_modules = sizeof(symbol_match_modules) / sizeof(symbol_match_module_t), i = 0;

    data = __malloc(sizeof(symbol_match_module_data_t));
    if (!data) goto clean_up;
    data->data = __malloc(n_modules * sizeof(void *));
    if (!data->data) goto clean_up;
    for (i = 0; i < n_modules; i++) {
        if (symbol_match_modules[i].init) {
            data->data[i] = symbol_match_modules[i].init(conf);
        } else {
            data->data[i] = NULL;
        }
    }
    return data;

clean_up:
    if (!data) return NULL;
    if (data->data) {
        for (i = 0; i < n_modules; i++) {
            if (data->data[i] && symbol_match_modules[i].destroy)
                symbol_match_modules[i].destroy(conf, data->data[i]);
        }
        __free(data);
    }
    return NULL;
}

size_t symbol_match_module_func(
    const char *file,
    const char *line,
    size_t start_offset,
    size_t current_offset,
    size_t total_length,
    list_t str_list,
    size_t *n_symbol,
    list_t line_num_list,
    parser_conf_t *conf,
    void *m_data) {
    size_t n_modules = sizeof(symbol_match_modules) / sizeof(symbol_match_module_t), n_match = 0, i =
            0;
    symbol_match_module_data_t *data = (symbol_match_module_data_t *) m_data;
    line_num_t *line_num = NULL;
    bool push_prev = true, push_symbol = true;

    if (!data) return current_offset;
    for (i = 0; i < n_modules; i++) {
        if (!symbol_match_modules[i].symbol_match)
            continue;
        n_match = symbol_match_modules[i].symbol_match(line, current_offset, total_length, &push_prev, &push_symbol,
                                                       data->data[i]);
        if (n_match > 0) {
            if (push_prev) {
                push_n_char_to_list(
                    line, start_offset, str_list, n_symbol, line_num_list, current_offset - start_offset, false, false
                );
            }
            if (push_symbol) {
                push_n_char_to_list(
                    line, current_offset, str_list, n_symbol, line_num_list, n_match, false, false
                );
            }
            return current_offset + n_match;
        }
    }
    return current_offset;
}

void destroy_symbol_match_module(parser_conf_t *conf, void *m_data) {
    symbol_match_module_data_t *data = (symbol_match_module_data_t *) m_data;
    size_t n_modules = sizeof(symbol_match_modules) / sizeof(symbol_match_module_t), i = 0;

    if (!data) return;
    for (i = 0; i < n_modules; i++) {
        if (data->data[i] && symbol_match_modules[i].destroy)
            symbol_match_modules[i].destroy(conf, data->data[i]);
    }
    __free(data);
}
