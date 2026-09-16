#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "tokens.h"

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_POINTER,
    TYPE_FUNCTION_PTR
} DataType;

typedef struct {
    DataType base_type;
    int pointer_level;
    bool is_mutable;
} PointerType;

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

typedef struct {
    char name[64];
    char type[64];
} fun_args;

typedef struct ASTNode {
    ASTNodeType type;

    union {
        struct { char value[64]; } literal;
        struct { TokenType op; struct ASTNode* left; struct ASTNode* right; } binary_op;
        struct { TokenType op; struct ASTNode* operand; } unary_op;
        struct { char name[64]; struct ASTNode* value; char type[64]; } var_decl;
        struct { struct ASTNode** statements; int count; } program;
        struct { int times; struct ASTNode** statements; int count; int repeat_count; } repeat_stmt;
        struct { struct ASTNode* value; } scho_stmt;
        struct { struct ASTNode* condition; struct ASTNode** then_statements; int then_count; struct ASTNode* else_node; } if_stmt;
        struct { struct ASTNode* condition; struct ASTNode** statements; int count; } while_stmt;
        struct { struct ASTNode* init; struct ASTNode* condition; struct ASTNode* increment; struct ASTNode** statements; int count; } for_stmt;
        struct { struct ASTNode* value; } return_stmt;
        struct { char name[64]; struct ASTNode* value; } const_var;
        struct { char name[64]; fun_args* arguments; char returnType[64]; struct ASTNode* body; } fun_def;
        struct { char name[64]; char returnType[64]; struct ASTNode** arguments; int arg_count; } fun_call;
        struct { char name[64]; struct ASTNode* value; } reassign;
    } data;

    // Pointer metadata fields
    bool is_pointer;
    int pointer_level;
    struct ASTNode* pointed_type;
    struct ASTNode* pointer_operand;

} ASTNode;

// Node Creation & Pointer Helpers
ASTNode* create_ast_node(ASTNodeType type);
ASTNode* create_dereference_node(ASTNode* operand);
ASTNode* create_address_of_node(ASTNode* operand);

// Package helpers
ASTNode* getPackage(char* name, ARGS_CONTEX* ctx);
ASTNode* getPackageAs(char* name, const char* alias, ARGS_CONTEX* ctx);

// Parser Prototypes
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
