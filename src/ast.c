#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/ast.h"
#include "../include/tokens.h"

static int add_overflow_long_long(long long a, long long b, long long *result) {
    if ((b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b)) {
        return 1;
    }
    *result = a + b;
    return 0;
}

static int sub_overflow_long_long(long long a, long long b, long long *result) {
    if ((b < 0 && a > LLONG_MAX + b) || (b > 0 && a < LLONG_MIN + b)) {
        return 1;
    }
    *result = a - b;
    return 0;
}

static int mul_overflow_long_long(long long a, long long b, long long *result) {
    if (a == 0 || b == 0) {
        *result = 0;
        return 0;
    }

    if (a == -1 && b == LLONG_MIN) return 1;
    if (b == -1 && a == LLONG_MIN) return 1;

    long long abs_a = a < 0 ? -a : a;
    long long abs_b = b < 0 ? -b : b;
    if (abs_a > LLONG_MAX / abs_b) {
        return 1;
    }

    *result = a * b;
    return 0;
}

// Fold a binary op over two literal operands, erroring out instead of
// overflowing. Operand strings may exceed the int range (float literals fold
// through their integer prefix), so parse and compute in long long.
static int fold_literals(TokenType op, const char* left, const char* right) {
    errno = 0;
    long long l = strtoll(left, NULL, 10);
    long long r = strtoll(right, NULL, 10);
    if (errno == ERANGE) raiseError("Integer literal out of range", "E0012");

    long long res = 0;
    int overflow = 0;
    switch (op) {
        case TOKEN_ADD: overflow = add_overflow_long_long(l, r, &res); break;
        case TOKEN_SUB: overflow = sub_overflow_long_long(l, r, &res); break;
        case TOKEN_STAR: overflow = mul_overflow_long_long(l, r, &res); break;
        case TOKEN_DIV:
            if (r == 0) raiseError("Compile-time division by zero detected", "E0005");
            res = l / r;
            break;
        case TOKEN_MODULO:
            if (r == 0) raiseError("Compile-time modulo by zero detected", "E0005");
            res = l % r;
            break;
        default: break;
    }
    if (overflow || res > INT_MAX || res < INT_MIN) {
        raiseError("Compile-time integer overflow", "E0012.1");
    }
    return (int)res;
}

Token* peek(const Token* t, const int* c) {
    return (Token*)&t[*c];
}

Token* advance(const Token* t, int* c) {
    (*c)++;
    return (Token*)&t[(*c)-1];
}

static ASTNode* make_literal(const char* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) raiseError("Memory allocation failed", "E0004");
    node->type = NODE_LITERAL;
    snprintf(node->data.literal.value, sizeof(node->data.literal.value), "%s", value);
    return node;
}

static ASTNode* make_variable(const char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) raiseError("Memory allocation failed", "E0004");
    node->type = NODE_VARIABLE;
    snprintf(node->data.literal.value, sizeof(node->data.literal.value), "%s", name);
    return node;
}

static ASTNode* make_binary(TokenType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) raiseError("Memory allocation failed", "E0004");
    node->type = NODE_BINARY_OP;
    node->data.binary_op.op = op;
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    return node;
}

static ASTNode* make_reassign(const char* name, ASTNode* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) raiseError("Memory allocation failed", "E0004");
    node->type = NODE_REASSIGN;
    snprintf(node->data.reassign.name, sizeof(node->data.reassign.name), "%s", name);
    node->data.reassign.value = value;
    return node;
}

static void qualify_name(char* dest, size_t dest_size, const char* ns, const char* name);

// Parse an assignment-style statement: `name = expr`, `name += expr`,
// `name++`, etc. All forms are desugared into a plain NODE_REASSIGN.
static ASTNode* parse_update(const Token* t, int* c, const char* ns) {
    Token* current = peek(t, c);
    if (current->type != TOKEN_NAME) {
        raiseError("Expected variable name in assignment", "E0025");
        return NULL;
    }

    Token* name_token = advance(t, c);
    char qualified[64];
    qualify_name(qualified, sizeof(qualified), ns, name_token->value);

    Token* op = peek(t, c);
    ASTNode* value = NULL;
    switch (op->type) {
        case TOKEN_INC:
            advance(t, c);
            value = make_binary(TOKEN_ADD, make_variable(qualified), make_literal("1"));
            break;
        case TOKEN_DEC:
            advance(t, c);
            value = make_binary(TOKEN_SUB, make_variable(qualified), make_literal("1"));
            break;
        case TOKEN_ASSIGN:
        case TOKEN_VAR_INFER:
            advance(t, c);
            value = parse_expression(t, c, ns);
            break;
        case TOKEN_ADD_ASSIGN:
            advance(t, c);
            value = make_binary(TOKEN_ADD, make_variable(qualified), parse_expression(t, c, ns));
            break;
        case TOKEN_SUB_ASSIGN:
            advance(t, c);
            value = make_binary(TOKEN_SUB, make_variable(qualified), parse_expression(t, c, ns));
            break;
        case TOKEN_STAR_ASSIGN:
            advance(t, c);
            value = make_binary(TOKEN_STAR, make_variable(qualified), parse_expression(t, c, ns));
            break;
        case TOKEN_DIV_ASSIGN:
            advance(t, c);
            value = make_binary(TOKEN_DIV, make_variable(qualified), parse_expression(t, c, ns));
            break;
        case TOKEN_MOD_ASSIGN:
            advance(t, c);
            value = make_binary(TOKEN_MODULO, make_variable(qualified), parse_expression(t, c, ns));
            break;
        default:
            raiseError("Expected assignment operator after name", "E0025");
            break;
    }

    return make_reassign(qualified, value);
}

// Copy `name` into dest, prefixed with the current namespace unless the name
// is already qualified. Errors out rather than overflowing or truncating.
static void qualify_name(char* dest, size_t dest_size, const char* ns, const char* name) {
    int n;
    if (ns != NULL && ns[0] != '\0' && strchr(name, '.') == NULL) {
        n = snprintf(dest, dest_size, "%s.%s", ns, name);
    } else {
        n = snprintf(dest, dest_size, "%s", name);
    }
    if (n < 0 || (size_t)n >= dest_size) {
        raiseError("Qualified name is too long", "E0011");
    }
}

ASTNode* parse_unary(const Token* t, int* c, const char* ns) {
    if (peek(t, c)->type == TOKEN_TILDE) {
        Token* op_token = advance(t,c);

        ASTNode* operand = parse_unary(t, c, ns);

        ASTNode* unary_node = (ASTNode*)malloc(sizeof(ASTNode));
        if (!unary_node) raiseError("Memory allocation failed", "E0004");

        unary_node->type = NODE_UNARY_OP;
        unary_node->data.unary_op.op = op_token->type;
        unary_node->data.unary_op.operand = operand;

        return unary_node;
    }

    return parse_primary(t, c, ns);
}

ASTNode* parse_multiplicative(const Token* t, int* c, const char* ns) {
    ASTNode* left = parse_unary(t, c, ns);
    
    while (peek(t, c)->type == TOKEN_STAR || peek(t, c)->type == TOKEN_DIV || peek(t, c)->type == TOKEN_MODULO) {
        Token* op_token = advance(t, c);
        ASTNode* right = parse_unary(t, c, ns);

        if (left->type == NODE_LITERAL && right->type == NODE_LITERAL) {
            int res = fold_literals(op_token->type, left->data.literal.value, right->data.literal.value);

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
            int res = fold_literals(op_token->type, left->data.literal.value, right->data.literal.value);

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
        if (current->type == TOKEN_L_INT) {
            errno = 0;
            long long v = strtoll(lit_token->value, NULL, 10);
            if (errno == ERANGE || v > INT_MAX) {
                raiseError("Integer literal out of range", "E0012");
            }
        }
        return node;
    }

    if (current->type == TOKEN_NAME) {
        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        if (!node) raiseError("Memory allocation failed", "E0004");
        
        Token* var_token = advance(t, c);

        if (peek(t, c)->type == TOKEN_LPAREN) {
            advance(t, c);
            node->type = NODE_CALL;
            node->data.fun_call.returnType[0] = '\0';
            node->data.fun_call.arguments = NULL;
            node->data.fun_call.arg_count = 0;
            qualify_name(node->data.fun_call.name, sizeof(node->data.fun_call.name), ns, var_token->value);

            if (peek(t, c)->type != TOKEN_RPAREN) {
                int arg_capacity = 4;
                node->data.fun_call.arguments = (ASTNode**)malloc(sizeof(ASTNode*) * arg_capacity);
                if (!node->data.fun_call.arguments) raiseError("Memory allocation failed", "E0004");

                while (peek(t, c)->type != TOKEN_RPAREN && peek(t, c)->type != TOKEN_EOF) {
                    if (node->data.fun_call.arg_count >= arg_capacity) {
                        arg_capacity *= 2;
                        ASTNode** tmp = (ASTNode**)realloc(node->data.fun_call.arguments, sizeof(ASTNode*) * arg_capacity);
                        if (!tmp) raiseError("Memory allocation failed", "E0004");
                        node->data.fun_call.arguments = tmp;
                    }

                    ASTNode* arg = parse_expression(t, c, ns);
                    node->data.fun_call.arguments[node->data.fun_call.arg_count++] = arg;

                    if (peek(t, c)->type == TOKEN_COMMA) {
                        advance(t, c);
                    } else {
                        break;
                    }
                }
            }

            if (peek(t, c)->type == TOKEN_RPAREN) {
                advance(t, c);
            } else {
                raiseError("Expected ')' after function call", "E0009");
            }
            return node;
        }

        node->type = NODE_VARIABLE;
        qualify_name(node->data.literal.value, sizeof(node->data.literal.value), ns, var_token->value);
        return node;
        
    } else if (current->type == TOKEN_REPEAT) {
        return parse_repeat(t, c, ns);
    } else if (current->type == TOKEN_ELSE || current->type == TOKEN_IF || current->type == TOKEN_ELSEIF) {
        return parse_if(t,c,ns);
    } else if (current->type == TOKEN_WHILE) {
        return parse_while(t, c, ns);
    } else if (current->type == TOKEN_FOR) {
        return parse_for(t, c, ns);
    } else if (current->type == TOKEN_FUN) {
        raiseError("Unexpected token: function definitions must be parsed at the top level", "E0009");
        return NULL;
    }

    raiseError("Unexpected token: expected a variable or literal expression", "E0001:1");
    return NULL; 
}

ASTNode* parse_expression(const Token* t, int* c, const char* ns) {
    ASTNode* left = parse_equality(t, c, ns);

    while(peek(t, c)->type == TOKEN_AMPERSAND || peek(t,c)->type == TOKEN_PIPE ||
            peek(t, c)->type == TOKEN_CARET){
        Token* op_token = advance(t,c);
        ASTNode* right = parse_equality(t, c, ns);

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

ASTNode* parse_relational(const Token* t, int* c, const char* ns) {
    ASTNode* left = parse_additive(t, c, ns);

    while (peek(t, c)->type == TOKEN_LT || peek(t, c)->type == TOKEN_GT ||
           peek(t, c)->type == TOKEN_LE || peek(t, c)->type == TOKEN_GE) {
        Token* op_token = advance(t, c);
        ASTNode* right = parse_additive(t, c, ns);

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

ASTNode* parse_equality(const Token* t, int* c, const char* ns) {
    ASTNode* left = parse_relational(t, c, ns);
    while (peek(t, c)->type == TOKEN_EQUAL || peek(t, c)->type == TOKEN_NE) {
        Token* op_token = advance(t, c);
        ASTNode* right = parse_relational(t, c, ns);

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

ASTNode* parse_statement(const Token* t, int* c, const char* ns) {
    Token* current = peek(t, c);

    if (current->type == TOKEN_VAR_DEF) {
        ASTNode* result = (ASTNode*)malloc(sizeof(ASTNode));
        if (!result) raiseError("Memory allocation failed", "E0004");
        
        result->type = NODE_DECLARATION;
        advance(t, c);
        
        if (peek(t, c)->type == TOKEN_NAME) {
            Token* name_token = advance(t, c);

            qualify_name(result->data.var_decl.name, sizeof(result->data.var_decl.name), ns, name_token->value);
        } else {
            raiseError("Missing variable name after 'val'", "E0005");
        }
        
        if (peek(t, c)->type == TOKEN_VAR_INFER) {
            advance(t, c); 
        } else {
            raiseError("Missing ':='  in variable declaration", "E0006");
        }
        
        result->data.var_decl.value = parse_expression(t, c, ns);
        return result;
    } else if (current->type == TOKEN_INT) {
        ASTNode* result = (ASTNode*)malloc(sizeof(ASTNode));
        if (!result) raiseError("Memory allocation failed", "E0004");
        
        result->type = NODE_DECLARATION;
        advance(t, c);
        
        if (peek(t, c)->type == TOKEN_NAME) {
            Token* name_token = advance(t, c);

            qualify_name(result->data.var_decl.name, sizeof(result->data.var_decl.name), ns, name_token->value);
        } else {
            raiseError("Missing variable name after 'int'", "E0005");
        }
        
        if (peek(t, c)->type == TOKEN_ASSIGN) {
            advance(t, c); 
        } else {
            raiseError("Missing '=' in variable declaration", "E0006");
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

            qualify_name(result->data.var_decl.name, sizeof(result->data.var_decl.name), ns, name_token->value);
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

    else if (current->type == TOKEN_RETURN) {
        ASTNode* result = (ASTNode*)malloc(sizeof(ASTNode));
        if (!result) raiseError("Memory allocation failed", "E0004");

        result->type = NODE_RETURN;
        advance(t, c); // consume 'return'

        // return may be followed by an expression
        result->data.return_stmt.value = parse_expression(t, c, ns);
        return result;
    }

    else if (current->type == TOKEN_NAME) {
        Token* next = (Token*)&t[*c + 1];
        if (next->type == TOKEN_ASSIGN || next->type == TOKEN_VAR_INFER ||
            next->type == TOKEN_ADD_ASSIGN || next->type == TOKEN_SUB_ASSIGN ||
            next->type == TOKEN_STAR_ASSIGN || next->type == TOKEN_DIV_ASSIGN ||
            next->type == TOKEN_MOD_ASSIGN || next->type == TOKEN_INC ||
            next->type == TOKEN_DEC) {
            return parse_update(t, c, ns);
        }

        return parse_expression(t, c, ns);
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
    } else if (current->type == TOKEN_IF) {
        return parse_if(t, c, ns);
    } else if (current->type == TOKEN_WHILE) {
        return parse_while(t, c, ns);
    } else if (current->type == TOKEN_FOR) {
        return parse_for(t, c, ns);
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
    // the body is unrolled repeat_count times at compile time, so an
    // unchecked count would let a two-line program emit gigabytes of IR
    errno = 0;
    long long repeat_count = strtoll(value_token->value, NULL, 10);
    if (errno == ERANGE || repeat_count > 1000000) {
        raiseError("Repeat count too large (max 1000000)", "E0013");
    }
    newNode->data.repeat_stmt.repeat_count = (int)repeat_count;

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

// Collect statements until `end` (or EOF). On success the trailing `end`
// has been consumed and the array is returned; `out_count` receives its size.
static ASTNode** parse_block_statements(const Token* t, int* c, const char* ns, int* out_count, const char* error_msg) {
    int cap = 8;
    int count = 0;
    ASTNode** statements = (ASTNode**)malloc(sizeof(ASTNode*) * cap);
    if (!statements) raiseError("Memory allocation failed", "E0004");

    while (peek(t, c)->type != TOKEN_END && peek(t, c)->type != TOKEN_EOF) {
        ASTNode* inner = parse_statement(t, c, ns);
        if (inner != NULL) {
            if (count >= cap) {
                cap *= 2;
                ASTNode** tmp = (ASTNode**)realloc(statements, sizeof(ASTNode*) * cap);
                if (!tmp) {
                    free(statements);
                    raiseError("Memory allocation failed while expanding block statements", "E0004");
                }
                statements = tmp;
            }
            statements[count++] = inner;
        }
    }

    if (peek(t, c)->type == TOKEN_END) {
        advance(t, c);
    } else {
        free(statements);
        raiseError("Unexpected end of file: missing 'end' for block", (char*)error_msg);
    }

    *out_count = count;
    return statements;
}

ASTNode* parse_while(const Token* t, int* c, const char* ns) {
    advance(t, c); // consume 'while'

    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) raiseError("Memory allocation failed", "E0004");
    node->type = NODE_WHILE;
    node->data.while_stmt.condition = NULL;

    ASTNode* condition = NULL;
    if (peek(t, c)->type == TOKEN_LPAREN) {
        advance(t, c);
        condition = parse_expression(t, c, ns);
        if (peek(t, c)->type == TOKEN_RPAREN) {
            advance(t, c);
        } else {
            raiseError("Missing ')' after while condition", "E0023.2");
        }
    } else {
        condition = parse_expression(t, c, ns);
    }
    node->data.while_stmt.condition = condition;

    node->data.while_stmt.statements = parse_block_statements(t, c, ns, &node->data.while_stmt.count, "E0010.2");

    return node;
}

ASTNode* parse_for(const Token* t, int* c, const char* ns) {
    advance(t, c); // consume 'for'

    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) raiseError("Memory allocation failed", "E0004");
    node->type = NODE_FOR;
    node->data.for_stmt.init = NULL;
    node->data.for_stmt.condition = NULL;
    node->data.for_stmt.increment = NULL;

    Token* current = peek(t, c);

    // Modern for: `for entry in list` - iterates the loop variable from 0
    // while it is smaller than the value of the given expression.
    if (current->type == TOKEN_NAME && ((Token*)&t[*c + 1])->type == TOKEN_IN) {
        Token* var_token = advance(t, c); // consume loop variable name
        advance(t, c); // consume 'in'

        char qualified[64];
        qualify_name(qualified, sizeof(qualified), ns, var_token->value);

        ASTNode* list_value = parse_expression(t, c, ns);

        ASTNode* init = (ASTNode*)malloc(sizeof(ASTNode));
        if (!init) raiseError("Memory allocation failed", "E0004");
        init->type = NODE_DECLARATION;
        strcpy(init->data.var_decl.name, qualified);
        init->data.var_decl.value = make_literal("0");

        node->data.for_stmt.init = init;
        node->data.for_stmt.condition = make_binary(TOKEN_LT, make_variable(qualified), list_value);
        node->data.for_stmt.increment = make_reassign(qualified, make_binary(TOKEN_ADD, make_variable(qualified), make_literal("1")));
    }
    // Classic for: `for int i=0; i<10; i++` or `for i=0; i<10; i=i+1`
    else if (current->type == TOKEN_INT || current->type == TOKEN_VAR_DEF ||
             current->type == TOKEN_CONST || current->type == TOKEN_NAME) {
        ASTNode* init = NULL;

        if (current->type == TOKEN_INT) {
            advance(t, c);
            Token* name_token = peek(t, c);
            if (name_token->type != TOKEN_NAME) raiseError("Missing variable name after 'int'", "E0005");
            advance(t, c);
            if (peek(t, c)->type != TOKEN_ASSIGN) raiseError("Missing '=' in for-loop init", "E0006");
            advance(t, c);

            init = (ASTNode*)malloc(sizeof(ASTNode));
            if (!init) raiseError("Memory allocation failed", "E0004");
            init->type = NODE_DECLARATION;
            qualify_name(init->data.var_decl.name, sizeof(init->data.var_decl.name), ns, name_token->value);
            init->data.var_decl.value = parse_expression(t, c, ns);
        } else if (current->type == TOKEN_VAR_DEF || current->type == TOKEN_CONST) {
            advance(t, c);
            Token* name_token = peek(t, c);
            if (name_token->type != TOKEN_NAME) raiseError("Missing variable name after 'val'", "E0005");
            advance(t, c);
            if (peek(t, c)->type != TOKEN_VAR_INFER) raiseError("Missing ':=' in for-loop init", "E0006");
            advance(t, c);

            init = (ASTNode*)malloc(sizeof(ASTNode));
            if (!init) raiseError("Memory allocation failed", "E0004");
            init->type = NODE_DECLARATION;
            qualify_name(init->data.var_decl.name, sizeof(init->data.var_decl.name), ns, name_token->value);
            init->data.var_decl.value = parse_expression(t, c, ns);
        } else { // TOKEN_NAME
            init = parse_update(t, c, ns);
        }
        node->data.for_stmt.init = init;

        if (peek(t, c)->type != TOKEN_SEMICOLON) raiseError("Expected ';' after for-loop init", "E0026");
        advance(t, c);

        node->data.for_stmt.condition = parse_expression(t, c, ns);

        if (peek(t, c)->type != TOKEN_SEMICOLON) raiseError("Expected ';' after for-loop condition", "E0026");
        advance(t, c);

        node->data.for_stmt.increment = parse_update(t, c, ns);
    } else {
        raiseError("Expected loop header after 'for'", "E0024");
    }

    node->data.for_stmt.statements = parse_block_statements(t, c, ns, &node->data.for_stmt.count, "E0010.3");

    return node;
}

ASTNode* parse(const Token* tokens, int count) {
    (void)count;
    int current_token = 0;
    
    char ns_stack[10][64];
    int ns_depth = 0;
    // worst case: 10 names of 63 chars, 9 dots, NUL
    char current_namespace[10 * 64] = "";
    
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

        if (current->type == TOKEN_FUN) {
            advance(tokens, &current_token);
            ASTNode* funNode = (ASTNode*)malloc(sizeof(ASTNode));
            if (!funNode) raiseError("Memory allocation failed", "E0004");
            funNode->type = NODE_FUN_DEF;

            Token* name_token = peek(tokens, &current_token);
            if (name_token->type != TOKEN_NAME) raiseError("Expected identifier after 'fun'", "E0009");

            qualify_name(funNode->data.fun_def.name, sizeof(funNode->data.fun_def.name), current_namespace, name_token->value);

            advance(tokens, &current_token); // consume name

            funNode->data.fun_def.arguments = NULL;
            if (peek(tokens, &current_token)->type == TOKEN_LPAREN) {
                advance(tokens, &current_token); // consume '('
                fun_args* arguments = NULL;
                int arg_count = 0;

                if (peek(tokens, &current_token)->type != TOKEN_RPAREN) {
                    arguments = (fun_args*)malloc(sizeof(fun_args) * 100);
                    if (!arguments) raiseError("Memory allocation failed", "E0004");
                    for (int i = 0; i < 100; i++) {
                        arguments[i].name[0] = '\0';
                        arguments[i].type[0] = '\0';
                    }

                    char next = 't'; // t -> type, n -> name
                    fun_args newarg;
                    while (peek(tokens, &current_token)->type != TOKEN_RPAREN && peek(tokens, &current_token)->type != TOKEN_EOF) {
                        if (next == 't') {
                            TokenType type_token = peek(tokens, &current_token)->type;
                            if (type_token != TOKEN_NAME && type_token != TOKEN_INT && type_token != TOKEN_FLOAT && type_token != TOKEN_CHAR) {
                                raiseError("Expected parameter type", "E0009");
                            }
                            strcpy(newarg.type, peek(tokens, &current_token)->value);
                            next = 'n';
                            advance(tokens, &current_token);
                        } else if (next == 'n') {
                            if (peek(tokens, &current_token)->type != TOKEN_NAME) raiseError("Expected parameter name", "E0009");
                            strcpy(newarg.name, peek(tokens, &current_token)->value);
                            advance(tokens, &current_token);
                            arguments[arg_count++] = newarg;
                            next = 'c';
                        } else if (peek(tokens, &current_token)->type == TOKEN_COMMA) {
                            advance(tokens, &current_token);
                            if (arg_count >= 100) raiseError("Too many function parameters", "E0008");
                            next = 't';
                        } else {
                            raiseError("Invalid function parameter list", "E0009");
                        }
                    }
                }

                if (peek(tokens, &current_token)->type == TOKEN_RPAREN) {
                    advance(tokens, &current_token);
                    funNode->data.fun_def.arguments = arguments;
                } else {
                    if (arguments) free(arguments);
                    raiseError("Unterminated parameter list", "E0009");
                }
            } else {
                raiseError("Expected parenthesis", "E0009");
            }

            // optional return type
            if (peek(tokens, &current_token)->type == TOKEN_NAME || peek(tokens, &current_token)->type == TOKEN_L_INT || peek(tokens, &current_token)->type == TOKEN_L_FLOAT) {
                Token* rt = advance(tokens, &current_token);
                strcpy(funNode->data.fun_def.returnType, rt->value);
            } else if (peek(tokens, &current_token)->type == TOKEN_INT) {
                advance(tokens, &current_token);
                strcpy(funNode->data.fun_def.returnType, "int");
            } else if (peek(tokens, &current_token)->type == TOKEN_FLOAT) {
                advance(tokens, &current_token);
                strcpy(funNode->data.fun_def.returnType, "float");
            } else {
                funNode->data.fun_def.returnType[0] = '\0';
            }

            // parse function body into a program node until END
            ASTNode* body = (ASTNode*)malloc(sizeof(ASTNode));
            if (!body) raiseError("Memory allocation failed", "E0004");
            body->type = NODE_PROGRAM;
            body->data.program.count = 0;
            int body_capacity = 8;
            body->data.program.statements = (ASTNode**)malloc(sizeof(ASTNode*) * body_capacity);
            if (!body->data.program.statements) raiseError("Memory allocation failed", "E0004");

            while (peek(tokens, &current_token)->type != TOKEN_END && peek(tokens, &current_token)->type != TOKEN_EOF) {
                ASTNode* inner = parse_statement(tokens, &current_token, current_namespace);
                if (inner != NULL) {
                    if (body->data.program.count >= body_capacity) {
                        body_capacity *= 2;
                        ASTNode** tmp = (ASTNode**)realloc(body->data.program.statements, sizeof(ASTNode*) * body_capacity);
                        if (!tmp) { free(body->data.program.statements); free(body); raiseError("Memory allocation failed while expanding function body", "E0004"); }
                        body->data.program.statements = tmp;
                    }
                    body->data.program.statements[body->data.program.count++] = inner;
                }
            }

            if (peek(tokens, &current_token)->type == TOKEN_END) {
                advance(tokens, &current_token); // consume END
            } else {
                raiseError("Unexpected end of file: missing 'end' for function", "E0010");
            }

            funNode->data.fun_def.body = body;

            // append function node to program_node
            if (program_node->data.program.count >= statement_capacity) {
                statement_capacity *= 2;
                ASTNode** temp = (ASTNode**)realloc(program_node->data.program.statements, sizeof(ASTNode*) * statement_capacity);
                if (!temp) { free(program_node->data.program.statements); free(program_node); raiseError("Memory allocation failed while expanding statements", "E0004"); }
                program_node->data.program.statements = temp;
            }
            program_node->data.program.statements[program_node->data.program.count++] = funNode;
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

ASTNode* parse_if(const Token* t, int* c, const char* ns) {
    advance(t, c);
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) raiseError("Memory allocation failed", "E0004");
    node->type = NODE_IF;
    node->data.if_stmt.condition = NULL;

    int cap = 8;
    node->data.if_stmt.then_count = 0;
    node->data.if_stmt.then_statements = (ASTNode**)malloc(sizeof(ASTNode*) * cap);
    if (!node->data.if_stmt.then_statements) raiseError("Memory allocation failed", "E0004");
    node->data.if_stmt.else_node = NULL;

    // optional parentheses around condition: allow `if expr` or `if (expr)`
    ASTNode* condition = NULL;
    if (peek(t, c)->type == TOKEN_LPAREN) {
        advance(t, c);
        condition = parse_expression(t, c, ns);
        if (peek(t, c)->type == TOKEN_RPAREN) {
            advance(t, c);
        } else {
            raiseError("Missing ')' after if condition", "E0020.2");
        }
    } else {
        condition = parse_expression(t, c, ns);
    }
    node->data.if_stmt.condition = condition;

    while (peek(t, c)->type != TOKEN_ELSEIF && peek(t, c)->type != TOKEN_ELSE && peek(t, c)->type != TOKEN_END && peek(t, c)->type != TOKEN_EOF) {
        ASTNode* inner = parse_statement(t, c, ns);
        if (inner != NULL) {
            if (node->data.if_stmt.then_count >= cap) {
                cap *= 2;
                ASTNode** tmp = (ASTNode**)realloc(node->data.if_stmt.then_statements, sizeof(ASTNode*) * cap);
                if (!tmp) {
                    free(node->data.if_stmt.then_statements);
                    free(node);
                    raiseError("Memory allocation failed while expanding if statements", "E0004");
                }
                node->data.if_stmt.then_statements = tmp;
            }
            node->data.if_stmt.then_statements[node->data.if_stmt.then_count++] = inner;
        }
    }

    ASTNode* current_if = node;
    while (peek(t, c)->type == TOKEN_ELSEIF) {
        advance(t, c);

        ASTNode* elseif_cond = NULL;
        if (peek(t, c)->type == TOKEN_LPAREN) {
            advance(t, c);
            elseif_cond = parse_expression(t, c, ns);
            if (peek(t, c)->type == TOKEN_RPAREN) {
                advance(t, c);
            } else {
                raiseError("Missing ')' after elseif condition", "E0021.2");
            }
        } else {
            elseif_cond = parse_expression(t, c, ns);
        }

        ASTNode* elseif_node = (ASTNode*)malloc(sizeof(ASTNode));
        if (!elseif_node) raiseError("Memory allocation failed", "E0004");
        elseif_node->type = NODE_IF;
        elseif_node->data.if_stmt.condition = elseif_cond;
        int cap2 = 8;
        elseif_node->data.if_stmt.then_count = 0;
        elseif_node->data.if_stmt.then_statements = (ASTNode**)malloc(sizeof(ASTNode*) * cap2);
        if (!elseif_node->data.if_stmt.then_statements) raiseError("Memory allocation failed", "E0004");
        elseif_node->data.if_stmt.else_node = NULL;

        while (peek(t, c)->type != TOKEN_ELSEIF && peek(t, c)->type != TOKEN_ELSE && peek(t, c)->type != TOKEN_END && peek(t, c)->type != TOKEN_EOF) {
            ASTNode* inner = parse_statement(t, c, ns);
            if (inner != NULL) {
                if (elseif_node->data.if_stmt.then_count >= cap2) {
                    cap2 *= 2;
                    ASTNode** tmp = (ASTNode**)realloc(elseif_node->data.if_stmt.then_statements, sizeof(ASTNode*) * cap2);
                    if (!tmp) {
                        free(elseif_node->data.if_stmt.then_statements);
                        free(elseif_node);
                        raiseError("Memory allocation failed while expanding elseif statements", "E0004");
                    }
                    elseif_node->data.if_stmt.then_statements = tmp;
                }
                elseif_node->data.if_stmt.then_statements[elseif_node->data.if_stmt.then_count++] = inner;
            }
        }

        current_if->data.if_stmt.else_node = elseif_node;
        current_if = elseif_node;
    }

    if (peek(t, c)->type == TOKEN_ELSE) {
        advance(t, c);

        // create a program-like node to hold else statements
        ASTNode* else_block = (ASTNode*)malloc(sizeof(ASTNode));
        if (!else_block) raiseError("Memory allocation failed", "E0004");
        else_block->type = NODE_PROGRAM;
        int cap3 = 8;
        else_block->data.program.count = 0;
        else_block->data.program.statements = (ASTNode**)malloc(sizeof(ASTNode*) * cap3);

        while (peek(t, c)->type != TOKEN_END && peek(t, c)->type != TOKEN_EOF) {
            ASTNode* inner = parse_statement(t, c, ns);
            if (inner != NULL) {
                if (else_block->data.program.count >= cap3) {
                    cap3 *= 2;
                    ASTNode** tmp = (ASTNode**)realloc(else_block->data.program.statements, sizeof(ASTNode*) * cap3);
                    if (!tmp) {
                        free(else_block->data.program.statements);
                        free(else_block);
                        raiseError("Memory allocation failed while expanding else statements", "E0004");
                    }
                    else_block->data.program.statements = tmp;
                }
                else_block->data.program.statements[else_block->data.program.count++] = inner;
            }
        }

        if (peek(t, c)->type == TOKEN_END) {
            advance(t, c);
        } else {
            raiseError("Unexpected end of file: missing 'end' for if/else block", "E0022");
        }

        current_if->data.if_stmt.else_node = else_block;
        return node;
    }

    if (peek(t, c)->type == TOKEN_END) {
        advance(t, c);
        return node;
    }

    return node;
}