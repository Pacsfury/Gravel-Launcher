#include <stdio.h>
#include <stdlib.h>
#include "../include/tokens.h"
#include "../include/ast.h"
#include "../include/tollvm.h"

int to_llvm_ir(Token* tokens[], int token_count) {
    FILE *outf;

    outf = fopen("output.ll", "w");

    if (outf == NULL) {
        printf("Error: Could not open or create the file.\n");
        return 1; 
    }
    ASTNode* ast_root = parse(tokens, token_count);

    fprintf(outf, "Hello");

    fclose(outf);
    return 0;

}