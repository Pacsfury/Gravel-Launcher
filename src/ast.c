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

        if (left->type == NODE_LITERAL && right->type == NODE_LITERAL) {
            int val_left = atoi(left->data.literal.value);
            int val_right = atoi(right->data.literal.value);
            int res = 0;

            if (op_token->type == TOKEN_STAR) {
                res = val_left * val_right;
            } else if (op_token->type == TOKEN_DIV) {
                if (val_right == 0) {
                    raiseError("Compile-time division by zero detected", "E0005");
                }
                res = val_left / val_right;
            }

            snprintf(left->data.literal.value, sizeof(left->data.literal.value), "%d", res);

            free(right);
            continue;
        }

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

        if (left->type == NODE_LITERAL && right->type == NODE_LITERAL) {
            int val_left = atoi(left->data.literal.value);
            int val_right = atoi(right->data.literal.value);
            int res = 0;

            if (op_token->type == TOKEN_ADD) {
                res = val_left + val_right;
            } else if (op_token->type == TOKEN_SUB) {
                res = val_left - val_right;
            }

            snprintf(left->data.literal.value, sizeof(left->data.literal.value), "%d", res);

            free(right); 
            continue;
        }

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

    if (current->type == TOKEN_L_INT || current->type == TOKEN_L_FLOAT || current->type == TOKEN_QUOTE) {
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
        
        if (ns != NULL && ns[0] != '\0' && strchr(var_token->value, '.') == NULL) {
            sprintf(node->data.literal.value, "%s.%s", ns, var_token->value);
        } else {
            strcpy(node->data.literal.value, var_token->value); 
        }
        return node;
        
    } else if (current->type == TOKEN_REPEAT) {
        parse_repeat(t, c, ns);
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
    } else if (current->type == TOKEN_CONST) {
        ASTNode* result = (ASTNode*)malloc(sizeof(ASTNode));
        if (!result) raiseError("Memory allocation failed", "E0004");
        
        result->type = NODE_CONSTANT;
        advance(t, c);
        
        if (peek(t, c)->type == TOKEN_NAME) {
            Token* name_token = advance(t, c);
            
            if (ns != NULL && ns[0] != '\0') {
                sprintf(result->data.var_decl.name, "%s.%s", ns, name_token->value);
            } else {
                strcpy(result->data.var_decl.name, name_token->value);
            }
        } else {
            raiseError("Missing variable name after 'const'", "E0005");
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
        advance(t, c);

        if (peek(t, c)->type == TOKEN_LPAREN) {
            advance(t, c);
        } else {
            raiseError("Missing '(' after 'scho' function", "E0007.1");
        }

        result->data.scho_stmt.value = parse_expression(t, c, ns);

        if (peek(t, c)->type == TOKEN_RPAREN) {
            advance(t, c);
        } else {
            raiseError("Missing ')' after function expression", "E0007.2");
        }
        
        return result;
    } else if (current->type == TOKEN_NEWLINE) {
        advance(t, c);
        return NULL;
    } else if (current->type == TOKEN_REPEAT) {
        return parse_repeat(t, c, ns);
    } else {
        return parse_expression(t, c, ns);
    }
}

ASTNode* parse_repeat(const Token* t, int* c, const char* ns) {
    advance(t, c); // consume 'repeat'

    Token* value_token = peek(t, c);
    if (value_token->type != TOKEN_L_INT) raiseError("Expected integer after 'repeat'", "E0009");
    advance(t, c); // consume repeat count

    ASTNode* newNode = (ASTNode*)malloc(sizeof(ASTNode));
    if (!newNode) raiseError("Memory allocation failed", "E0004");
    newNode->type = NODE_REPEAT;
    newNode->data.repeat_stmt.repeat_count = atoi(value_token->value);

    int rep_capacity = 10;
    newNode->data.repeat_stmt.count = 0;
    newNode->data.repeat_stmt.statements = (ASTNode**)malloc(sizeof(ASTNode*) * rep_capacity);
    if (!newNode->data.repeat_stmt.statements) raiseError("Memory allocation failed", "E0004");

    while (peek(t, c)->type != TOKEN_END && peek(t, c)->type != TOKEN_EOF) {
        ASTNode* inner = parse_statement(t, c, ns);
        if (inner != NULL) {
            if (newNode->data.repeat_stmt.count >= rep_capacity) {
                rep_capacity *= 2;
                ASTNode** tmp = (ASTNode**)realloc(newNode->data.repeat_stmt.statements, sizeof(ASTNode*) * rep_capacity);
                if (!tmp) {
                    free(newNode->data.repeat_stmt.statements);
                    free(newNode);
                    raiseError("Memory allocation failed while expanding repeat statements", "E0004");
                }
                newNode->data.repeat_stmt.statements = tmp;
            }
            newNode->data.repeat_stmt.statements[newNode->data.repeat_stmt.count++] = inner;
        }
    }

    if (peek(t, c)->type == TOKEN_END) {
        advance(t, c);
    } else {
        raiseError("Unexpected end of file: missing 'end' for repeat block", "E0010.1");
    }

    return newNode;
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
    
    int statement_capacity = 100;
    program_node->data.program.statements = (ASTNode**)malloc(sizeof(ASTNode*) * statement_capacity);
    if (!program_node->data.program.statements) raiseError("Memory allocation failed", "E0004");
    
    while (peek(tokens, &current_token)->type != TOKEN_EOF) {
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
            if (ns_depth == 0) raiseError("Unexpected 'end' without matching entry", "E0010");
            
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
            if (program_node->data.program.count >= statement_capacity) {
                statement_capacity *= 2;
                ASTNode** temp = (ASTNode**)realloc(program_node->data.program.statements, sizeof(ASTNode*) * statement_capacity);
                if (!temp) {
                    free(program_node->data.program.statements);
                    free(program_node);
                    raiseError("Memory allocation failed while expanding statements", "E0004");
                }
                program_node->data.program.statements = temp;
            }

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