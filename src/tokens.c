#include "../include/tokens.h"
#include "../include/vector.h"
#include "../include/argc.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// TOKENIZER

void raiseError(char error[], char id[]) {
    printf("--- ERROR! ---\n");
    printf("%s\n", error);
    printf("Run `gravel explain %s` to get more details (COMING SOON)\n", id);
    exit(1);
} 

Token* tokens = NULL;
int token_count = 0;
int token_capacity = 0; 

void reserveTokenSpace() {
    if (token_count >= token_capacity) {
        token_capacity = (token_capacity == 0) ? 512 : token_capacity * 2;
        Token* temp = realloc(tokens, token_capacity * sizeof(Token));
        if (!temp) {
            free(tokens);
            raiseError("Out of memory allocation for tokens", "E0000");
        }
        tokens = temp;
    }
}

void skipBlank(const char** current) {
    while (**current == ' ' || **current == '\t') {
        (*current)++;
    }
}

void showTokens() {
    int i = 0;
    while(tokens[i].type != TOKEN_EOF) {
        printf("%s", tokens[i].value);
        i++;
    }
}

void tokenize(const char* file, ARGS_CONTEX* ctx) {
    const char* source = file;
    while (*source != '\0') {
        skipBlank(&source);

        if (*source == '\0') {
            break;
        }

        reserveTokenSpace();

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
                    raiseError("Unexpected token", "E0001");
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
                if (*source == '\0') { raiseError("Unterminated string", "E0002"); }
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
                if (*source == '\0') { raiseError("Unterminated string", "E0002"); }
                break;
            case '&':
                tokens[token_count].type = TOKEN_AMPERSAND;
                break;
            default:
                if (isalpha(*source)) {
                    int len = 0;
                    char buffer[64];

                    while ((isalnum(*source) || *source == '.') && len < 63) {
                        buffer[len++] = *source;
                        source++;
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
                    } else if (strcmp(buffer, "namespace") == 0) {
                        tokens[token_count].type = TOKEN_NAMESPACE;
                    } else if (strcmp(buffer, "import") == 0) {
                        tokens[token_count].type = TOKEN_IMPORT;
                    } else if (strcmp(buffer, "class") == 0) {
                        tokens[token_count].type = TOKEN_CLASS;
                    } else if (strcmp(buffer, "fun") == 0) {
                        tokens[token_count].type = TOKEN_FUN;
                    } else if (strcmp(buffer, "impl") == 0) {
                        tokens[token_count].type = TOKEN_IMPL;
                    } else if (strcmp(buffer, "extl") == 0) {
                        tokens[token_count].type = TOKEN_EXTL;
                    } else if (strcmp(buffer, "repeat") == 0) {
                        tokens[token_count].type = TOKEN_REPEAT;

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
                        tokens[token_count].type = TOKEN_L_FLOAT;
                    } else {
                        tokens[token_count].type = TOKEN_L_INT;
                    }
                    strcpy(tokens[token_count].value, n_buffer);
                    
                    token_count++;
                    continue;
                } else {
                    raiseError("Unknown character", "E0003");
                    source++;
                    continue;
                }
        }  
        source++;
        token_count++;
    }
    
    reserveTokenSpace();
    tokens[token_count].type = TOKEN_EOF;

    to_llvm_ir(tokens, token_count, ctx);
    

}

void tokenizeFile(char* file, ARGS_CONTEX* ctx) {
    FILE* input = fopen(file, "r");
    if (!input) return;

    char line[256];
    size_t buffer_capacity = 2048;
    char *buffer = malloc(buffer_capacity);
    size_t buffer_len = 0;

    if (!buffer) {
        fclose(input);
        return;
    }

    buffer[0] = '\0';

    while (fgets(line, sizeof(line), input) != NULL) {
        size_t line_len = strlen(line);
        
        while (buffer_len + line_len >= buffer_capacity) {
            buffer_capacity *= 2;
            char* temp = realloc(buffer, buffer_capacity);
            if (!temp) {
                free(buffer);
                fclose(input);
                raiseError("Out of memory allocation for file buffer", "E0000");
            }
            buffer = temp;
        }
        
        memcpy(buffer + buffer_len, line, line_len + 1);
        buffer_len += line_len;
    }

    tokenize(buffer, ctx);

    fclose(input);
    free(buffer);
    free(tokens);
}
