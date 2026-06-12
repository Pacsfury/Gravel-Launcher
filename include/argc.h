#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct args_contex {
	int argc;
	char **argv;
}ARGS_CONTEX;

bool args_init(ARGS_CONTEX* ctx, int argc, char* argv[]);

bool hasArg(ARGS_CONTEX *ctx, char* arg);

char* getArg(ARGS_CONTEX* ArgsContex, char* argName);
