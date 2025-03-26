//
// Created by Fx Kaze on 25-3-21.
//

#ifndef QUOTE_LINE_ENDLESS_MATCH_H
#define QUOTE_LINE_ENDLESS_MATCH_H

#include "c_parser.h"

#define QUOTE_LINE_ENDLESS_MATCH_MODULE\
    {\
        init_quote_line_endless_match,\
        destroy_quote_line_endless_match,\
        string_start_match,\
        NULL,\
        NULL,\
        false\
    }

#define DEFAULT_QUOTE_LINE_ENDLESS_MATCH_1 { \
    MK_STRING("//"),\
}

#define DEFAULT_QUOTE_LINE_ENDLESS_MATCH \
    DEFAULT_QUOTE_LINE_ENDLESS_MATCH_1

void *init_quote_line_endless_match(parser_conf_t *conf);

void destroy_quote_line_endless_match(parser_conf_t *conf, void *data);

#endif //QUOTE_LINE_ENDLESS_MATCH_H
