#pragma once
#include "argc.h"

typedef enum {
    TOKEN_EOF,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_FLOAT,
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_STAR, //Can be either pointer dereference or multiplication
    TOKEN_DIV,
    TOKEN_MODULO,
    TOKEN_VAR_INFER, // :=
    TOKEN_ASSIGN, // =
    TOKEN_NAME,
    TOKEN_TYPEIS_LIST, //for char[][], int[], etc
    TOKEN_EQUAL,
    TOKEN_AMPERSAND, // &
    TOKEN_PIPE, // |
    TOKEN_CARET, // ^
    TOKEN_EXC, // !
    TOKEN_TILDE, // ~
    TOKEN_NEWLINE,
    TOKEN_RPAREN,
    TOKEN_LPAREN,
    TOKEN_ARROW,
    TOKEN_QUOTE,
    TOKEN_COMMA,
    TOKEN_L_INT,
    TOKEN_L_FLOAT,
    TOKEN_SEMICOLON,
    TOKEN_LT, // <
    TOKEN_GT, // >
    TOKEN_LE, // <=
    TOKEN_GE, // >=
    TOKEN_NE, // !=
    TOKEN_INC, // ++
    TOKEN_DEC, // --
    TOKEN_ADD_ASSIGN, // +=
    TOKEN_SUB_ASSIGN, // -=
    TOKEN_STAR_ASSIGN, // *=
    TOKEN_DIV_ASSIGN, // /=
    TOKEN_MOD_ASSIGN, // %=

    //Keywords
    TOKEN_SCHO,
    TOKEN_END,
    TOKEN_IF,
    TOKEN_ELSEIF,
    TOKEN_ELSE,
    TOKEN_NAMESPACE,
    TOKEN_IMPORT,
    TOKEN_CLASS,
    TOKEN_FUN,
    TOKEN_IMPL,
    TOKEN_EXTL,
    TOKEN_RETURN,
    TOKEN_REPEAT,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_VAR_DEF, // val
    TOKEN_CONST

} TokenType;


typedef struct {
    TokenType type;
    char value[64];
} Token;

typedef enum {
    NODE_NUMBER,
    NODE_BINARY_OP2
} NodeType;

void raiseError(char error[], char id[]);

void skipBlank(const char** current);

void tokenize(const char* file, ARGS_CONTEX* ctx);

void showTokens();

void tokenizeFile(char* file, ARGS_CONTEX* ctx);

extern int token_count;