#include <stdio.h>
#include <stdlib.h>
#include "../include/tokens.h"
#include "../include/ast.h"
#include "../include/tollvm.h"

void llvm_create_var(FILE* outf, char name[], char val[], char type[]) {

    char res[] = "";
    // "@var12 = global i32 12, align 4"
    snprintf(res, sizeof(res), "@%s= global %s %s %s, align 4", name, type, val);
    fprintf(outf, res);
}

int to_llvm_ir(Token* tokens[], int token_count) {
    FILE *outf;

    outf = fopen("output.ll", "w");

    if (outf == NULL) {
        printf("Error: Could not open or create the file.\n");
        return 1; 
    }
    ASTNode* ast_root = parse(tokens, token_count);

    fprintf(outf, "Hello");
    llvm_create_var(&outf, "NAME", "32", "i32");

    fclose(outf);
    return 0;

}