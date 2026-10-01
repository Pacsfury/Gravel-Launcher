#include <lib.h>
#include <stdlib.h>
#include <ctype.h>
#include <tokens.h>
#include <launcher.h>


#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #include <direct.h>
    #define CREATE_FOLDER(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #define CREATE_FOLDER(path) mkdir(path, 0777)
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
    CREATE_FOLDER(".gravel_cache");
    char temp_cache[256] = ".gravel_cache/gravel_cache_temp.tmp";
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
        snprintf(final_cache_path, sizeof(final_cache_path), ".gravel_cache/gravel_cache_%s.grv", name);

        remove(final_cache_path);
        rename(temp_cache, final_cache_path);

        register_package_from_file(final_cache_path);
        
    } else {
        remove(temp_cache);
    }
}

static
void
new_project(void)
{
    FILE* ignore = fopen(".gitignore", "w");
    if (ignore != NULL) {
        fprintf(ignore, ".llvm_cache/\n.gravel_cache/\n*.ll\n");
        fclose(ignore);
    }

    FILE* libs = fopen("Libs.grvdep", "w");
    if (libs != NULL) {
        fclose(libs);
    }

    CREATE_FOLDER("code");
    CREATE_FOLDER("libs");
}

void
winll_handle_arg(ARGS_CONTEX* ctx)
{
    system(getArg(ctx, "winll"));
}

void
pyll_handle_arg(ARGS_CONTEX* ctx)
{
    system(strcat("python ", getArg(ctx, "pyll")));
}

void
new_handle_arg(ARGS_CONTEX* ctx)
{
    new_project();
    exit(0);
}

void
run_handle_arg(ARGS_CONTEX* ctx)
{
    if (ctx->argc < 3)
    {
        printf("Run selected but no file!\n");
        exit(0);
    }
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
    for (int i = 1; i < ctx->argc; i++) {
        if (strcmp(ctx->argv[i], "run") != 0)
            continue;
        for (int j = i + 1; j < ctx->argc; j++) {
            if (ctx->argv[j] == NULL || strncmp(ctx->argv[j], "-", 1) == 0)
                continue;
            register_package_from_file(ctx->argv[j]);
        }
        for (int j = i + 1; j < ctx->argc; j++) {
            if (ctx->argv[j] == NULL || strncmp(ctx->argv[j], "-", 1) == 0)
                continue;
            tokenizeFile(ctx->argv[j], ctx);
        }
        break;
    }
}