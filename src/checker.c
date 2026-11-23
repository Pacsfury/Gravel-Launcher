#include "../include/checker.h"
#include "../include/tokens.h"

// It can be locally defined as is because none of the values aren't returned,
// so that they would need to have an actual place inside the memory
// static is also just so it's inside this file and that in can only be defined once and dependant on the source file.
static
grammar_checker_t
checker_info = {0};

bool checkGrammar(Token* tokens, int len) {
    int i = 0;
    while (i < len) {
        Token token = tokens[i];
        switch (token.type) {
            case TOKEN_SCHO:
                if (i + 1 < len && tokens[i + 1].type == TOKEN_LPAREN) {
                    int j = i + 2;
                    int depth = 1;
                    while (j < len && depth > 0) {
                        if (tokens[j].type == TOKEN_LPAREN)
                            depth++;
                        else if (tokens[j].type == TOKEN_RPAREN)
                            depth--;
                        j++;
                    }
                    if (depth != 0) {
                        raiseError("Invalid scho statement: missing closing parenthesis", "E9999");
                    }

                    if (i + 2 >= j) {
                        raiseError("Invalid scho statement: empty parentheses", "E9999");
                    }
                    i = j;
                } else {
                    raiseError("Invalid scho statement", "E9999");
                }
                break;
            case TOKEN_NAMESPACE:
                if (i + 1 < len && tokens[i + 1].type == TOKEN_NAME) {
                    checker_info.end_indent++;
                    i += 2; /* namespace and its name */
                } else {
                    raiseError("Invalid namespace definition", "E9999");
                }
                break;
            case TOKEN_IF:
                checker_info.end_indent++;
                checker_info.if_indent++;
                checker_info.on_if = true;
                i++;
                break;
            case TOKEN_ELSEIF:
                if (checker_info.if_indent > 0) {
                    i++;
                } else {
                    raiseError("Invalid ELSEIF without previous IF", "E9999");
                }
                break;
            case TOKEN_ELSE:
                if (checker_info.if_indent > 0) {
                    checker_info.on_if = false;
                    checker_info.if_indent--;
                    i++;
                } else {
                    raiseError("Invalid ELSE without previous IF or ELSEIF", "E9999");
                }
                break;
            case TOKEN_WHILE:
                checker_info.end_indent++;
                i++;
                break;
            case TOKEN_FOR:
                checker_info.end_indent++;
                i++;
                break;
            case TOKEN_FUN:
                if (i + 2 < len && tokens[i + 2].type == TOKEN_LPAREN) {
                    int j = 0;
                    while (i + j < len && tokens[i + j].type != TOKEN_RPAREN && tokens[i + j].type != TOKEN_EOF) {
                        j++;
                    }
                    if (i + j >= len || tokens[i + j].type != TOKEN_RPAREN) {
                        raiseError("Expected ')'. Found 'EOF' instead", "E9999");
                    }
                    i += j + 1;

                } else {
                    i++;
                }
                break;
            default:
                i++;
                break;
        }
    }
    return true;
}