#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/lib.h"

#include "../include/argc.h"

void
help_handle_arg(ARGS_CONTEX* ctx);

ARGS_data
args_list[] =
{
    { help_handle_arg, "-h", "Shows all commands and their descriptions."},
    { winll_handle_arg, "winll", "Executes a file <exec> winll <input file>"},
    { pyll_handle_arg, "pyll", "Runs a given python script. <exec> pyll <input file>"},
    { new_handle_arg, "new", "Creates a new github project."},
    { run_handle_arg, "run", "Runs a file. Example usage: <exec> run <input file>"},
};

void
help_handle_arg(ARGS_CONTEX* ctx)
{
    printf("All commands:\n");

    for (int i = 0; i < sizeof(args_list) / sizeof(args_list[0]); i++)
    {
        printf("    Command: %s, description: %s\n", args_list[i].name, args_list[i].desc ? args_list[i].desc : "None");
    }

    exit(0);
}

bool args_init(int argc, char *argv[])
{
    if (argv == NULL || argc < 0)
        return false;

    ARGS_CONTEX* ctx = malloc(sizeof(ARGS_CONTEX));
    if (ctx == NULL)
        return false;

    ctx->argc = argc;
    ctx->argv = argv;

    for (int i = 0; i < argc; i++)
    {
        for (size_t a = 0; a < sizeof(args_list) / sizeof(args_list[0]); a++)
        {
            if (strcmp(argv[i], args_list[a].name) == 0)
            {
                if (args_list[a].function != NULL)
                    args_list[a].function(ctx);

                return true;
            }
        }
    }

    return false;
}

bool hasArg(ARGS_CONTEX* ctx, char* argName) {
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
            return ctx->argv[i + 1];
        }
    }
    return NULL;
}