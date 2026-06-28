#include <stdio.h>
#include <string.h>
#include "../include/launcher.h"
#include "../include/argc.h"
#include "../include/tokens.h"
#include "../include/ast.h"
#include "../include/tollvm.h"

int main(int argc, char *argv[]) {

    ARGS_CONTEX ctx = { argc, argv };
    args_init(&ctx, argc, argv);

    if(hasArg(&ctx, "init")) {
        _launcherInit();
    }

    Token tokens[7];

    tokens[0].type = TOKEN_VAR_DEF; strcpy(tokens[0].value, "val");
    tokens[1].type = TOKEN_NAME;    strcpy(tokens[1].value, "x");
    tokens[2].type = TOKEN_ASSIGN;  strcpy(tokens[2].value, "=");
    tokens[3].type = TOKEN_INT;     strcpy(tokens[3].value, "65");

    tokens[4].type = TOKEN_SCHO;    strcpy(tokens[4].value, "scho");
    tokens[5].type = TOKEN_NAME;    strcpy(tokens[5].value, "x");
    
    tokens[6].type = TOKEN_EOF; 

    to_llvm_ir(tokens, 7);
    
    printf("Done");

    if(hasArg(&ctx, "end")) {
        _launcherFree();
    } else {

        _launcherFree();
    }
    
    return 0;
}