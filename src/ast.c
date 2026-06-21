#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/ast.h"
#include "../include/tokens.h"

Token* peek(const Token* t, const int* c);
Token* advance(const Token* t, int* c);
ASTNode* parse_primary(const Token* t, int* c);
ASTNode* parse_multiplicative(const Token* t, int* c);
ASTNode* parse_additive(const Token* t, int* c);
ASTNode* parse_expression(const Token* t, int* c);
ASTNode* parse_statement(const Token* t, int* c);
ASTNode* parse(const Token* tokens, int count);
void print_ast(const ASTNode* node, int depth);
void raiseError(const char* message);

void raiseError(const char* message) {
    fprintf(stderr, "Parser Error: %s\n", message);
    exit(EXIT_FAILURE); 
}

Token* peek(const Token* t, const int* c) {
    return (Token*)&t[*c];
}

Token* advance(const Token* t, int* c) {
    (*c)++;
    return (Token*)&t[(*c)-1];
}

ASTNode* parse_multiplicative(const Token* t, int* c) {
    ASTNode* left = parse_primary(t, c);
    
    while (peek(t, c)->type == TOKEN_STAR || peek(t, c)->type == TOKEN_DIV) {
        Token* op_token = advance(t, c);

        ASTNode* right = parse_primary(t, c);
        ASTNode* bin_node = (ASTNode*)malloc(sizeof(ASTNode));
        if (!bin_node) raiseError("Memory allocation failed");
        
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
        if (!bin_node) raiseError("Memory allocation failed");

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
        if (!node) raiseError("Memory allocation failed");
        
        node->type = NODE_LITERAL;
        Token* lit_token = advance(t, c);
        strcpy(node->data.literal.value, lit_token->value);
        return node;
    }

    if (current->type == TOKEN_NAME) {
        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        if (!node) raiseError("Memory allocation failed");
        
        node->type = NODE_VARIABLE;
        Token* var_token = advance(t, c);
        strcpy(node->data.literal.value, var_token->value); 
        return node;
    }

    raiseError("Unexpected token: expected a variable or literal expression");
    return NULL; 
}

ASTNode* parse_expression(const Token* t, int* c) {
    return parse_additive(t, c);
}

ASTNode* parse_statement(const Token* t, int* c) {
    if (peek(t, c)->type == TOKEN_VAR_DEF) {
        ASTNode* result = (ASTNode*)malloc(sizeof(ASTNode));
        if (!result) raiseError("Memory allocation failed");
        
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
    if (!program_node) raiseError("Memory allocation failed");
    
    program_node->type = NODE_PROGRAM;
    program_node->data.program.count = 0;
    
    program_node->data.program.statements = (ASTNode**)malloc(sizeof(ASTNode*) * 100);
    if (!program_node->data.program.statements) raiseError("Memory allocation failed");
    
    while (peek(tokens, &current_token)->type != TOKEN_EOF) {
        if (program_node->data.program.count >= 100) {
            raiseError("Program exceeds maximum limit of 100 statements");
        }

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

    for (int i = 0; i < depth; i++) printf("  ");

    switch (node->type) {
        case NODE_DECLARATION:
            printf("[DECLARATION] Variable: %s\n", node->data.var_decl.name);
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