#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/ast.h"
#include "../include/tokens.h"


Token* peek(Token* t, int* c) {
    return &t[*c];
}

Token* advance(Token* t, int* c) {
    (*c)++;
    return &t[(*c)-1];
}

ASTNode* parse_multiplicative(const Token* t, int* c) {
    ASTNode* left = parse_primary(t, c);
    
    while (peek(t, c)->type == TOKEN_STAR || peek(t, c)->type == TOKEN_DIV) {
        Token* op_token = advance(t, c);

        ASTNode* right = parse_primary(t, c);
        ASTNode* bin_node = (ASTNode*)malloc(sizeof(ASTNode));
        bin_node->type = NODE_BINARY_OP;
        bin_node->data.binary_op.op = op_token->type;
        bin_node->data.binary_op.left = left;
        bin_node->data.binary_op.right = right;

        left = bin_node;
    }
    return left;
}

ASTNode* parse_additive(const Token* t, int* c) {
    ASTNode* left = parse_multiplicative(t, c);
    
    while (peek(t, c)->type == TOKEN_ADD || peek(t, c)->type == TOKEN_SUB) {
        Token* op_token = advance(t, c);

        ASTNode* right = parse_multiplicative(t, c);
        ASTNode* bin_node = (ASTNode*)malloc(sizeof(ASTNode));
        bin_node->type = NODE_BINARY_OP;
        bin_node->data.binary_op.op = op_token->type;
        bin_node->data.binary_op.left = left;
        bin_node->data.binary_op.right = right;

        left = bin_node;
    }
    return left;
}


ASTNode* parse_primary(const Token* t, int* c) {
    Token* current = peek(t, c);

    if (current->type == TOKEN_INT || current->type == TOKEN_FLOAT || current->type == TOKEN_QUOTE) {
        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_LITERAL;
        Token* lit_token = advance(t, c);
        strcpy(node->data.literal.value, lit_token->value);
        return node;
    }

    if (current->type == TOKEN_NAME) {
        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_VARIABLE;
        Token* var_token = advance(t, c);
        strcpy(node->data.literal.value, var_token->value);
        return node;
    }
}

ASTNode* parse_expression(const Token* t, int*c) {
    return parse_additive(t, c);
}

ASTNode* parse_statement(const Token* t, int* c) {
    if (peek(t, c)->type == TOKEN_VAR_DEF) {
        ASTNode* result = (ASTNode*)malloc(sizeof(ASTNode));
        result->type = NODE_DECLARATION;
        advance(t, c);
        if (peek(t, c)->type == TOKEN_NAME) {
            Token* name_token = advance(t, c);
            strcpy(result->data.var_decl.name, name_token->value);
        } else {
            raiseError("Missing variable name after 'val'");
        }
        if (peek(t, c)->type == TOKEN_ASSIGN) {
            advance(t, c);
        } else {
            raiseError("Missing '=' in variable declaration");
        }
        result->data.var_decl.value = parse_expression(t, c);
        return result;
    } else {
        return parse_expression(t, c);
    }
}

ASTNode* parse(const Token* tokens, int count) {
    int current_token = 0;
    
    ASTNode* program_node = (ASTNode*)malloc(sizeof(ASTNode));
    program_node->type = NODE_PROGRAM;
    program_node->data.program.count = 0;
    
    program_node->data.program.statements = (ASTNode**)malloc(sizeof(ASTNode*) * 100);
    
    while (peek(tokens, &current_token)->type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement(tokens, &current_token);
        
        if (stmt != NULL) {
            int idx = program_node->data.program.count;
            program_node->data.program.statements[idx] = stmt;
            program_node->data.program.count++;
        }
    }
    
    return program_node; 
}


void print_ast(const ASTNode* node, int depth) {
    if (node == NULL) return;

    // Imprimir espacios de sangría según la profundidad en el árbol
    for (int i = 0; i < depth; i++) printf("  ");

    switch (node->type) {
        case NODE_DECLARATION:
            printf("[DECLARATION] Variable: %s\n", node->data.var_decl.name);
            // Imprimir el valor asignado con un nivel más de profundidad
            print_ast(node->data.var_decl.value, depth + 1);
            break;
            
        case NODE_LITERAL:
            printf("[LITERAL] Value: %s\n", node->data.literal.value);
            break;
            
        case NODE_VARIABLE:
            printf("[VARIABLE] Name: %s\n", node->data.literal.value);
            break;
            
        case NODE_BINARY_OP:
            printf("[BINARY_OP] Operator Type: %d\n", node->data.binary_op.op);
            print_ast(node->data.binary_op.left, depth + 1);
            print_ast(node->data.binary_op.right, depth + 1);
            break;
        case NODE_PROGRAM:
            printf("[PROGRAM] Total Lines: %d\n", node->data.program.count);
            for (int i = 0; i < node->data.program.count; i++) {
                print_ast(node->data.program.statements[i], depth + 1);
            }
            break;

    }
}
