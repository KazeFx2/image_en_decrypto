//
// Created by Fx Kaze on 25-3-22.
//

#include "split_symbol_match.h"
#include "setmem.h"

typedef struct split_symbol_match_data_s {
    list_t split_symbol_matches;
} split_symbol_match_data_t;

static int compare_symbols(node_t *a, node_t *b) {
    string_t *a_s = (string_t *) get_data(a),
            *b_s = (string_t *) get_data(b);
    if (a_s->len > b_s->len) return 1;
    if (a_s->len < b_s->len) return -1;
    return 0;
}

void *init_split_symbol_match(parser_conf_t *conf) {
    list_t tmp = NULL;
    split_symbol_match_data_t *data = (split_symbol_match_data_t *) __malloc(sizeof(split_symbol_match_data_t));
    if (data == NULL) return NULL;
    tmp = array_to_list(
        conf->split_symbol,
        sizeof(string_t),
        conf->split_symbol_count
    );
    if (!tmp) {
        __free(data);
        return NULL;
    }
    data->split_symbol_matches = sort_by(
        tmp,
        compare_symbols,
        sizeof(string_t),
        DESC
    );
    destroy_list(tmp);
    if (!data->split_symbol_matches) {
        __free(data);
        return NULL;
    }
    return data;
}

void destroy_split_symbol_match(parser_conf_t *conf, void *m_data) {
    if (!m_data) return;
    split_symbol_match_data_t *data = (split_symbol_match_data_t *) m_data;
    if (data->split_symbol_matches) {
        destroy_list(data->split_symbol_matches);
    }
    __free(data);
}

size_t split_symbol_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool *push_symbol,
                          void *m_data) {
    split_symbol_match_data_t *data = (split_symbol_match_data_t *) m_data;
    string_t *symbol = NULL;

    if (!data->split_symbol_matches) return 0;
    if (push_prev) *push_prev = true;
    if (push_symbol) *push_symbol = false;
    FOREACH(node_t, i, data->split_symbol_matches) {
        symbol = (string_t *) get_data(i);
        if (c_string_cmp(str + offset, *symbol) == 0)
            return symbol->len;
    }
    return 0;
}
