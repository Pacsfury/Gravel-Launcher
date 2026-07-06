#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/ast.h"
#include "../include/tokens.h"

Token* peek(const Token* t, const int* c) {
    return (Token*)&t[*c];
}

Token* advance(const Token* t, int* c) {
    (*c)++;
    return (Token*)&t[(*c)-1];
}

ASTNode* parse_multiplicative(const Token* t, int* c, const char* ns) {
    ASTNode* left = parse_primary(t, c, ns);
    
    while (peek(t, c)->type == TOKEN_STAR || peek(t, c)->type == TOKEN_DIV) {
        Token* op_token = advance(t, c);

        ASTNode* right = parse_primary(t, c, ns);
        ASTNode* bin_node = (ASTNode*)malloc(sizeof(ASTNode));
        if (!bin_node) raiseError("Memory allocation failed", "E0004");
        
        bin_node->type = NODE_BINARY_OP;
        bin_node->data.binary_op.op = op_token->type;
        bin_node->data.binary_op.left = left;
        bin_node->data.binary_op.right = right;

        left = bin_node;
    }
    return left;
}

ASTNode* parse_additive(const Token* t, int* c, const char* ns) {
    ASTNode* left = parse_multiplicative(t, c, ns);
    
    while (peek(t, c)->type == TOKEN_ADD || peek(t, c)->type == TOKEN_SUB) {
        Token* op_token = advance(t, c);

        ASTNode* right = parse_multiplicative(t, c, ns);
        ASTNode* bin_node = (ASTNode*)malloc(sizeof(ASTNode));
        if (!bin_node) raiseError("Memory allocation failed", "E0004");

        bin_node->type = NODE_BINARY_OP;
        bin_node->data.binary_op.op = op_token->type;
        bin_node->data.binary_op.left = left;
        bin_node->data.binary_op.right = right;

        left = bin_node;
    }
    return left;
}

ASTNode* parse_primary(const Token* t, int* c, const char* ns) {
    Token* current = peek(t, c);

    if (current->type == TOKEN_INT || current->type == TOKEN_FLOAT || current->type == TOKEN_QUOTE) {
        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        if (!node) raiseError("Memory allocation failed", "E0004");
        
        node->type = NODE_LITERAL;
        Token* lit_token = advance(t, c);
        strcpy(node->data.literal.value, lit_token->value);
        return node;
    }

    if (current->type == TOKEN_NAME) {
        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        if (!node) raiseError("Memory allocation failed", "E0004");
        
        node->type = NODE_VARIABLE;
        Token* var_token = advance(t, c);
        
        // If we are currently inside a namespace AND the variable isn't already explicitly prefixed
        if (ns != NULL && ns[0] != '\0' && strchr(var_token->value, '.') == NULL) {
            sprintf(node->data.literal.value, "%s.%s", ns, var_token->value);
        } else {
            strcpy(node->data.literal.value, var_token->value); 
        }
        return node;
    }

    raiseError("Unexpected token: expected a variable or literal expression", "E0001:1");
    return NULL; 
}

ASTNode* parse_expression(const Token* t, int* c, const char* ns) {
    return parse_additive(t, c, ns);
}

ASTNode* parse_statement(const Token* t, int* c, const char* ns) {
    Token* current = peek(t, c);

    if (current->type == TOKEN_VAR_DEF) {
        ASTNode* result = (ASTNode*)malloc(sizeof(ASTNode));
        if (!result) raiseError("Memory allocation failed", "E0004");
        
        result->type = NODE_DECLARATION;
        advance(t, c);
        
        if (peek(t, c)->type == TOKEN_NAME) {
            Token* name_token = advance(t, c);
            
            // Apply namespace prefix to the newly declared variable
            if (ns != NULL && ns[0] != '\0') {
                sprintf(result->data.var_decl.name, "%s.%s", ns, name_token->value);
            } else {
                strcpy(result->data.var_decl.name, name_token->value);
            }
        } else {
            raiseError("Missing variable name after 'val'", "E0005");
        }
        
        if (peek(t, c)->type == TOKEN_VAR_INFER) {
            advance(t, c); 
        } else {
            raiseError("Missing '=' (or :=) in variable declaration", "E0006");
        }
        
        result->data.var_decl.value = parse_expression(t, c, ns);
        return result;
    } 

    else if (current->type == TOKEN_SCHO) {
        ASTNode* result = (ASTNode*)malloc(sizeof(ASTNode));
        if (!result) raiseError("Memory allocation failed", "E0004");
        
        result->type = NODE_SCHO;
        advance(t, c); // consume 'scho'

        if (peek(t, c)->type == TOKEN_LPAREN) {
            advance(t, c); // consume '('
        } else {
            raiseError("Missing '(' after 'scho' function", "E0007.1");
        }

        result->data.scho_stmt.value = parse_expression(t, c, ns);

        if (peek(t, c)->type == TOKEN_RPAREN) {
            advance(t, c); // consume ')'
        } else {
            raiseError("Missing ')' after function expression", "E0007.2");
        }
        
        return result;
    } else if (current->type == TOKEN_NEWLINE) {
        advance(t, c);
        return NULL;
    } else {
        return parse_expression(t, c, ns);
    }
}

ASTNode* parse(const Token* tokens, int count) {
    int current_token = 0;
    
    char ns_stack[10][64]; 
    int ns_depth = 0;
    char current_namespace[256] = "";
    
    ASTNode* program_node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!program_node) raiseError("Memory allocation failed", "E0004");
    
    program_node->type = NODE_PROGRAM;
    program_node->data.program.count = 0;
    
    program_node->data.program.statements = (ASTNode**)malloc(sizeof(ASTNode*) * 100);
    if (!program_node->data.program.statements) raiseError("Memory allocation failed", "E0004");
    
    while (peek(tokens, &current_token)->type != TOKEN_EOF) {
        if (program_node->data.program.count >= 100) {
            raiseError("Program exceeds maximum limit of 100 statements", "E0008");
        }

        Token* current = peek(tokens, &current_token);

        if (current->type == TOKEN_NAMESPACE) {
            advance(tokens, &current_token); 
            
            Token* name_token = peek(tokens, &current_token);
            if (name_token->type != TOKEN_NAME) raiseError("Expected identifier after 'namespace'", "E0009");
            
            if (ns_depth >= 10) raiseError("Maximum namespace depth exceeded", "E0008.1");
            
            strcpy(ns_stack[ns_depth++], name_token->value);
            advance(tokens, &current_token);
            
            current_namespace[0] = '\0';
            for (int i = 0; i < ns_depth; i++) {
                strcat(current_namespace, ns_stack[i]);
                if (i < ns_depth - 1) strcat(current_namespace, ".");
            }
            continue;
        }

        if (current->type == TOKEN_END) {
            advance(tokens, &current_token);
            if (ns_depth == 0) raiseError("Unexpected 'end' without matching 'namespace'", "E0010");
            
            ns_depth--;
            
            current_namespace[0] = '\0';
            for (int i = 0; i < ns_depth; i++) {
                strcat(current_namespace, ns_stack[i]);
                if (i < ns_depth - 1) strcat(current_namespace, ".");
            }
            continue; 
        }

        ASTNode* stmt = parse_statement(tokens, &current_token, current_namespace);
        
        if (stmt != NULL) {
            int idx = program_node->data.program.count;
            program_node->data.program.statements[idx] = stmt;
            program_node->data.program.count++;
        }
    }
    
    if (ns_depth > 0) {
        raiseError("Unexpected end of file: missing 'end' for namespace", "E0010");
    }
    
    return program_node; 
}