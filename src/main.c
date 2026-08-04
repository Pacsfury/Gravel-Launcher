#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/launcher.h"
#include "../include/argc.h"
#include "../include/tokens.h"
#include "../include/ast.h"
#include "../include/tollvm.h"

int main(int argc, char *argv[]) {

    clock_t start_time = clock();

    ARGS_CONTEX ctx = { argc, argv };
    args_init(&ctx, argc, argv);

    if(hasArg(&ctx, "init")) {
        _launcherInit();
    }

    if (hasArg(&ctx, "pyll")) {
        char command[512];
        snprintf(command, sizeof(command), "python %s", getArg(&ctx, "pyll"));
        system(command);
    }


    if(hasArg(&ctx, "run")) {
        tokenizeFile(getArg(&ctx, "run"), &ctx);
    }
    
    _launcherFree();
  
    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("| %f s | %d tokens |\n", time_taken, token_count);


    system("python ./llvm/llvm.py");

    end_time = clock();
    time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("| %f s | %d tokens |\n", time_taken, token_count);
    return 0;
}