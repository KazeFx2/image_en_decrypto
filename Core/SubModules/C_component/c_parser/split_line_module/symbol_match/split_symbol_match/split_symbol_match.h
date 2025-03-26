//
// Created by Fx Kaze on 25-3-22.
//

#ifndef SPLIT_SYMBOL_MATCH_H
#define SPLIT_SYMBOL_MATCH_H

#include "c_parser.h"

#define SPLIT_SYMBOL_MATCH_MODULE\
  {\
    init_split_symbol_match,\
    destroy_split_symbol_match,\
    split_symbol_match,\
  }

#define DEFAULT_SPLIT_SYMBOL_MATCH_1 MK_STRING(" ")
#define DEFAULT_SPLIT_SYMBOL_MATCH_2 MK_STRING("\t")
#define DEFAULT_SPLIT_SYMBOL_MATCH_3 MK_STRING("\0")
#define DEFAULT_SPLIT_SYMBOL_MATCH_4 MK_STRING("\n")

#define DEFAULT_SPLIT_SYMBOL_MATCH\
  DEFAULT_SPLIT_SYMBOL_MATCH_1,\
  DEFAULT_SPLIT_SYMBOL_MATCH_2,\
  DEFAULT_SPLIT_SYMBOL_MATCH_3,\
  DEFAULT_SPLIT_SYMBOL_MATCH_4,\

void *init_split_symbol_match(parser_conf_t *conf);

void destroy_split_symbol_match(parser_conf_t *conf, void *data);

size_t split_symbol_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool *push_symbol,
                        void *data);

#endif //SPLIT_SYMBOL_MATCH_H
