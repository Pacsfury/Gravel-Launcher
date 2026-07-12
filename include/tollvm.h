#pragma once

#include <stdio.h>
#include "tokens.h"
#include "ast.h"
#include "argc.h"

void llvm_scho(FILE* outf, const char* val_to_print);

int to_llvm_ir(const Token* tokens, int token_count, ARGS_CONTEX* ctx);
