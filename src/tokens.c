#include "../include/tokens.h"
#include "../include/vector.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

//TOKENIZER

void raiseError(char error[]) {
    //Make errors more helping and explaining
    printf("--- ERROR! ---\n");
    printf("%s\n", error);
    exit(1);
} 

Token tokens[512];
int token_count = 0;

void skipBlank(const char** current) {
    while (**current == ' ' || **current == '\t') {
        (*current)++;
    }
}

void showTokens() {
    int i = 0;
    while(tokens[i].type != TOKEN_EOF) {
        printf(tokens[i].value);
        i++;
    }
}

void tokenize(const char* file) {
    const char* source = file;
    while (*source != '\0') {
        skipBlank(&source);

        if (*source == '\0') {
            break;
        }

        switch (*source) {
            case '+':
                tokens[token_count].type = TOKEN_ADD;
                break;
            case '-':
                if (*(source+1) == '>') {
                    tokens[token_count].type = TOKEN_ARROW;
                    source++;
                } else { 
                    tokens[token_count].type = TOKEN_SUB;
                }
                break;
            case '*':
                tokens[token_count].type = TOKEN_STAR;
                break;
            case '/':
                tokens[token_count].type = TOKEN_DIV;
                break;
            case '=':
                if (*(source + 1) == '=') {
                    tokens[token_count].type = TOKEN_EQUAL;
                    source++;
                } else {
                    tokens[token_count].type = TOKEN_ASSIGN;
                }
                break;
            case ':':
                if (*(source + 1) == '=') {
                    tokens[token_count].type = TOKEN_VAR_INFER;
                    source++;
                } else {
                    raiseError("Unexpected token");
                }
                break;
            case '\n':
                tokens[token_count].type = TOKEN_NEWLINE;
                tokens[token_count].value[0] = '\0';
                break;
            case '(':
                tokens[token_count].type = TOKEN_LPAREN;
                break;
            case ')':
                tokens[token_count].type = TOKEN_RPAREN;
                break;
            case '"':
                tokens[token_count].type = TOKEN_QUOTE;
                source++;
                int s_len = 0;
                while (*source != '"' && *source != '\0') {
                    tokens[token_count].value[s_len++] = *source;
                    source++;
                }
                tokens[token_count].value[s_len] = '\0';
                if (*source == '\0') { raiseError("Unterminated string"); }
                break;
            case '\'':
                tokens[token_count].type = TOKEN_QUOTE;
                source++;
                int t_len = 0;
                while (*source != '\'' && *source != '\0') {
                    tokens[token_count].value[t_len++] = *source;
                    source++;
                }
                tokens[token_count].value[t_len] = '\0';
                if (*source == '\0') { raiseError("Unterminated string"); }
                break;
            case '&':
                tokens[token_count].type = TOKEN_AMPERSAND;
                break;
            default:
                if (isalpha(*source)) {
                    int len = 0;
                    char buffer[64];
                    while (isalnum(*source)) {
                        buffer[len++] = *source;
                        source++;
                    }
                    if (len > 63) {
                        raiseError("Exceeded max size.");
                    } 
                    buffer[len] = '\0';

                    if (strcmp(buffer, "val") == 0) {
                        tokens[token_count].type = TOKEN_VAR_DEF;
                    } else if (strcmp(buffer, "int") == 0) {
                        tokens[token_count].type = TOKEN_INT;
                    } else if (strcmp(buffer, "float") == 0) {
                        tokens[token_count].type = TOKEN_FLOAT;
                    } else if (strcmp(buffer, "char") == 0) {
                        tokens[token_count].type = TOKEN_CHAR;
                    } else if (strcmp(buffer, "scho") == 0) {
                        tokens[token_count].type = TOKEN_SCHO;
                    } else if (strcmp(buffer, "end") == 0) {
                        tokens[token_count].type = TOKEN_END;
                    } else {
                        tokens[token_count].type = TOKEN_NAME;
                        strcpy(tokens[token_count].value, buffer);
                    }
                    token_count++;
                    continue;
                } else if (isdigit(*source)) {
                    int n_len = 0;
                    char n_buffer[64];
                    int is_float = 0;

                    while ((isdigit(*source) || *source == '.') && n_len < 63) {
                        if (*source == '.') {
                            is_float = 1;
                        }
                        n_buffer[n_len++] = *source;
                        source++;
                    }
                    n_buffer[n_len] = '\0';

                    if (is_float) {
                        tokens[token_count].type = TOKEN_FLOAT;
                    } else {
                        tokens[token_count].type = TOKEN_INT;
                    }
                    strcpy(tokens[token_count].value, n_buffer);
                    
                    token_count++;
                    continue;
                } else {
                    raiseError("Unknown character");
                    source++;
                    continue;
                }


        }  
        source++;
        token_count++;
    }
    tokens[token_count].type = TOKEN_EOF;

    to_llvm_ir(tokens, token_count);
    //Finish with tokenizer
}

void tokenizeFile(char* file) {
    FILE* input = fopen(file, "r");
    char line[256];
    char *buffer = malloc(2048);
    size_t buffer_len = 0;

    if (!input || !buffer) {
        if (input) fclose(input);
        free(buffer);
        return;
    }

    buffer[0] = '\0';

    while (fgets(line, sizeof(line), input) != NULL) {
        size_t line_len = strlen(line);
        if (buffer_len + line_len >= 2048) {
            break;
        }
        memcpy(buffer + buffer_len, line, line_len + 1);
        buffer_len += line_len;
    }

    tokenize(buffer);

    fclose(input);
    free(buffer);
}
