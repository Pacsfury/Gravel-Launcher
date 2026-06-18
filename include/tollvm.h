#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "tokens.h"
#include "ast.h"

void to_llvm_ir(Token* tokens[], int token_count);