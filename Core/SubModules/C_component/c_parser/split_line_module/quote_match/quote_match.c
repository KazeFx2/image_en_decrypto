//
// Created by Fx Kaze on 25-3-20.
//

#include "quote_match.h"
#include "setmem.h"
#include "line_endless_match/line_endless_match.h"
#include "split_line_module/quote_match/sample_match/quote_sample_match.h"

quote_match_module_t quote_match_modules[] = {
    QUOTE_SAMPLE_MATCH_MODULE,
    QUOTE_LINE_ENDLESS_MATCH_MODULE,
};

typedef struct quote_match_module_data_s {
    void **data;
    bool in_quote;
    size_t current_match_index;
} quote_match_module_data_t;

void *init_quote_match_module(parser_conf_t *conf) {
    quote_match_module_data_t *data = NULL;
    size_t n_modules = sizeof(quote_match_modules) / sizeof(quote_match_module_t), i = 0;

    data = __malloc(sizeof(quote_match_module_data_t));
    if (!data) goto clean_up;
    data->current_match_index = 0;
    data->in_quote = false;
    data->data = __malloc(n_modules * sizeof(void *));
    if (!data->data) goto clean_up;
    for (i = 0; i < n_modules; i++) {
        if (quote_match_modules[i].init) {
            data->data[i] = quote_match_modules[i].init(conf);
        } else {
            data->data[i] = NULL;
        }
    }
    return data;

clean_up:
    if (!data) return NULL;
    if (data->data) {
        for (i = 0; i < n_modules; i++) {
            if (data->data[i] && quote_match_modules[i].destroy)
                quote_match_modules[i].destroy(conf, data->data[i]);
        }
        __free(data);
    }
    return NULL;
}

size_t quote_match_module_func(
    const char *file,
    const char *line,
    size_t start_offset,
    size_t current_offset,
    size_t total_length,
    list_t str_list,
    size_t *n_word,
    list_t line_num_list,
    parser_conf_t *conf,
    void *m_data) {
    size_t n_modules = sizeof(quote_match_modules) / sizeof(quote_match_module_t), n_match = 0, i =
            0;
    quote_match_module_data_t *data = (quote_match_module_data_t *) m_data;
    line_num_t *line_num = NULL;
    bool escape = false;
    bool stick_prev = false;
    bool push_prev = true, push_word = true;

    if (!data) return current_offset;
    if (data->in_quote) {
        stick_prev = true;
        goto solve_remain;
    }
    for (i = 0; i < n_modules; i++) {
        if (!quote_match_modules[i].start_match)
            continue;
        n_match = quote_match_modules[i].start_match(line, current_offset, total_length, NULL, &push_word,
                                                     data->data[i]);
        if (n_match > 0) {
            if (push_word) {
                push_n_char_to_list(
                    line, current_offset, str_list, n_word, line_num_list, n_match, false, false
                );
            }
            current_offset += n_match;
            data->current_match_index = i;
            data->in_quote = true;
            break;
        }
    }
    // data->in_quote = true;
    if (!data->in_quote) {
        return 0;
    }
solve_remain:
    if (!quote_match_modules[data->current_match_index].end_match) {
        data->in_quote = false;
        push_n_char_to_list(line, current_offset, str_list, n_word, line_num_list, total_length - current_offset, false,
                            stick_prev);
        if (!conf->keep_ln) {
            line_num = (line_num_t *) get_data(get_last(line_num_list));
            push_to_list(
                "\n", 1, line_num->line_num, line_num->line_length, str_list, n_word, true);
            return total_length + 1;
        }
        return total_length;
    }
    for (i = current_offset; i < total_length; i++) {
        n_match = quote_match_modules[data->current_match_index].end_match(
            line, i, total_length, &push_prev, &push_word, data->data[data->current_match_index]
        );
        if (n_match > 0 && !escape) {
            if (push_prev) {
                push_n_char_to_list(
                    line, current_offset, str_list, n_word, line_num_list, i - current_offset,false, stick_prev
                );
            }
            current_offset = i;
            if (push_word) {
                push_n_char_to_list(
                    line, current_offset, str_list, n_word, line_num_list, n_match, false, false
                );
            }
            current_offset += n_match;
            data->in_quote = false;
            return current_offset;
        }
        if (quote_match_modules[data->current_match_index].escape) {
            n_match = quote_match_modules[data->current_match_index].escape_match(
                line, i, total_length,NULL, NULL, data->data[data->current_match_index]
            );
            if (n_match > 0) {
                escape = !escape;
            } else {
                escape = false;
            }
        }
    }
    if (i == total_length) {
        push_n_char_to_list(
            line, current_offset, str_list, n_word, line_num_list, total_length - current_offset,true, stick_prev
        );
        current_offset = i;
        if (!conf->keep_ln) {
            line_num = (line_num_t *) get_data(get_last(line_num_list));
            stick_push_to_list(
                "\n", 1, line_num->line_num, line_num->line_length, str_list);
            return current_offset + 1;
        }
    }
    return current_offset;
}

void destroy_quote_match_module(parser_conf_t *conf, void *m_data) {
    quote_match_module_data_t *data = (quote_match_module_data_t *) m_data;
    size_t n_modules = sizeof(quote_match_modules) / sizeof(quote_match_module_t), i = 0;

    if (!data) return;
    for (i = 0; i < n_modules; i++) {
        if (data->data[i] && quote_match_modules[i].destroy)
            quote_match_modules[i].destroy(conf, data->data[i]);
    }
    __free(data);
}
