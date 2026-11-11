#pragma once

#include "../include/tokens.h"

bool checkGrammar(Token* tokens, int len);

typedef struct
{
    int end_indent;
    int if_indent;
    bool on_if;
} grammar_checker_t;