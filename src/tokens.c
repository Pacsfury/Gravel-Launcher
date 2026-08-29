#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/argc.h"
#include "../include/checker.h"
#include "../include/launcher.h"
#include "../include/tokens.h"
#include "../include/tollvm.h"
#include "../include/vector.h"

#define MAX_TOKEN_CAPACITY 1000000

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
int suppress_llvm_generation = 0;

void reserveTokenSpace(void) {
    if (token_count >= token_capacity) {
        if (token_capacity >= MAX_TOKEN_CAPACITY) {
            raiseError("Exceeded maximum token limit (potential infinite loop or massive file)", "E0000");
            return;
        }
        size_t new_capacity = (token_capacity == 0) ? 512 : token_capacity * 2;
        if (new_capacity > MAX_TOKEN_CAPACITY) {
            new_capacity = MAX_TOKEN_CAPACITY;
        }
        Token* temp = realloc(tokens, new_capacity * sizeof(Token));
        if (!temp) {
            free(tokens);
            tokens = NULL;
            token_capacity = 0;
            token_count = 0;
            raiseError("Out of memory allocation for tokens", "E0000");
            return;
        }
        tokens = temp;
        token_capacity = new_capacity;
    }
}

void skipBlank(const char** current) {
    while (**current == ' ' || **current == '\t') {
        (*current)++;
    }
}

void showTokens(void) {
    int i = 0;
    while (tokens[i].type != TOKEN_EOF) {
        printf("%s", tokens[i].value);
        i++;
    }
}

static const struct {
    const char *kw;
    TokenType type;
} KEYWORDS[] = {
    {"char",      TOKEN_CHAR},
    {"class",     TOKEN_CLASS},
    {"const",     TOKEN_CONST},
    {"else",      TOKEN_ELSE},
    {"elseif",    TOKEN_ELSEIF},
    {"end",       TOKEN_END},
    {"extl",      TOKEN_EXTL},
    {"float",     TOKEN_FLOAT},
    {"for",       TOKEN_FOR},
    {"fun",       TOKEN_FUN},
    {"if",        TOKEN_IF},
    {"impl",      TOKEN_IMPL},
    {"import",    TOKEN_IMPORT},
    {"in",        TOKEN_IN},
    {"int",       TOKEN_INT},
    {"namespace", TOKEN_NAMESPACE},
    {"package",   TOKEN_PACKAGE},
    {"repeat",    TOKEN_REPEAT},
    {"return",    TOKEN_RETURN},
    {"scho",      TOKEN_SCHO},
    {"val",       TOKEN_VAR_DEF},
    {"while",     TOKEN_WHILE},
};

#define NUM_KEYWORDS (sizeof(KEYWORDS) / sizeof(KEYWORDS[0]))

static inline TokenType lookup_keyword(const char *buffer) {
    int left = 0;
    int right = NUM_KEYWORDS - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(buffer, KEYWORDS[mid].kw);

        if (cmp == 0) {
            return KEYWORDS[mid].type;
        } else if (cmp < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return TOKEN_NAME;
}

TokenS tokenize(const char* file, ARGS_CONTEX* ctx, char* from) {
    (void)ctx;
    (void)from;

    const char* source = file;
    token_count = 0;

    while (*source != '\0') {
        skipBlank(&source);
        if (*source == '\0') break;

        reserveTokenSpace();

        if (isalpha(*source) || *source == '_') {
            int len = 0;
            char buffer[64];
            while ((isalnum(*source) || *source == '.' || *source == '_') && len < 63) {
                if (*source == '\\' && *(source + 1) == 'n') {
                    buffer[len++] = '\n';
                    source += 2;
                } else {
                    buffer[len++] = *source;
                    source++;
                }
            }
            buffer[len] = '\0';
            strcpy(tokens[token_count].value, buffer);
            tokens[token_count].type = lookup_keyword(buffer);
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
            source++;
            continue;
        }
    }

    reserveTokenSpace();
    tokens[token_count].type = TOKEN_EOF;

    for (int i = 0; i < token_count; i++) {
        if (tokens[i].type == TOKEN_PACKAGE && i + 1 < token_count && tokens[i + 1].type == TOKEN_NAME) {
            addPackage(tokens[i + 1].value, from ? from : "");
        }
    }

    if (!suppress_llvm_generation && checkGrammar(tokens, token_count)) {
        to_llvm_ir(tokens, token_count, ctx);
    }

    TokenS tokenRes;
    tokenRes.content = tokens;
    tokenRes.count = token_count;
    return tokenRes;
}

void tokenizeFile(char* file, ARGS_CONTEX* ctx) {
    FILE* input = fopen(file, "r");
    if (!input) {
        raiseError("File does not exist or cannot be read", "E0031");
        return;
    }

    char line[256];
    size_t buffer_capacity = 2048;
    char* buffer = malloc(buffer_capacity);
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

    tokenize(buffer, ctx, file);
    fclose(input);
    free(buffer);
    free(tokens);
}
