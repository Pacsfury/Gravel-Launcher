#include <stdio.h>
#include <stdlib.h>
#include "../include/tokens.h"
#include "../include/ast.h"
#include "../include/tollvm.h"

void llvm_scho(FILE* outf, int sch, int* scho_init) {
    if (*scho_init == 0) {
        fprintf(outf,
            "declare i32 @putchar(i32)\n\n"
            "define void @cprint(i32 %%charasc) {\n"
            "entry:\n"
            "    %%0 = call i32 @putchar(i32 %%charasc)\n"
            "    ret void\n"
            "}\n\n"
        );
        *scho_init = 1;
    }

    fprintf(outf, "    call void @cprint(i32 %d)\n", sch);
}

void llvm_create_var(FILE* outf, const char* name, const char* val, const char* type) {
    fprintf(outf, "@%s = global %s %s, align 4\n", name, type, val);
}

int to_llvm_ir(const Token* tokens, int token_count) {
    int scho_init = 0;
    FILE *outf;

    outf = fopen("output.ll", "w");

    if (outf == NULL) {
        printf("Error: Could not open or create the file.\n");
        return 1; 
    }
    
    ASTNode* ast_root = parse(tokens, token_count);

    fprintf(outf, "; ModuleID = 'output.ll'\n\n");
    
    llvm_create_var(outf, "NAME", "32", "i32");
    fprintf(outf, "\n");

    fprintf(outf, "define i32 @main() {\nentry:\n");
    
    llvm_scho(outf, 65, &scho_init);

    fprintf(outf, "    ret i32 0\n}\n");

    fclose(outf);
    return 0;
}