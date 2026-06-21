#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "tokens.h"
#include "ast.h"

int to_llvm_ir(const Token* tokens, int token_count);

void llvm_create_var(FILE* outf, const char* name[], const char* val[], const char* type[]);

void llvm_init_cout(FILE* outf);