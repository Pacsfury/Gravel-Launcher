#pragma once
#include "tokens.h"
#include <stdlib.h>
#include <string.h>

typedef enum {
    NODE_LITERAL,
    NODE_BINARY_OP,
    NODE_VARIABLE,
    NODE_DECLARATION
} ASTNodeType;

typedef struct ASTNode {
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
    } data;
} ASTNode;