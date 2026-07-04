#include "../include/argc.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool args_init(ARGS_CONTEX *ctx, int argc, char* argv[]) {
    if (ctx == NULL || argv == NULL || argc < 0)
        return false;
    ctx->argc = argc; // (*ctx).argc
    ctx->argv = argv; // (*ctx).argv
    return true;

}

bool hasArg(ARGS_CONTEX *ctx, char* argName) {
    if (ctx == NULL || argName == NULL)
        return false;

    for (int i = 0; i < ctx->argc; i++) {
        if (strcmp(ctx->argv[i], argName) == 0) {
            return true;
        }
    }
    return false;
}

char* getArg(ARGS_CONTEX* ctx, char* argName) {
    if (ctx == NULL || argName == NULL)
        return NULL;


    for (int i = 0; i < ctx->argc - 1; i++) {
        if (strcmp(ctx->argv[i], argName) == 0) {
            return ctx->argv[i+1];
        }
    }
    return NULL;
}