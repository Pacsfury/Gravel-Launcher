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

    Token mock_tokens[] = { // Example tokens
        { TOKEN_INT, "int" },
        { TOKEN_NAME, "x" },
        { TOKEN_VAR_INFER, "=" },
        { TOKEN_INT, "5" },
        { TOKEN_ADD, "+" },
        { TOKEN_INT, "3" },
        { TOKEN_STAR, "*" },
        { TOKEN_INT, "2" },
        { TOKEN_EOF, "" }
    };

    to_llvm_ir(mock_tokens, 9);

    _launcherFree();
    return 0;
}

// int main() { //DEBUG FUNCTION
//     // Simulamos los tokens de la instrucción: val x := 5 + 3 * 2
//     Token mock_tokens[] = { // Example tokens
//         { TOKEN_INT, "int" },
//         { TOKEN_NAME, "x" },
//         { TOKEN_VAR_DEF, "=" },
//         { TOKEN_INT, "5" },
//         { TOKEN_ADD, "+" },
//         { TOKEN_INT, "3" },
//         { TOKEN_STAR, "*" },
//         { TOKEN_INT, "2" },
//         { TOKEN_EOF, "" }
//     };
//     int token_count = 9;

//     // DEBUG: Yep, spanish
//     printf("--- Iniciando el Parser ---\n");
    
//     // Ejecutamos tu función principal
//     ASTNode* ast_root = parse(mock_tokens, token_count);

//     printf("--- Árbol de Sintaxis Abstracta (AST) generado ---\n");
//     // Imprimimos el resultado empezando con profundidad 0
//     print_ast(ast_root, 0);

//     // TODO: Aquí deberías liberar la memoria con free() más adelante
//     return 0;
// }

