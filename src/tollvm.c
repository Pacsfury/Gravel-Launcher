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
    if (stmt->type == NODE_DECLARATION || stmt->type == NODE_CONSTANT) {
        if (!already_emitted(stmt->data.var_decl.name)) {
            fprintf(outf, "@%s = global i32 0, align 4\n", stmt->data.var_decl.name);
            mark_emitted(stmt->data.var_decl.name);
        }
    } else if (stmt->type == NODE_REPEAT) {
        for (int i = 0; i < stmt->data.repeat_stmt.count; i++) {
            emit_globals_for_statement(stmt->data.repeat_stmt.statements[i], outf);
        }
    }
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


void llvm_scho(FILE* outf, const char* val_to_print) {
    fprintf(outf, "    call void @cprint(i32 %s)\n", val_to_print);
}


static char* compile_node(FILE* outf, ASTNode* node, int* register_count) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_LITERAL: {
            const char* v = node->data.literal.value;
            if (strlen(v) == 1 && !isdigit(v[0])) {
                char buf[8];
                snprintf(buf, sizeof(buf), "%d", (int)v[0]);
                return safe_strdup(buf);
            }
            return safe_strdup(v);
        }
        case NODE_CONSTANT:
        case NODE_DECLARATION: {
            char* val = compile_node(outf, node->data.var_decl.value, register_count);
            fprintf(outf, "    store i32 %s, ptr @%s, align 4\n", val, node->data.var_decl.name);
            free(val);

            return NULL; 
        }



        case NODE_BINARY_OP: {
            char* left = compile_node(outf, node->data.binary_op.left, register_count);
            char* right = compile_node(outf, node->data.binary_op.right, register_count);
            
            const char* op_str = "";
            switch(node->data.binary_op.op) {
                case TOKEN_ADD:  op_str = "add"; break;
                case TOKEN_SUB:  op_str = "sub"; break;
                case TOKEN_STAR: op_str = "mul"; break;
                case TOKEN_DIV:  op_str = "sdiv"; break;
                default:         op_str = "add"; break;
            }

            int reg = (*register_count)++;
            fprintf(outf, "    %%%d = %s i32 %s, %s\n", reg, op_str, left, right);
            
            free(left);
            free(right);

            char buf[32];
            snprintf(buf, sizeof(buf), "%%%d", reg);
            return safe_strdup(buf);
        }

        case NODE_SCHO: {
            char* val = compile_node(outf, node->data.scho_stmt.value, register_count);
            
            llvm_scho(outf, val);
            
            free(val);
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
    fprintf(outf, "target datalayout = \"e-m:e-i64:64-f80:128-n8:16:32:64-S128\"\n\n");

    fprintf(outf, "declare i32 @putchar(i32)\n\n");
    fprintf(outf, "define void @cprint(i32 %%charasc) {\n");
    fprintf(outf, "entry:\n");
    fprintf(outf, "    %%0 = call i32 @putchar(i32 %%charasc)\n");
    fprintf(outf, "    ret void\n");
    fprintf(outf, "}\n\n");

    for (int i = 0; i < ast_root->data.program.count; i++) {
        ASTNode* stmt = ast_root->data.program.statements[i];
        if (stmt->type == NODE_DECLARATION || stmt->type == NODE_CONSTANT) {
            fprintf(outf, "@%s = global i32 0, align 4\n", stmt->data.var_decl.name);
        } else if (stmt->type == NODE_REPEAT) {
            for (int j = 0; j < stmt->data.repeat_stmt.count; j++) {
                emit_globals_for_statement(stmt->data.repeat_stmt.statements[j], outf);
            }
        }
    }

    fprintf(outf, "\n");

    fprintf(outf, "define i32 @main() {\n");
    fprintf(outf, "entry:\n");
    
    int register_count = 1;

    for (int i = 0; i < ast_root->data.program.count; i++) {
        ASTNode* stmt = ast_root->data.program.statements[i];
        char* leftover = compile_node(outf, stmt, &register_count);
        if (leftover) free(leftover); 
    }

    fprintf(outf, "    ret i32 0\n");
    fprintf(outf, "}\n");

    fclose(outf);
    printf("Compiled Succesfully\n");
    if (!hasArg(ctx, "-wE")) {
        exit(0);
    }
    return 0;
}
