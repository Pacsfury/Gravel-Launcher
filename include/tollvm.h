#pragma once

#include <stdio.h>

#include "argc.h"
#include "ast.h"
#include "tokens.h"

void llvm_scho(FILE* outf, const char* val_to_print);

int to_llvm_ir(const Token* tokens, int token_count, ARGS_CONTEX* ctx);

static char* compile_node(FILE* outf, ASTNode* node, int* register_count);

static const char* llvm_type_for(const char* type_name);