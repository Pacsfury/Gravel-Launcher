#pragma once

typedef enum {
    TOKEN_EOF,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_FLOAT,
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_STAR, //Can be either pointer dereference or multiplication
    TOKEN_DIV,
    TOKEN_VAR_DEF,
    TOKEN_VAR_INFER,
    TOKEN_ASSIGN,
    TOKEN_NAME,
    TOKEN_TYPEIS_LIST, //for char[][], int[], etc
    TOKEN_EQUAL,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_AMPERSAND,
    TOKEN_NEWLINE,
    TOKEN_RPAREN,
    TOKEN_LPAREN,
    TOKEN_ARROW,
    TOKEN_QUOTE
} TokenType;


typedef struct {
    TokenType type;
    char value[64];
} Token;

typedef enum {
    NODE_NUMBER,
    NODE_BINARY_OP
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int int_value;
    char op;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

void raiseError(char error[]);

void skipBlank(const char** current);

void tokenize(const char* file);

void showTokens();
