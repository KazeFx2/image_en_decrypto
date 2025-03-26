//
// Created by Fx Kaze on 25-3-22.
//

#include "key_symbol_match.h"
#include "setmem.h"

typedef struct key_symbol_match_data_s {
    list_t key_symbol_matches;
} key_symbol_match_data_t;

static int compare_symbols(node_t *a, node_t *b) {
    string_t *a_s = (string_t *) get_data(a),
            *b_s = (string_t *) get_data(b);
    if (a_s->len > b_s->len) return 1;
    if (a_s->len < b_s->len) return -1;
    return 0;
}

void *init_key_symbol_match(parser_conf_t *conf) {
    list_t tmp = NULL;
    key_symbol_match_data_t *data = (key_symbol_match_data_t *) __malloc(sizeof(key_symbol_match_data_t));
    if (data == NULL) return NULL;
    tmp = array_to_list(
        conf->key_symbol,
        sizeof(string_t),
        conf->key_symbol_count
    );
    if (!tmp) {
        __free(data);
        return NULL;
    }
    data->key_symbol_matches = sort_by(
        tmp,
        compare_symbols,
        sizeof(string_t),
        DESC
    );
    destroy_list(tmp);
    if (!data->key_symbol_matches) {
        __free(data);
        return NULL;
    }
    return data;
}

void destroy_key_symbol_match(parser_conf_t *conf, void *m_data) {
    if (!m_data) return;
    key_symbol_match_data_t *data = (key_symbol_match_data_t *) m_data;
    if (data->key_symbol_matches) {
        destroy_list(data->key_symbol_matches);
    }
    __free(data);
}

size_t key_symbol_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool *push_symbol,
                      void *m_data) {
    key_symbol_match_data_t *data = (key_symbol_match_data_t *) m_data;
    string_t *symbol = NULL;

    if (!data->key_symbol_matches) return 0;
    if (push_prev) *push_prev = true;
    if (push_symbol) *push_symbol = true;
    FOREACH(node_t, i, data->key_symbol_matches) {
        symbol = (string_t *) get_data(i);
        if (c_string_cmp(str + offset, *symbol) == 0)
            return symbol->len;
    }
    return 0;
}
