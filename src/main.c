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

int main(int argc, char* argv[]) {
    clock_t start_time = clock();

    _launcherInit();

    if (!args_init(argc, argv))
    {
        printf("No arguments given or a wrong argument! Use -h for help!\n");
        exit(0);
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
