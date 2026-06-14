#include "../include/tokens.h"
#include <string.h>
#include <ctype.h>

//TOKENIZER

void raiseError(char error[]) {
    //Complete later
    //Make errors more helping and explaining
} 

Token tokens[512];
int token_count = 0;

void skipBlank(const char** current) {
    while (**current == ' ') {
        (*current)++;
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
            case '\t':
                tokens[token_count].type = TOKEN_INDENT;
                break;
            case '\n':
                tokens[token_count].type = TOKEN_NEWLINE;
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

    //Finish with tokenizer
}