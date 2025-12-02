#include "globals.h"
#include "scanner.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s arquivo.c-\n", argv[0]);
        return EXIT_FAILURE;
    }

    abrirFonte(argv[1]);

    /* 1) Parser: constrói AST */
    TreeNode *ast = parse();

    if (Error) {
        fprintf(stderr, "Compilacao encerrada devido a erros.\n");
        return EXIT_FAILURE;
    }

    /* 2) Analisador semântico */
    semantic_check(ast);

    if (Error) {
        fprintf(stderr, "Compilacao encerrada devido a erros semanticos.\n");
        return EXIT_FAILURE;
    }

    /* 3) Geração de código intermediário (quando implementarem) */
    codegen(ast, "code3addr.txt");

    printf("Compilacao finalizada.\n");
    return EXIT_SUCCESS;
}
