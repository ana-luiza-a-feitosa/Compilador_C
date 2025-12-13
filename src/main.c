#include "globals.h"
#include "scanner.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void dumpFile(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return;
    int c;
    puts("===== out/out.c =====");
    while ((c = fgetc(f)) != EOF) putchar(c);
    puts("\n===== end out/out.c =====");
    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.c->\n", argv[0]);
        return 1;
    }

    /* zera flag global de erro */
    Error = 0;

    if (!openSource(argv[1])) {
        fprintf(stderr, "Nao foi possivel abrir %s\n", argv[1]);
        return 1;
    }

    TreeNode *ast = parse();
    closeSource();

    if (Error || !ast) {
        fprintf(stderr, "Compilacao falhou.\n");
        return 1;
    }

    semantic_check(ast);
    if (Error) {
        fprintf(stderr, "Compilacao falhou (semantico).\n");
        return 1;
    }

#ifdef _WIN32
    system("if not exist out mkdir out");
#else
    system("mkdir -p out");
#endif

    /* gera código C em out/out.c (o codegen já coloca #include "../runtime/runtime.h") */
    codegen(ast, "out/out.c");
    dumpFile("out/out.c");

#ifdef _WIN32
    /* COMPILA + LINKA com o runtime */
    if (system("gcc out\\out.c runtime\\runtime.c -o out\\a.exe") != 0) {
        fprintf(stderr, "Falha ao compilar/linkar out/out.c + runtime\n");
        return 1;
    }
    puts("===== executando out/a.exe =====");
    return system("out\\a.exe");
#else
    if (system("gcc out/out.c runtime/runtime.c -o out/a.out") != 0) {
        fprintf(stderr, "Falha ao compilar/linkar out/out.c + runtime\n");
        return 1;
    }
    puts("===== executando out/a.out =====");
    return system("./out/a.out");
#endif
}
