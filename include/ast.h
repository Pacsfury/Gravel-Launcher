#pragma once
#include <stdlib.h>
#include <string.h>

#include "tokens.h"

#define PARSER_MAX_CLASSES 64
#define PARSER_MAX_PARAMS  100
#define PARSER_IDENT_LEN   64
#define PARSER_MAX_SYMS    256

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
    NODE_REASSIGN,
    NODE_INLINE,
    NODE_CLASS,
    NODE_FUN_CLASS
} ASTNodeType;

typedef struct {
    char name[10 * 64];
    const Token* tokens;
    int body_start;
    int is_type;  // 1 for `class name: type`
} ClassDef;

typedef struct {
    char name[64];
    char type[64];
} fun_args;

// Members of type-class instances that the parser has to track:
//  SYM_STRING   - a char[] member; lives at compile time only, `value` is its text
//  SYM_READONLY - an int/float member (impl, scalar extl); can only change inside __USE__
typedef enum { SYM_STRING = 1, SYM_READONLY = 2 } TypeSymKind;

typedef struct {
    char name[10 * 64];
    int kind;
    char value[64];
} TypeSym;

// All mutable parser state lives here instead of in file-level globals.
typedef struct {
    ClassDef class_defs[PARSER_MAX_CLASSES];
    int      class_def_count;

    char     params[PARSER_MAX_PARAMS][PARSER_IDENT_LEN];
    int      param_count;

    TypeSym  syms[PARSER_MAX_SYMS];
    int      sym_count;

    int       in_use_fn;      // 1 while parsing the body of a __USE__ function
    int       extl_active;    // 1 while parsing a type class body (definition or instance)
    TokenType extl_type;      // token type of the extl literal (0 = unknown, while validating a definition)
    char      extl_value[64]; // text of the extl literal
} ParserState;

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

        struct {
            char target[8];
            char code[1024];
        } inline_code;

        struct {
            char target[8];
            char code[1024];
        } class_code;

    } data;
} ASTNode;


ParserState* parser_state(void);
void parser_state_reset(void);

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