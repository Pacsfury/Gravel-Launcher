#include "../include/ast.h"
#include "../include/tokens.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token* peek(const Token* t, int* c) {
    return &t[*c];
}

Token* advance(const Token* t, int* c) {
    (*c)++;
    return &t[(*c)-1];
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
    return parse_primary(t, c);
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
    ASTNode* root = NULL;
    
    while (peek(tokens, &current_token)->type != TOKEN_EOF) {
        root = parse_statement(tokens, &current_token);
    }
    
    return root; 
}
