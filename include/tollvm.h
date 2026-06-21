#pragma once
#include <stdio.h>
#include "tokens.h"

void llvm_scho(FILE* outf, int sch, int* scho_init);
void llvm_create_var(FILE* outf, const char* name, const char* val, const char* type);
int to_llvm_ir(const Token* tokens, int token_count);