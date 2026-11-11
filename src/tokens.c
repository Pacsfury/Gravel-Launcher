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
#include "../include/lib.h"


token_data_t* token_data = NULL;

token_data_t* get_token_data(void) {
    return token_data;
}

bool set_token_data(token_data_t* a) {
    if (!a)
        return false;
    token_data = a;
    return true;
}

// TOKENIZER
void raiseError(char error[], char id[]) {
    printf("--- ERROR! ---\n");
    printf("%s\n", error);
    printf("Run `gravel explain %s` to get more details (COMING SOON)\n", id);
    exit(1);
}

void reserveTokenSpace(void) {
    if (!token_data) {
        raiseError("Missing variable. 'token_data'", "E0005");
        return;
    }

    if (token_data->token_count >= token_data->token_capacity) {
        if (token_data->token_capacity >= MAX_TOKEN_CAPACITY) {
            raiseError("Exceeded maximum token limit (potential infinite loop or massive file)", "E0000");
            return;
        }
        size_t new_capacity = (token_data->token_capacity == 0) ? 512 : token_data->token_capacity * 2;
        if (new_capacity > MAX_TOKEN_CAPACITY) {
            new_capacity = MAX_TOKEN_CAPACITY;
        }
        Token* temp = realloc(token_data->tokens, new_capacity * sizeof(Token));
        if (!temp) {
            free(token_data->tokens);
            token_data->tokens = NULL;
            token_data->token_capacity = 0;
            token_data->token_count = 0;
            raiseError("Out of memory allocation for tokens", "E0000");
            return;
        }
        token_data->tokens = temp;
        token_data->token_capacity = new_capacity;
    }
}

void skipBlank(const char** current) {
    while (**current == ' ' || **current == '\t') {
        (*current)++;
    }
}

/*

    * Not used I assume so that's why it wont get a function variable to token_data_t*

*/
void showTokens() {
    token_data_t* token_data = get_token_data();

    if (!token_data)
        raiseError("Missing variable. 'token_data'", "E0005");

    for (int i = 0; token_data->tokens[i].type != TOKEN_EOF; i++)
        printf("%s", token_data->tokens[i].value);
}

/*
 * OPTIMIZATION #1: Binary Search for Keyword Lookup
 * CRITICAL: Keep KEYWORDS array sorted alphabetically!
 * If you add a new keyword, insert it in alphabetical order.
 * Violating this will cause binary search to miss keywords!
 */
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
    {"inline",    TOKEN_INLINE},
    {"int",       TOKEN_INT},
    {"namespace", TOKEN_NAMESPACE},
    {"package",   TOKEN_PACKAGE},
    {"repeat",    TOKEN_REPEAT},
    {"return",    TOKEN_RETURN},
    {"scho",      TOKEN_SCHO},
    {"val",       TOKEN_VAR_DEF},
    {"while",     TOKEN_WHILE},
};

/*-- OPTIMIZED: Binary search for keyword lookup O(log n) instead of O(n) --*/
static inline TokenType lookup_keyword(const char *buffer) {
    int left = 0;
    int right = (sizeof(KEYWORDS) / sizeof(KEYWORDS[0])) - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(buffer, KEYWORDS[mid].kw);

        if (cmp == 0) {
            return KEYWORDS[mid].type;  // Found exact match
        } else if (cmp < 0) {
            right = mid - 1;             // Search left half
        } else {
            left = mid + 1;              // Search right half
        }
    }

    return TOKEN_NAME;  // Not a keyword, treat as identifier
}

TokenS tokenize(const char* file, ARGS_CONTEX* ctx, char* from) {
    const char* source = file;
    while (*source != '\0') {
        skipBlank(&source);
        if (*source == '\0') {
            break;
        }
        reserveTokenSpace();
        switch (*source) {
            case '+':
                if (*(source + 1) == '+') {
                    token_data->tokens[token_data->token_count].type = TOKEN_INC;
                    source++;
                } else if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_ADD_ASSIGN;
                    source++;
                } else {
                    token_data->tokens[token_data->token_count].type = TOKEN_ADD;
                }
                break;
            case '-':
                if (*(source + 1) == '>') {
                    token_data->tokens[token_data->token_count].type = TOKEN_ARROW;
                    source++;
                } else if (*(source + 1) == '-') {
                    token_data->tokens[token_data->token_count].type = TOKEN_DEC;
                    source++;
                } else if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_SUB_ASSIGN;
                    source++;
                } else {
                    token_data->tokens[token_data->token_count].type = TOKEN_SUB;
                }
                break;
            case '*':
                if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_STAR_ASSIGN;
                    source++;
                } else {
                    token_data->tokens[token_data->token_count].type = TOKEN_STAR;
                }
                break;
            case '/':
                if (*(source + 1) == '/') {
                    while (*source != '\n' && *source != '\0') {
                        source++;
                    }
                    continue;
                } else if (*(source + 1) == '*') {
                    source += 2;  // skip "/*"
                    while (*source != '\0' && !(*source == '*' && *(source + 1) == '/')) {
                        source++;
                    }
                    if (*source == '\0') {
                        raiseError("Unterminated block comment", "E0002.1");
                    }
                    source += 2;  // skip "*/"
                    continue;
                } else if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_DIV_ASSIGN;
                    source++;
                } else {
                    token_data->tokens[token_data->token_count].type = TOKEN_DIV;
                }
                break;
            case '%':
                if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_MOD_ASSIGN;
                    source++;
                } else {
                    token_data->tokens[token_data->token_count].type = TOKEN_MODULO;
                }
                break;
            case '<':
                if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_LE;
                    source++;
                } else {
                    token_data->tokens[token_data->token_count].type = TOKEN_LT;
                }
                break;
            case '>':
                if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_GE;
                    source++;
                } else {
                    token_data->tokens[token_data->token_count].type = TOKEN_GT;
                }
                break;
            case '!':
                if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_NE;
                    source++;
                } else {
                    // Not doing anything yet
                    token_data->tokens[token_data->token_count].type = TOKEN_EXC;
                }
                break;
            case '~':
                token_data->tokens[token_data->token_count].type = TOKEN_TILDE;
                break;
            case '|':
                token_data->tokens[token_data->token_count].type = TOKEN_PIPE;
                break;
            case '^':
                token_data->tokens[token_data->token_count].type = TOKEN_CARET;
                break;
            case ';':
                token_data->tokens[token_data->token_count].type = TOKEN_SEMICOLON;
                break;
            case ',':
                token_data->tokens[token_data->token_count].type = TOKEN_COMMA;
                break;
            case '=':
                if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_EQUAL;
                    source++;
                } else {
                    token_data->tokens[token_data->token_count].type = TOKEN_ASSIGN;
                }
                break;
            case ':':
                if (*(source + 1) == '=') {
                    token_data->tokens[token_data->token_count].type = TOKEN_VAR_INFER;
                    source++;
                } else if (token_data->token_count > 0 && token_data->tokens[token_data->token_count - 1].type == TOKEN_PACKAGE) {
                    source++;
                    continue;
                } else {
                    raiseError("Unexpected token", "E0001");
                }
                break;
            case '\n':
                token_data->tokens[token_data->token_count].type = TOKEN_NEWLINE;
                token_data->tokens[token_data->token_count].value[0] = '\0';
                break;
            case '(':
                token_data->tokens[token_data->token_count].type = TOKEN_LPAREN;
                break;
            case ')':
                token_data->tokens[token_data->token_count].type = TOKEN_RPAREN;
                break;
            case '"':
                token_data->tokens[token_data->token_count].type = TOKEN_QUOTE;
                source++;
                int s_len = 0;
                while (*source != '"' && *source != '\0') {
                    token_data->tokens[token_data->token_count].value[s_len++] = *source;
                    source++;
                }
                token_data->tokens[token_data->token_count].value[s_len] = '\0';
                if (*source == '\0') {
                    raiseError("Unterminated string", "E0002");
                }
                break;
            case '\'':
                token_data->tokens[token_data->token_count].type = TOKEN_QUOTE;
                source++;
                int t_len = 0;
                while (*source != '\'' && *source != '\0') {
                    token_data->tokens[token_data->token_count].value[t_len++] = *source;
                    source++;
                }
                token_data->tokens[token_data->token_count].value[t_len] = '\0';
                if (*source == '\0') {
                    raiseError("Unterminated string", "E0002");
                }
                break;
            case '&':
                token_data->tokens[token_data->token_count].type = TOKEN_AMPERSAND;
                break;
            default:
                if (isalpha(*source)) {
                    int len = 0;
                    char buffer[64];
                    while ((isalnum(*source) || *source == '.' || *source == '_') && len < 63) {
                        if (*source == '\\' && *(++source) == 'n') {
                            buffer[len++] = '\n';
                        } else {
                            buffer[len++] = *source;
                            source++;
                        }
                    }
                    buffer[len] = '\0';
                    strcpy(token_data->tokens[token_data->token_count].value, buffer);

                    /*-- USE OPTIMIZED BINARY SEARCH for keyword lookup --*/
                    token_data->tokens[token_data->token_count].type = lookup_keyword(buffer);

                    token_data->token_count++;
                    continue;
                } else if (isdigit(*source)) {
                    /*-- Number parsing section --*/
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
                        token_data->tokens[token_data->token_count].type = TOKEN_L_FLOAT;
                    } else {
                        token_data->tokens[token_data->token_count].type = TOKEN_L_INT;
                    }
                    strcpy(token_data->tokens[token_data->token_count].value, n_buffer);
                    token_data->token_count++;
                    continue;
                } else {
                    raiseError("Unknown character", "E0003");
                    source++;
                    continue;
                }
        }
        source++;
        token_data->token_count++;
    }

    reserveTokenSpace();
    token_data->tokens[token_data->token_count].type = TOKEN_EOF;

    for (int i = 0; i < token_data->token_count; i++) {
        if (token_data->tokens[i].type == TOKEN_PACKAGE && i + 1 < token_data->token_count && token_data->tokens[i + 1].type == TOKEN_NAME) {
            addPackage(token_data->tokens[i + 1].value, from ? from : "");
        }
    }

    if (!token_data->suppress_llvm_generation && checkGrammar(token_data->tokens, token_data->token_count)) {
        to_llvm_ir(ctx);
    }

    TokenS tokenRes;
    tokenRes.content = token_data->tokens;
    tokenRes.count = token_data->token_count;
    return tokenRes;
}

void tokenizeFile(char* file, ARGS_CONTEX* ctx) {
    char* buffer = get_file_contents(file);

    if (!buffer) {
        raiseError("File does not exist or cannot be read", "E0031");
        return;
    }


    tokenize(buffer, ctx, file);
    free(buffer);
}

bool cleanup_tokenizer(void) {
    if (!token_data)
        return false;

    free(token_data->tokens);
    free(token_data);

    return true;
}

bool init_tokenizer(void) {
    token_data = (token_data_t*)malloc(sizeof(token_data_t));

    if (!token_data)
        return false;

    token_data->token_count = 0;
    token_data->token_capacity = 0;
    token_data->suppress_llvm_generation = 0;

    token_data->tokens = (Token*)malloc(sizeof(Token) * DEFAULT_START_TOKEN_CAPACITY);

    return true;
}