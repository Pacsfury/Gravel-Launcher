#pragma once
#include <stdlib.h>
#include <string.h>

#include "tokens.h"

typedef enum {
    NODE_LITERAL,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_VARIABLE,
    NODE_DECLARATION,
    NODE_PROGRAM,
    NODE_SCHO,
    NODE_REPEAT,
    NODE_CONSTANT,
    NODE_FUN_DEF,
    NODE_CALL,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_RETURN,
    NODE_REASSIGN
} ASTNodeType;

// identifier buffers hold a full Token value (64 bytes)
typedef struct {
    char name[64];
    char type[64];
} fun_args;

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
            TokenType op;
            struct ASTNode* operand;
        } unary_op;

        struct {
            char name[64];
            struct ASTNode* value;
            char type[64];
        } var_decl;

        struct {
            struct ASTNode** statements;
            int count;
        } program;

        struct {
            int times;
            struct ASTNode** statements;
            int count;
            int repeat_count;
        } repeat_stmt;

        struct {
            struct ASTNode* value;
        } scho_stmt;

        struct {
            struct ASTNode* condition;
            struct ASTNode** then_statements;
            int then_count;
            struct ASTNode* else_node;  // can be NODE_PROGRAM or NODE_IF or NULL
        } if_stmt;

        struct {
            struct ASTNode* condition;
            struct ASTNode** statements;
            int count;
        } while_stmt;

        struct {
            struct ASTNode* init;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode** statements;
            int count;
        } for_stmt;

        struct {
            struct ASTNode* value;
        } return_stmt;

        struct {
            char name[64];
            struct ASTNode* value;
        } const_var;

        struct {
            char name[64];
            fun_args* arguments;
            char returnType[64];
            struct ASTNode* body;
        } fun_def;

        struct {
            char name[64];
            char returnType[64];
            struct ASTNode** arguments;
            int arg_count;
        } fun_call;

        struct {
            char name[64];
            struct ASTNode* value;
        } reassign;

    } data;
} ASTNode;

Token* peek(const Token* t, const int* c);
Token* advance(const Token* t, int* c);
ASTNode* parse(const Token* tokens, int count, ARGS_CONTEX* ctx);
ASTNode* parse_primary(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_unary(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_multiplicative(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_additive(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_equality(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_expression(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_statement(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_repeat(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_if(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_while(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
ASTNode* parse_for(const Token* t, int* c, const char* ns, ARGS_CONTEX* ctx);
void print_ast(const ASTNode* node, int depth);