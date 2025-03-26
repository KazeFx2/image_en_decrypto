//
// Created by Fx Kaze on 25-3-21.
//

#include "line_endless_match.h"
#include "setmem.h"

typedef struct quote_line_endless_match_data_s {
    quote_line_endless_match_t *quote_line_endless_matches;
    size_t quote_line_endless_match_count;
    size_t current_index;
    bool keep_ln;
} quote_line_endless_match_data_t;

void *init_quote_line_endless_match(parser_conf_t *conf) {
    quote_line_endless_match_data_t *data = (quote_line_endless_match_data_t *) __malloc(
        sizeof(quote_line_endless_match_data_t));
    if (data == NULL) return NULL;
    data->quote_line_endless_matches = conf->quote_line_endless_matches;
    data->quote_line_endless_match_count = conf->quote_line_endless_match_count;
    data->current_index = 0;
    data->keep_ln = conf->keep_ln;
    return data;
}

void destroy_quote_line_endless_match(parser_conf_t *conf, void *data) {
    if (!data) return;
    __free(data);
}
