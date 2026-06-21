#include <stdio.h>
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

    if(hasArg(&ctx, "end")) {
        _launcherFree();
    }
    Token* token;
    to_llvm_ir(token, 1);
    _launcherFree();
    return 0;
}

