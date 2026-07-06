#pragma once
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

typedef enum {
    NODE_LITERAL,
    NODE_BINARY_OP,
    NODE_VARIABLE,
    NODE_DECLARATION,
    NODE_PROGRAM,
    NODE_SCHO
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;

    union {
        struct {
            char value[64];
        } literal;

        struct {
            TokenType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;

        struct {
            char name[64];
            struct ASTNode* value;
        } var_decl;

        struct {
            struct ASTNode** statements;
            int count;
        } program;

        struct {
            struct ASTNode* value; 
        } scho_stmt;

    } data;
} ASTNode;


Token* peek(const Token* t, const int* c);
Token* advance(const Token* t, int* c);
ASTNode* parse(const Token* tokens, int count);
ASTNode* parse_primary(const Token* t, int* c, const char* ns);
ASTNode* parse_multiplicative(const Token* t, int* c, const char* ns);
ASTNode* parse_additive(const Token* t, int* c, const char* ns);
ASTNode* parse_expression(const Token* t, int* c, const char* ns);
ASTNode* parse_statement(const Token* t, int* c, const char* ns);
void print_ast(const ASTNode* node, int depth);