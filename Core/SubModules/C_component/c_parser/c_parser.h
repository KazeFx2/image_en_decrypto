//
// Created by Fx Kaze on 25-3-20.
//

#ifndef CPARSER_H
#define CPARSER_H

#include <stdio.h>
#include "c_list.h"
#include "split_line_module/quote_match/quote_match.h"

struct quote_match_module_s;

typedef struct parser_conf_s {
    /// the default buffer size when reading a new line from the file.
    size_t default_line_buffer_size;
    /// whether keep '\n' while splitting words.
    bool keep_ln;
    /// whether considering '\\' + '\n' as the combine of lines.
    bool escape_ln;
    /// quote scope match.
    struct quote_sample_match_s *quote_sample_matches;
    size_t quote_sample_match_count;
    /// quote line endless match.
    struct quote_line_endless_match_s *quote_line_endless_matches;
    size_t quote_line_endless_match_count;
    /// split symbols.
    string_t *split_symbol;
    size_t split_symbol_count;
    /// key symbols.
    string_t *key_symbol;
    size_t key_symbol_count;
} parser_conf_t;

char *read_line(FILE *fp, size_t *line_length, bool *eof, parser_conf_t *conf, bool keep_ln);

list_t cparser_file_to_word_list(const char *file_name, size_t *n_word, parser_conf_t *conf);

#ifdef __cplusplus
#include "undef.h"
#endif

#endif //CPARSER_H
