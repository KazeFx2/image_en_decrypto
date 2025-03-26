//
// Created by Fx Kaze on 25-3-21.
//

#ifndef QUOTE_SAMPLE_MATCH_H
#define QUOTE_SAMPLE_MATCH_H

#include "c_parser.h"

#define QUOTE_SAMPLE_MATCH_MODULE\
    {\
        init_quote_sample_match,\
        destroy_quote_sample_match,\
        string_start_match,\
        string_end_match,\
        string_escape_match,\
        true\
    }

#define DEFAULT_QUOTE_MATCH_1 { \
MK_STRING("\""),\
MK_STRING("\""),\
MK_STRING("\\"),\
true \
}

#define DEFAULT_QUOTE_MATCH_2 { \
MK_STRING("'"),\
MK_STRING("'"),\
MK_STRING("\\"),\
true \
}

#define DEFAULT_QUOTE_MATCH_3 { \
MK_STRING("/*"),\
MK_STRING("*/"),\
MK_STRING(""),\
false \
}

#define DEFAULT_QUOTE_MATCH_4 { \
MK_STRING("//"),\
MK_STRING("\n"),\
MK_STRING(""),\
false \
}

#define DEFAULT_QUOTE_MATCH \
    DEFAULT_QUOTE_MATCH_1,\
    DEFAULT_QUOTE_MATCH_2,\
    DEFAULT_QUOTE_MATCH_3,\
    DEFAULT_QUOTE_MATCH_4

void *init_quote_sample_match(parser_conf_t *conf);

void destroy_quote_sample_match(parser_conf_t *conf, void *data);

size_t string_start_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool * push_word, void *data);

size_t string_end_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool * push_word, void *data);

size_t string_escape_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool * push_word, void *data);

#endif //QUOTE_SAMPLE_MATCH_H
