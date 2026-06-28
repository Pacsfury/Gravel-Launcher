#pragma once

#include <stdio.h>
#include "tokens.h"
#include "ast.h"


int to_llvm_ir(const Token* tokens, int token_count);
