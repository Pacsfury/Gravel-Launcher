#pragma once
#include <stdlib.h>
#include <string.h>
#include "tokens.h"


typedef enum {
    NODE_LITERAL,
    NODE_BINARY_OP,
    NODE_VARIABLE,
    NODE_DECLARATION,
    NODE_PROGRAM
} ASTNodeType;

typedef struct {
    ASTNodeType type;

    union {
        //LITERAL and VARIABLES
        struct {
            char value[64];
        } literal;

        //BINARY_OP
        struct {
            TokenType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;

        //DEFINITIONS
        struct {
            char name[64];
            struct ASTNode* value;
        } var_decl;

        //PROGRAM
        struct {
            struct ASTNode** statements;
            int count;
        } program;

    } data;
} ASTNode;

Token* peek(Token* t, int* c);

Token* advance(Token* t, int* c);

ASTNode* parse(const Token* tokens, int count);

ASTNode* parse_primary(const Token* t, int* c);

ASTNode* parse_expression(const Token* t, int*c);

ASTNode* parse_statement(const Token* t, int* c);

ASTNode* parse_multiplicative(const Token* t, int* c);
ASTNode* parse_additive(const Token* t, int* c);

//For debugging! To make this less painful :::::)))))))))))))))))
void print_ast(const ASTNode* node, int depth);