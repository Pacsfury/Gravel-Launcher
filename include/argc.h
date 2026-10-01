#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct args_contex {
    int argc;
    char** argv;
} ARGS_CONTEX;

typedef struct
{
    void (*function)(ARGS_CONTEX*);
    const char* name;
    const char* desc;
} ARGS_data;

bool args_init(int argc, char* argv[]);

bool hasArg(ARGS_CONTEX* ctx, char* arg);

char* getArg(ARGS_CONTEX* ArgsContex, char* argName);
