#include "../include/lib.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../include/tokens.h"
#include "../include/launcher.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #include <process.h>
#else
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #include <direct.h>
    #define CREATE_FOLDER(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #define CREATE_FOLDER(path) mkdir(path, 0777)
#endif

// This memory has to be manually free'd btw. If it won't be free'd it will create a error when compiled and ran with sanitizers!!!!!
char*
get_file_contents(const char* file_name)
{
    FILE* file = fopen(file_name, "r");

    if (!file)
        return NULL;

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return NULL;
    }

    size_t file_size = ftell(file);

    if (!file_size)
    {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        return NULL;
    }

    char* result = malloc((size_t)file_size + 1);

    if (!result)
    {
        fclose(file);
        raiseError("Out of memory allocation for file buffer", "E0000");
        return NULL;
    }

    size_t bytes_read = fread(result, 1, (size_t)file_size, file);

    if (bytes_read != (size_t)file_size || ferror(file))
    {
        free(result);
        fclose(file);
        return NULL;
    }

    result[bytes_read] = '\0';
    fclose(file);

    return result;
}

static intptr_t download_package(const char* url, const char* output_path) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    const char* args[] = {"curl", "-s", "--output", output_path, "--", url, NULL};
    return _spawnvp(_P_WAIT, "curl", args);
#else
    pid_t child = fork();
    if (child == -1)
        return -1;
    if (child == 0) {
        execlp("curl", "curl", "-s", "--output", output_path, "--", url, (char*)NULL);
        _exit(127);
    }

    int status;
    while (waitpid(child, &status, 0) == -1) {
        if (errno != EINTR)
            return -1;
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}

static void register_package_from_file(const char* file_path)
{
    char* contents = get_file_contents(file_path);

    if (!contents)
    {
        raiseError("File does not exist or cannot be read", "E0031");
        return;
    }

    char* line = contents;

    while (*line)
    {
        char* next = strpbrk(line, "\r\n");

        if (next)
            *next = '\0';

        char* p = strstr(line, "package");

        if (p)
        {
            p += 7;

            while (*p == ' ' || *p == '\t')
                p++;

            if (*p == ':')
            {
                p++;

                while (*p == ' ' || *p == '\t')
                    p++;

                char name[64] = {0};
                int i = 0;

                while (*p &&
                       (*p == '_' || *p == '.' ||
                        isalnum((unsigned char)*p)) &&
                       i < 63)
                {
                    name[i++] = *p++;
                }

                if (i > 0)
                {
                    addPackage(name, (char*)file_path);
                    free(contents);
                    return;
                }
            }
        }

        if (!next)
            break;

        line = next + 1;

        if (*line == '\n' && *next == '\r')
            line++;
    }

    free(contents);
}

static void register_package_from_url(const char* url) {
    CREATE_FOLDER(".gravel_cache");

    char temp_cache[256] = ".gravel_cache/gravel_cache_temp.tmp";
    intptr_t ret = download_package(url, temp_cache);

    if (ret != 0)
    {
        remove(temp_cache);
        return;
    }

    char* contents = get_file_contents(temp_cache);

    if (!contents)
    {
        remove(temp_cache);
        raiseError("File does not exist or cannot be read", "E0031");
        return;
    }

    char name[64] = {0};
    char* line = contents;

    while (*line)
    {
        char* next = strpbrk(line, "\r\n");

        if (next)
            *next = '\0';

        char* p = strstr(line, "package");

        if (p)
        {
            p += 7;

            while (*p == ' ' || *p == '\t')
                p++;

            if (*p == ':') {
                p++;

                while (*p == ' ' || *p == '\t')
                    p++;

                int i = 0;

                while (*p && (*p == '_' || *p == '.' || isalnum((unsigned char)*p)) && i < (int)sizeof(name) - 1)
                    name[i++] = *p++;

                name[i] = '\0';

                if (i > 0)
                    break;
            }
        }

        if (!next)
            break;

        line = next + 1;

        if (*line == '\n' && *next == '\r')
            line++;
    }

    free(contents);

    if (name[0] != '\0') {
        char final_cache_path[256];

        snprintf(final_cache_path, sizeof(final_cache_path),
                 ".gravel_cache/gravel_cache_%s.grv", name);

        remove(final_cache_path);

        if (rename(temp_cache, final_cache_path) != 0) {
            remove(temp_cache);
            return;
        }

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
    // Lets hope the file path aint that much longer
    char command[512];
    char* file = getArg(ctx, "pyll");

    if (file == NULL)
    {
        fprintf(stderr, "Usage: pyll <input file>\n");
        return;
    }

    int len = snprintf(command, sizeof(command), "python3 \"%s\"", file);

    if (len < 0 || (size_t)len >= sizeof(command))
    {
        fprintf(stderr, "Command too long.\n");
        return;
    }

    system(command);
}

void
new_handle_arg(ARGS_CONTEX* ctx)
{
    (void)ctx;
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

    char* cargo_contents = get_file_contents("Libs.grvdep");

    if (cargo_contents)
    {
        char* cargo_contents_line = strtok(cargo_contents, "\r\n");

        while (cargo_contents_line != NULL)
        {
            if (cargo_contents_line[0] != '\0') {
                if (!strncmp(cargo_contents_line, "web:", 4)) {
                    register_package_from_url(cargo_contents_line + 4);
                } else {
                    register_package_from_file(cargo_contents_line);
                }
            }

            cargo_contents_line = strtok(NULL, "\r\n");
        }

        free(cargo_contents);
    }
    else
    {
        printf("Libs.grvdep is missing, it would be highly recomended to create this file, so you can add your own libraries!\n");
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