#ifndef _LIB_H_
#define _LIB_H_

#pragma once

#include "../include/argc.h"

char*
get_file_contents(const char* file_name);

void
winll_handle_arg(ARGS_CONTEX* ctx);

void
pyll_handle_arg(ARGS_CONTEX* ctx);

void
new_handle_arg(ARGS_CONTEX* ctx);

void
run_handle_arg(ARGS_CONTEX* ctx);

#endif