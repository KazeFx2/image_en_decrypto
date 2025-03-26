//
// Created by Fx Kaze on 25-3-20.
//

#include <string.h>
#include "c_parser.h"

#include "setmem.h"
#include "c_string_ref_list/string_ref_list.h"
#include "split_line_module/split_line_module.h"
#include "split_line_module/quote_match/line_endless_match/line_endless_match.h"
#include "split_line_module/quote_match/sample_match/quote_sample_match.h"
#include "split_line_module/symbol_match/split_symbol_match/split_symbol_match.h"
#include "split_line_module/symbol_match/key_symbol_match/key_symbol_match.h"

char *read_line(FILE *fp, size_t *line_length, bool *eof, parser_conf_t *conf, bool keep_ln) {
    size_t buffer_size = conf->default_line_buffer_size;
    char *line = NULL, ch = 0, *new_line = NULL;

    *line_length = 0;
    *eof = false;
    line = (char *) __malloc(buffer_size);
    if (!line) {
        goto clean_up;
    }
    while ((ch = (char) getc(fp)) != EOF) {
        if (!keep_ln) {
            if (ch == '\n')
                break;
        }
        line[(*line_length)++] = ch;
        if (*line_length >= buffer_size) {
            buffer_size *= 2;
            new_line = (char *) __realloc(line, buffer_size);
            if (!new_line) {
                goto clean_up;
            }
            line = new_line;
        }
        if (keep_ln) {
            if (ch == '\n')
                break;
        }
    }
    if (ch == EOF)
        *eof = true;
    line[*line_length] = '\0';
    return line;
clean_up:
    if (line)
        __free(line);
    return NULL;
}

list_t cparser_file_to_word_list(
    const char *file,
    size_t *n_word,
    parser_conf_t *conf
) {
    FILE *fd = NULL;
    list_t string_list = NULL, line_num_list = NULL;
    size_t total_length = 0;
    line_num_t line_num = {0, 0};
    node_t *end = NULL;
    char *line = NULL, *append_line = NULL, *new_line = NULL;
    bool eof = false;
    void **data = NULL;

    fd = fopen(file, "r");
    if (!fd) {
        fprintf(stderr, "Failed to open file %s\n", file);
        goto clean_up;
    }
    string_list = init_c_string_ref_list();
    if (!string_list) {
        fprintf(stderr, "Failed to allocate memory for string_ref_list\n");
        goto clean_up;
    }
    line_num_list = init_list_default();
    if (!line_num_list) {
        fprintf(stderr, "Failed to allocate memory for line_num_list\n");
        goto clean_up;
    }
    data = init_split_line_data(conf);
    if (!data) {
        fprintf(stderr, "Failed to allocate memory for split_line_data\n");
        goto clean_up;
    }
    while (!eof) {
        clear_list(line_num_list);
        line = read_line(fd, &line_num.line_length, &eof, conf, conf->keep_ln);
        push_end(line_num_list, &line_num, sizeof(line_num_t));
        total_length = line_num.line_length;
        if (conf->escape_ln) {
            end = get_last(line_num_list);
            while (line[line_num.line_length - 1] == '\\' && !eof) {
                if (conf->keep_ln) {
                    ((line_num_t *) get_data(end))->line_length--;
                    total_length--;
                }
                line_num.line_num++;
                append_line = read_line(fd, &line_num.line_length, &eof, conf, conf->keep_ln);
                new_line = (char *) __realloc(line,
                                              ((line_num_t *) get_data(end))->line_length + line_num.line_length);
                if (!new_line) {
                    goto clean_up;
                }
                line = new_line;
                new_line = NULL;
                memcpy(line + ((line_num_t *) get_data(end))->line_length - 1, append_line,
                       line_num.line_length);
                __free(append_line);
                append_line = NULL;
                total_length += line_num.line_length;
                push_node_before2(line_num_list, (void **) &end->next, &line_num, sizeof(line_num_t));
                end = end->next;
            }
        }
        if (split_line(
            file, string_list, n_word, line, line_num_list, total_length, conf, data
        )) {
            fprintf(stderr, "split_line failed\n");
            goto clean_up;
        }
        line_num.line_num++;
        __free(line);
    }
    line = NULL;
    fclose(fd);
    destroy_list(line_num_list);
    destroy_split_line_data(conf, data);
    return string_list;
clean_up:
    if (append_line)
        __free(append_line);
    if (line)
        __free(line);
    if (fd) {
        fclose(fd);
    }
    if (string_list)
        destroy_list(string_list);
    if (line_num_list)
        destroy_list(line_num_list);
    if (data)
        destroy_split_line_data(conf, data);
    return string_list;
}

int main(int argc, char *argv[]) {
    quote_sample_match_t matches[] = {
        DEFAULT_QUOTE_MATCH
    };
    quote_line_endless_match_t line_endless_matches[] = {
        DEFAULT_QUOTE_LINE_ENDLESS_MATCH
    };
    string_t split_symbol_matches[] = {
        DEFAULT_SPLIT_SYMBOL_MATCH
    };
    string_t key_symbol_matches[] = {
        DEFAULT_KEY_SYMBOL_MATCH
    };
    parser_conf_t conf = {
        1024,
        true,
        true,
        matches,
        sizeof(matches) / sizeof(quote_sample_match_t),
        line_endless_matches,
        sizeof(line_endless_matches) / sizeof(quote_line_endless_match_t),
        split_symbol_matches,
        sizeof(split_symbol_matches) / sizeof(string_t),
        key_symbol_matches,
        sizeof(key_symbol_matches) / sizeof(string_t),
    };
    size_t n_word = 0;
    list_t string_list = cparser_file_to_word_list(
        argv[1], &n_word, &conf
    );
    FOREACH(node_t, i, string_list) {
        pos_string_t *str = *(pos_string_t **) get_data(i);
        if (strcmp(str->string.str, "\n") == 0) {
            printf("(%04lu, %04lu) | %s\n", str->position.row + 1, str->position.col + 1, "\\n");
        } else {
            printf("(%04lu, %04lu) | %s\n", str->position.row + 1, str->position.col + 1, str->string.str);
        }
    }
    return 0;
}
