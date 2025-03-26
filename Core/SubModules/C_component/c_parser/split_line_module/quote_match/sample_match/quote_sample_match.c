//
// Created by Fx Kaze on 25-3-21.
//

#include "quote_sample_match.h"
#include "setmem.h"

typedef struct quote_sample_match_data_s {
    quote_sample_match_t *quote_sample_matches;
    size_t quote_sample_match_count;
    size_t current_index;
    bool keep_ln;
} quote_sample_match_data_t;

void *init_quote_sample_match(parser_conf_t *conf) {
    quote_sample_match_data_t *data = (quote_sample_match_data_t *) __malloc(sizeof(quote_sample_match_data_t));
    if (data == NULL) return NULL;
    data->quote_sample_matches = conf->quote_sample_matches;
    data->quote_sample_match_count = conf->quote_sample_match_count;
    data->current_index = 0;
    data->keep_ln = conf->keep_ln;
    return data;
}

void destroy_quote_sample_match(parser_conf_t *conf, void *data) {
    if (!data) return;
    __free(data);
}

size_t string_start_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool *push_word,
                          void *m_data) {
    quote_sample_match_data_t *data = (quote_sample_match_data_t *) m_data;

    if (push_prev) *push_prev = false;
    if (push_word) *push_word = true;
    for (data->current_index = 0; data->current_index < data->quote_sample_match_count; data->current_index++) {
        if (!data->keep_ln && c_string_cmp("\n", data->quote_sample_matches[data->current_index].end_str) == 0)
            continue;
        if (c_string_cmp(str + offset, data->quote_sample_matches[data->current_index].start_str) == 0)
            return data->quote_sample_matches[data->current_index].start_str.len;
    }
    return 0;
}

size_t string_end_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool *push_word,
                        void *m_data) {
    quote_sample_match_data_t *data = (quote_sample_match_data_t *) m_data;

    if (push_prev) *push_prev = true;
    if (push_word) *push_word = true;
    if (c_string_cmp(str + offset, data->quote_sample_matches[data->current_index].end_str) == 0)
        return data->quote_sample_matches[data->current_index].end_str.len;
    return 0;
}

size_t string_escape_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool *push_word,
                           void *m_data) {
    quote_sample_match_data_t *data = (quote_sample_match_data_t *) m_data;

    if (push_prev) *push_prev = true;
    if (push_word) *push_word = true;
    if (data->quote_sample_matches[data->current_index].escape && c_string_cmp(
            str + offset, data->quote_sample_matches[data->current_index].escape_str) == 0)
        return data->quote_sample_matches[data->current_index].escape_str.len;
    return 0;
}
