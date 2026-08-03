#define MAX_EMITTED_GLOBALS 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/tokens.h"
#include "../include/ast.h"
#include "../include/tollvm.h"
#include "../include/argc.h"

static char emitted_globals[MAX_EMITTED_GLOBALS][256];
static int emitted_globals_count = 0;
static const char* current_function_return_type = "void";
static int current_function_has_return = 0;
static int current_function_param_count = 0;
static char current_function_param_names[32][64];
static char current_function_param_ptrs[32][64];
static char current_function_param_types[32][32];
#define MAX_EMITTED_FUNCS 1024
static char emitted_funcs[MAX_EMITTED_FUNCS][256];
static char emitted_funcs_ret[MAX_EMITTED_FUNCS][32];
static int emitted_funcs_count = 0;

static void register_emitted_function(const char* name, const char* ret) {
    if (emitted_funcs_count < MAX_EMITTED_FUNCS) {
        strcpy(emitted_funcs[emitted_funcs_count], name);
        strncpy(emitted_funcs_ret[emitted_funcs_count], ret, sizeof(emitted_funcs_ret[0]) - 1);
        emitted_funcs_ret[emitted_funcs_count][sizeof(emitted_funcs_ret[0]) - 1] = '\0';
        emitted_funcs_count++;
    }
}

static const char* lookup_emitted_function_return(const char* name) {
    for (int i = 0; i < emitted_funcs_count; i++) {
        if (strcmp(emitted_funcs[i], name) == 0) return emitted_funcs_ret[i];
    }
    return NULL;
}

static int already_emitted(const char* name) {
    for (int i = 0; i < emitted_globals_count; i++) {
        if (strcmp(emitted_globals[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

static void mark_emitted(const char* name) {
    if (emitted_globals_count < MAX_EMITTED_GLOBALS) {
        strcpy(emitted_globals[emitted_globals_count++], name);
    }
}

static void emit_globals_for_statement(ASTNode* stmt, FILE* outf) {
    if (!stmt) return;

    if (stmt->type == NODE_DECLARATION || stmt->type == NODE_CONSTANT) {
        if (!already_emitted(stmt->data.var_decl.name)) {
            fprintf(outf, "@%s = global i32 0, align 4\n", stmt->data.var_decl.name);
            mark_emitted(stmt->data.var_decl.name);
        }
        return;
    }

    if(stmt->type == NODE_REASSIGN){
        if (!already_emitted(stmt->data.var_decl.name)){
            raiseError("An undeclared variable cannot be reassigned", "E0030");
        }
        return;
    }

    if (stmt->type == NODE_FUN_DEF && stmt->data.fun_def.body && stmt->data.fun_def.body->type == NODE_PROGRAM) {
        for (int i = 0; i < stmt->data.fun_def.body->data.program.count; i++) {
            emit_globals_for_statement(stmt->data.fun_def.body->data.program.statements[i], outf);
        }
        return;
    }

    if (stmt->type == NODE_REPEAT) {
        for (int i = 0; i < stmt->data.repeat_stmt.count; i++) {
            emit_globals_for_statement(stmt->data.repeat_stmt.statements[i], outf);
        }
        return;
    }

    if (stmt->type == NODE_IF) {
        for (int i = 0; i < stmt->data.if_stmt.then_count; i++) {
            emit_globals_for_statement(stmt->data.if_stmt.then_statements[i], outf);
        }
        if (stmt->data.if_stmt.else_node) {
            if (stmt->data.if_stmt.else_node->type == NODE_IF) {
                emit_globals_for_statement(stmt->data.if_stmt.else_node, outf);
            } else if (stmt->data.if_stmt.else_node->type == NODE_PROGRAM) {
                for (int j = 0; j < stmt->data.if_stmt.else_node->data.program.count; j++) {
                    emit_globals_for_statement(stmt->data.if_stmt.else_node->data.program.statements[j], outf);
                }
            }
        }
    }
}

static const char* llvm_type_for(const char* type_name) {
    if (!type_name || type_name[0] == '\0') return "void";
    if (strcmp(type_name, "int") == 0) return "i32";
    if (strcmp(type_name, "float") == 0) return "float";
    if (strcmp(type_name, "void") == 0) return "void";
    return "i32";
}

static char* compile_node(FILE* outf, ASTNode* node, int* register_count);

static void emit_function_definition(FILE* outf, ASTNode* node) {
    if (!node || node->type != NODE_FUN_DEF) return;

    const char* return_type = llvm_type_for(node->data.fun_def.returnType);
    register_emitted_function(node->data.fun_def.name, return_type);
    char arguments[256] = "";
    current_function_param_count = 0;

    if (node->data.fun_def.arguments) {
        size_t max_params = sizeof(current_function_param_names) / sizeof(current_function_param_names[0]);

        for (int i = 0; i < 32; i++) {
            fun_args* arg = &node->data.fun_def.arguments[i];
            if (arg->name[0] == '\0') break;

            if (current_function_param_count >= max_params) {
                break;
            }

            const char* arg_type = llvm_type_for(arg->type);

            size_t current_len = strlen(arguments);
            if (current_len < sizeof(arguments)) {
                snprintf(arguments + current_len, 
                        sizeof(arguments) - current_len, 
                        "%s%s %%%s",
                        (arguments[0] != '\0') ? ", " : "",
                        arg_type,
                        arg->name);
            }

            strncpy(current_function_param_names[current_function_param_count], 
                    arg->name, 
                    sizeof(current_function_param_names[0]) - 1);
            current_function_param_names[current_function_param_count][sizeof(current_function_param_names[0]) - 1] = '\0';

            strncpy(current_function_param_types[current_function_param_count], 
                    arg->type, 
                    sizeof(current_function_param_types[0]) - 1);
            current_function_param_types[current_function_param_count][sizeof(current_function_param_types[0]) - 1] = '\0';

            snprintf(current_function_param_ptrs[current_function_param_count], 
                    sizeof(current_function_param_ptrs[0]), 
                    "%s.addr", arg->name);

            current_function_param_count++;
        }
    }
    fprintf(outf, "define %s @%s(%s) {\n", return_type, node->data.fun_def.name, arguments);
    fprintf(outf, "entry:\n");

    for (int i = 0; i < current_function_param_count; i++) {
        const char* arg_type = llvm_type_for(current_function_param_types[i]);
        fprintf(outf, "    %%%s = alloca %s, align 4\n", current_function_param_ptrs[i], arg_type);
        fprintf(outf, "    store %s %%%s, ptr %%%s, align 4\n", arg_type, current_function_param_names[i], current_function_param_ptrs[i]);
    }

    int function_register_count = 1;
    /* set current function return type for use by compile_node when emitting returns */
    current_function_return_type = return_type;
    current_function_has_return = 0;
    if (node->data.fun_def.body && node->data.fun_def.body->type == NODE_PROGRAM) {
        for (int i = 0; i < node->data.fun_def.body->data.program.count; i++) {
            ASTNode* stmt = node->data.fun_def.body->data.program.statements[i];
            char* leftover = compile_node(outf, stmt, &function_register_count);
            if (leftover) free(leftover);
        }
    }

    /* reset current function return type after body compiled */
    const int had_ret = current_function_has_return;
    current_function_return_type = "void";
    current_function_has_return = 0;

    if (!had_ret) {
        if (strcmp(return_type, "void") == 0) {
            fprintf(outf, "    ret void\n");
        } else if (strcmp(return_type, "float") == 0) {
            fprintf(outf, "    ret float 0.0\n");
        } else {
            fprintf(outf, "    ret %s 0\n", return_type);
        }
    }

    fprintf(outf, "}\n\n");
}

// compile_node returns NULL for statements; require a value when the node is
// used in a value position (operand, argument, condition, ...).
static char* compile_value_node(FILE* outf, ASTNode* node, int* register_count) {
    char* val = compile_node(outf, node, register_count);
    if (!val) {
        raiseError("Expression does not produce a value", "E0032");
    }
    return val;
}

static char* safe_strdup(const char* s) {
    char* d = malloc(strlen(s) + 1);
    if (d == NULL) {
        fprintf(stderr, "Memory allocation failed in LLVM generator\n");
        exit(EXIT_FAILURE);
    }
    strcpy(d, s);
    return d;
}

static int label_counter = 0;

void llvm_scho(FILE* outf, const char* val_to_print) {
    fprintf(outf, "    call void @cprint(i32 %s)\n", val_to_print);
}

static char* compile_node(FILE* outf, ASTNode* node, int* register_count) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_LITERAL: {
            const char* v = node->data.literal.value;

            if (strcmp(v, "\\n") == 0) {
                return safe_strdup("10");
            }

            if (strlen(v) == 1 && !isdigit((unsigned char)v[0])) {
                char buf[8];
                snprintf(buf, sizeof(buf), "%d", (int)(unsigned char)v[0]);
                return safe_strdup(buf);
            }
            return safe_strdup(v);
        }

        case NODE_REASSIGN: {
            char* val = compile_value_node(outf, node->data.reassign.value, register_count);
            int arg_index = -1;
            for (int i = 0; i < current_function_param_count; i++) {
                if (strcmp(node->data.reassign.name, current_function_param_names[i]) == 0) {
                    arg_index = i;
                    break;
                }
            }
            if (arg_index >= 0) {
                const char* arg_type = llvm_type_for(current_function_param_types[arg_index]);
                fprintf(outf, "    store %s %s, ptr %%%s, align 4\n", arg_type, val, current_function_param_ptrs[arg_index]);
            } else {
                fprintf(outf, "    store i32 %s, ptr @%s, align 4\n", val, node->data.reassign.name);
            }
            free(val);
            return NULL;
        }


        case NODE_VARIABLE: {
            for (int i = 0; i < current_function_param_count; i++) {
                if (strcmp(node->data.literal.value, current_function_param_names[i]) == 0) {
                    const char* arg_type = llvm_type_for(current_function_param_types[i]);
                    int reg = (*register_count)++;
                    fprintf(outf, "    %%%d = load %s, ptr %%%s, align 4\n", reg, arg_type, current_function_param_ptrs[i]);
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%%%d", reg);
                    return safe_strdup(buf);
                }
            }

            int reg = (*register_count)++;
            fprintf(outf, "    %%%d = load i32, ptr @%s, align 4\n", reg, node->data.literal.value);
            
            char buf[32];
            snprintf(buf, sizeof(buf), "%%%d", reg);
            return safe_strdup(buf);
        }

        case NODE_CONSTANT:
        case NODE_DECLARATION: {
            if (node->data.var_decl.value) {
                char* val = compile_value_node(outf, node->data.var_decl.value, register_count);
                fprintf(outf, "    store i32 %s, ptr @%s, align 4\n", val, node->data.var_decl.name);
                free(val);
            }
            return NULL; 
        }

        case NODE_BINARY_OP: {
            char* left = compile_value_node(outf, node->data.binary_op.left, register_count);
            char* right = compile_value_node(outf, node->data.binary_op.right, register_count);
            
            const char* op_str = "";
            switch(node->data.binary_op.op) {
                case TOKEN_ADD:    op_str = "add"; break;
                case TOKEN_SUB:    op_str = "sub"; break;
                case TOKEN_STAR:   op_str = "mul"; break;
                case TOKEN_DIV:    op_str = "sdiv"; break;
                case TOKEN_MODULO: op_str = "srem"; break;
                case TOKEN_EQUAL:  op_str = "eq"; break;
                default:           op_str = "add"; break;
            }
            if (node->data.binary_op.op == TOKEN_EQUAL) {
                int cmp_reg = (*register_count)++;
                fprintf(outf, "    %%%d = icmp eq i32 %s, %s\n", cmp_reg, left, right);
                int zext_reg = (*register_count)++;
                fprintf(outf, "    %%%d = zext i1 %%%d to i32\n", zext_reg, cmp_reg);

                free(left);
                free(right);

                char buf[32];
                snprintf(buf, sizeof(buf), "%%%d", zext_reg);
                return safe_strdup(buf);
            } else {
                int reg = (*register_count)++;
                fprintf(outf, "    %%%d = %s i32 %s, %s\n", reg, op_str, left, right);

                free(left);
                free(right);

                char buf[32];
                snprintf(buf, sizeof(buf), "%%%d", reg);
                return safe_strdup(buf);
            }
        }

        case NODE_SCHO: {
            char* val = compile_value_node(outf, node->data.scho_stmt.value, register_count);
            llvm_scho(outf, val);
            free(val);
            return NULL;
        }

        case NODE_CALL: {
            const char* rt = NULL;
            if (node->data.fun_call.returnType[0]) rt = node->data.fun_call.returnType;
            if (!rt) rt = lookup_emitted_function_return(node->data.fun_call.name);
            if (!rt) rt = "void";

            char args_buf[512] = "";
            if (node->data.fun_call.arguments && node->data.fun_call.arg_count > 0) {
                for (int i = 0; i < node->data.fun_call.arg_count; i++) {
                    ASTNode* arg = node->data.fun_call.arguments[i];
                    char* arg_val = compile_value_node(outf, arg, register_count);
                    if (i > 0) {
                        strncat(args_buf, ", ", sizeof(args_buf) - strlen(args_buf) - 1);
                    }
                    strncat(args_buf, "i32 ", sizeof(args_buf) - strlen(args_buf) - 1);
                    strncat(args_buf, arg_val, sizeof(args_buf) - strlen(args_buf) - 1);
                    free(arg_val);
                }
            }

            if (strcmp(rt, "void") == 0) {
                if (args_buf[0] != '\0') {
                    fprintf(outf, "    call void @%s(%s)\n", node->data.fun_call.name, args_buf);
                } else {
                    fprintf(outf, "    call void @%s()\n", node->data.fun_call.name);
                }
                return NULL;
            }

            int reg = (*register_count)++;
            if (args_buf[0] != '\0') {
                fprintf(outf, "    %%%d = call %s @%s(%s)\n", reg, rt, node->data.fun_call.name, args_buf);
            } else {
                fprintf(outf, "    %%%d = call %s @%s()\n", reg, rt, node->data.fun_call.name);
            }
            char buf[32];
            snprintf(buf, sizeof(buf), "%%%d", reg);
            return safe_strdup(buf);
        }

        case NODE_RETURN: {
            current_function_has_return = 1;
            if (node->data.return_stmt.value) {
                char* val = compile_value_node(outf, node->data.return_stmt.value, register_count);
                if (strcmp(current_function_return_type, "void") == 0) {
                    /* function declared void but return has value: still emit ret i32 by default */
                    fprintf(outf, "    ret i32 %s\n", val);
                } else if (strcmp(current_function_return_type, "float") == 0) {
                    fprintf(outf, "    ret float %s\n", val);
                } else {
                    fprintf(outf, "    ret %s %s\n", current_function_return_type, val);
                }
                free(val);
            } else {
                fprintf(outf, "    ret void\n");
            }
            return NULL;
        }
        
        case NODE_IF: {
            int my_id = label_counter++;

            char* cond = compile_value_node(outf, node->data.if_stmt.condition, register_count);
            int cmp_reg = (*register_count)++;
            fprintf(outf, "    %%%d = icmp ne i32 %s, 0\n", cmp_reg, cond);
            free(cond);

            int then_label = my_id * 3 + 1;
            int else_label = my_id * 3 + 2;
            int end_label = my_id * 3 + 3;

            fprintf(outf, "    br i1 %%%d, label %%then%d, label %%else%d\n", cmp_reg, then_label, else_label);

            fprintf(outf, "then%d:\n", then_label);
            int then_returned = 0;
            for (int i = 0; i < node->data.if_stmt.then_count; i++) {
                if (node->data.if_stmt.then_statements[i]->type == NODE_RETURN) then_returned = 1;
                
                char* leftover = compile_node(outf, node->data.if_stmt.then_statements[i], register_count);
                if (leftover) free(leftover);
            }
            if (!then_returned) {
                fprintf(outf, "    br label %%end%d\n", end_label);
            }

            fprintf(outf, "else%d:\n", else_label);
            int else_returned = 0;
            if (node->data.if_stmt.else_node) {
                if (node->data.if_stmt.else_node->type == NODE_IF) {
                    char* leftover = compile_node(outf, node->data.if_stmt.else_node, register_count);
                    if (leftover) free(leftover);
                } else if (node->data.if_stmt.else_node->type == NODE_PROGRAM) {
                    for (int i = 0; i < node->data.if_stmt.else_node->data.program.count; i++) {
                        if (node->data.if_stmt.else_node->data.program.statements[i]->type == NODE_RETURN) else_returned = 1;
                        
                        char* leftover = compile_node(outf, node->data.if_stmt.else_node->data.program.statements[i], register_count);
                        if (leftover) free(leftover);
                    }
                }
            }
            if (!else_returned) {
                fprintf(outf, "    br label %%end%d\n", end_label);
            }

            // end
            fprintf(outf, "end%d:\n", end_label);
            return NULL;
        }

        case NODE_PROGRAM:
            return NULL;

        case NODE_REPEAT: {
            for (int r = 0; r < node->data.repeat_stmt.repeat_count; r++) {
                for (int s = 0; s < node->data.repeat_stmt.count; s++) {
                    char* leftover = compile_node(outf, node->data.repeat_stmt.statements[s], register_count);
                    if (leftover) free(leftover);
                }
            }
            return NULL;
        }
    }
    
    return NULL;
}

int to_llvm_ir(const Token* tokens, int token_count, ARGS_CONTEX* ctx) {
    FILE *outf = fopen("output.ll", "w");
    if (outf == NULL) {
        printf("Error: Could not open or create output.ll file.\n");
        return 1; 
    }
    
    ASTNode* ast_root = parse(tokens, token_count);

    if (!ast_root || ast_root->type != NODE_PROGRAM) {
        fprintf(stderr, "Error: Invalid AST Root.\n");
        fclose(outf);
        return 1;
    }

    fprintf(outf, "; ModuleID = 'output.ll'\n");

    fprintf(outf, "declare i32 @putchar(i32)\n\n");

    for (int i = 0; i < ast_root->data.program.count; i++) {
        emit_globals_for_statement(ast_root->data.program.statements[i], outf);
    }

    fprintf(outf, "\n");
    fprintf(outf, "define void @cprint(i32 %%charasc) {\n");
    fprintf(outf, "entry:\n");
    fprintf(outf, "    %%0 = call i32 @putchar(i32 %%charasc)\n");
    fprintf(outf, "    ret void\n");
    fprintf(outf, "}\n\n");

    // Emit global variables (including those declared inside if/elseif/else/repeat)
    for (int i = 0; i < ast_root->data.program.count; i++) {
        emit_globals_for_statement(ast_root->data.program.statements[i], outf);
    }

    fprintf(outf, "\n");

    for (int i = 0; i < ast_root->data.program.count; i++) {
        ASTNode* stmt = ast_root->data.program.statements[i];
        if (stmt->type == NODE_FUN_DEF) {
            emit_function_definition(outf, stmt);
        }
    }

    fprintf(outf, "define i32 @main() {\n");
    fprintf(outf, "entry:\n");

    int register_count = 1;
    for (int i = 0; i < ast_root->data.program.count; i++) {
        ASTNode* stmt = ast_root->data.program.statements[i];
        if (stmt->type != NODE_FUN_DEF) {
            char* leftover = compile_node(outf, stmt, &register_count);
            if (leftover) free(leftover);
        }
    }

    fprintf(outf, "    ret i32 0\n");
    fprintf(outf, "}\n");

    fclose(outf);
    printf("Compiled Successfully\n");

    if (!hasArg(ctx, "-wE")) {
        #ifdef _WIN32
            system(".\\llvm\\llvm.exe");
        #else
            system("./llvm/llvm.exe");
        #endif
        exit(0);
    }
    return 0;
}