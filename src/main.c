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

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

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

static void register_package_from_url(const char* url) {
    char temp_cache[256] = "gravel_cache_temp.tmp";
    char command[512];
    
    snprintf(command, sizeof(command), "curl -s \"%s\" -o %s", url, temp_cache);
    int ret = system(command);
    if (ret != 0) {
        remove(temp_cache);
        return;
    }

    FILE* file = fopen(temp_cache, "r");
    if (!file) {
        remove(temp_cache);
        return;
    }

    char name[64] = {0};
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

        int i = 0;
        while (*p && (*p == '_' || *p == '.' || isalnum((unsigned char)*p)) && i < 63) {
            name[i++] = *p++;
        }
        if (i > 0) {
            break;
        }
    }
    fclose(file);

    if (name[0] != '\0') {
        char final_cache_path[256];
        snprintf(final_cache_path, sizeof(final_cache_path), "gravel_cache_%s.grv", name);

        remove(final_cache_path);
        rename(temp_cache, final_cache_path);

        register_package_from_file(final_cache_path);
        
    } else {
        remove(temp_cache);
    }
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
        FILE* cargo = fopen("Libs.grvdep", "r");
        if (cargo != NULL) {
            char buffer[256];

            while (fgets(buffer, sizeof(buffer), cargo) != NULL) {
                buffer[strcspn(buffer, "\r\n")] = '\0';

                if (buffer[0] == '\0')
                    continue;

                if (!strncmp(buffer, "web:", 4)) {
                    register_package_from_url(buffer + 4);
                } else {
                    register_package_from_file(buffer);
                }
            }
            fclose(cargo);
        }

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
