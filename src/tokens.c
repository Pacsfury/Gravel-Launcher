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
    while (**current == ' ' || **current == '\t' || **current == '\n') {
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
                tokens[token_count].type = TOKEN_SUB;
                break;
            case '*':
                tokens[token_count].type = TOKEN_STAR;
                break;
            case '/':
                tokens[token_count].type = TOKEN_DIV;
                break;
            case '=':
                if (*(source + 1) == '=') {
                    tokens[token_count].type = TOKEN_ISEQUAL;
                    source++;
                } else {
                    tokens[token_count].type = TOKEN_EQUAL;
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
                    //put later
                }


        }  
        source++;
        token_count++;
    }
    tokens[token_count].type = TOKEN_EOF;

    //Finish with tokenizer
}