//
// Created by Fx Kaze on 25-3-22.
//

#ifndef KEY_SYMBOL_MATCH_H
#define KEY_SYMBOL_MATCH_H

#include "c_parser.h"

#define KEY_SYMBOL_MATCH_MODULE\
  {\
    init_key_symbol_match,\
    destroy_key_symbol_match,\
    key_symbol_match,\
  }

#define DEFAULT_KEY_SYMBOL_MATCH\
  MK_STRING("~"),\
  MK_STRING("!"),\
  MK_STRING("!="),\
  MK_STRING("#"),\
  MK_STRING("%"),\
  MK_STRING("%="),\
  MK_STRING("^"),\
  MK_STRING("^="),\
  MK_STRING("&"),\
  MK_STRING("&&"),\
  MK_STRING("&="),\
  MK_STRING("*"),\
  MK_STRING("*="),\
  MK_STRING("("),\
  MK_STRING(")"),\
  MK_STRING("-"),\
  MK_STRING("--"),\
  MK_STRING("->"),\
  MK_STRING("-="),\
  MK_STRING("+"),\
  MK_STRING("++"),\
  MK_STRING("+="),\
  MK_STRING("="),\
  MK_STRING("=="),\
  MK_STRING("{"),\
  MK_STRING("}"),\
  MK_STRING("["),\
  MK_STRING("]"),\
  MK_STRING("|"),\
  MK_STRING("||"),\
  MK_STRING("|="),\
  MK_STRING(":"),\
  MK_STRING("::"),\
  MK_STRING(";"),\
  MK_STRING("<"),\
  MK_STRING("<<"),\
  MK_STRING("<<="),\
  MK_STRING("<="),\
  MK_STRING(">"),\
  MK_STRING(">>"),\
  MK_STRING(">>="),\
  MK_STRING(">="),\
  MK_STRING("?"),\
  MK_STRING("/"),\
  MK_STRING("/="),\
  MK_STRING(","),\
  MK_STRING("."),\
  MK_STRING("..."),\

void *init_key_symbol_match(parser_conf_t *conf);

void destroy_key_symbol_match(parser_conf_t *conf, void *data);

size_t key_symbol_match(const char *str, size_t offset, size_t total_length, bool *push_prev, bool *push_symbol,
                      void *data);

#endif //KEY_SYMBOL_MATCH_H
