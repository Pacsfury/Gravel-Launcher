#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/argc.h"
#include "../include/ast.h"
#include "../include/launcher.h"
#include "../include/tokens.h"
#include "../include/tollvm.h"

static void register_package_from_file(const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (!file)
        return;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* p = strstr(line, "package");
        if (!p)
            continue;

        p += 7;
        while (*p == ' ' || *p == '\t') p++;
        if (*p != ':')
            continue;
        p++;
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

        char name[64] = {0};
        int i = 0;
        while (*p && (*p == '_' || *p == '.' || isalnum((unsigned char)*p)) && i < 63) {
            name[i++] = *p++;
        }
        if (i > 0) {
            addPackage(name, (char*)file_path);
            break;
        }
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    clock_t start_time = clock();

    _launcherInit();

    ARGS_CONTEX ctx;
    args_init(&ctx, argc, argv);

    if (hasArg(&ctx, "winll")) {
        system(getArg(&ctx, "winll"));
    }

    if (hasArg(&ctx, "pyll")) {
        system(strcat("python ", getArg(&ctx, "pyll")));
    }

    if (hasArg(&ctx, "run")) {
        for (int i = 1; i < ctx.argc; i++) {
            if (strcmp(ctx.argv[i], "run") != 0)
                continue;

            for (int j = i + 1; j < ctx.argc; j++) {
                if (ctx.argv[j] == NULL || strncmp(ctx.argv[j], "-", 1) == 0)
                    continue;
                register_package_from_file(ctx.argv[j]);
            }

            for (int j = i + 1; j < ctx.argc; j++) {
                if (ctx.argv[j] == NULL || strncmp(ctx.argv[j], "-", 1) == 0)
                    continue;
                tokenizeFile(ctx.argv[j], &ctx);
            }
            break;
        }
    }

    _launcherFree();

    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("| %f s | %d tokens | COMPILE\n", time_taken, token_count);

    system("python ./llvm/llvm.py");

    end_time = clock();
    time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("| %f s | %d tokens | TOTAL\n", time_taken, token_count);
    return 0;
}
