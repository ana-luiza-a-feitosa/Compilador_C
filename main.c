#include "globals.h"

FILE *source;
FILE *listing;
int lineno = 0;
TreeNode *syntaxTree;
int Error = 0;

TreeNode *parse(void);
void buildSymtab(TreeNode *);
void typeCheck(TreeNode *);
void printSymTab(FILE *);
void printTree(TreeNode *);
void codeGen(TreeNode *, char *);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo.cm>\n", argv[0]);
        exit(1);
    }
    
    char sourceFile[100];
    strcpy(sourceFile, argv[1]);
    
    source = fopen(sourceFile, "r");
    if (source == NULL) {
        fprintf(stderr, "Erro: arquivo '%s' nao encontrado\n", sourceFile);
        exit(1);
    }
    
    listing = stdout;
    
    fprintf(listing, "\n========================================\n");
    fprintf(listing, "   COMPILADOR C-MINUS\n");
    fprintf(listing, "========================================\n");
    fprintf(listing, "Arquivo: %s\n", sourceFile);
    fprintf(listing, "========================================\n\n");
    
    fprintf(listing, ">>> Fase 1: ANALISE LEXICA e SINTATICA\n");
    syntaxTree = parse();
    
    if (Error) {
        fprintf(listing, "\nCompilacao ABORTADA devido a erros.\n");
        fclose(source);
        return 1;
    }
    
    fprintf(listing, "\n>>> Fase 2: CONSTRUCAO DA AST\n");
    fprintf(listing, "\n=== ARVORE SINTATICA ABSTRATA (AST) ===\n");
    printTree(syntaxTree);
    
    fprintf(listing, "\n>>> Fase 3: ANALISE SEMANTICA\n");
    buildSymtab(syntaxTree);
    
    if (Error) {
        fprintf(listing, "\nCompilacao ABORTADA devido a erros semanticos.\n");
        fclose(source);
        return 1;
    }
    
    typeCheck(syntaxTree);
    
    if (Error) {
        fprintf(listing, "\nCompilacao ABORTADA devido a erros de tipo.\n");
        fclose(source);
        return 1;
    }
    
    printSymTab(listing);
    
    fprintf(listing, "\n>>> Fase 4: GERACAO DE CODIGO INTERMEDIARIO\n");
    
    char codeFile[100];
    strcpy(codeFile, sourceFile);
    char *dot = strrchr(codeFile, '.');
    if (dot) *dot = '\0';
    strcat(codeFile, ".tm");
    
    codeGen(syntaxTree, codeFile);
    
    fprintf(listing, "\nCodigo intermediario gerado em: %s\n", codeFile);
    
    FILE *codeOut = fopen(codeFile, "r");
    if (codeOut) {
        fprintf(listing, "\n");
        char line[256];
        while (fgets(line, sizeof(line), codeOut)) {
            fprintf(listing, "%s", line);
        }
        fclose(codeOut);
    }
    
    fprintf(listing, "\n========================================\n");
    fprintf(listing, "   COMPILACAO CONCLUIDA COM SUCESSO!\n");
    fprintf(listing, "========================================\n");
    
    fclose(source);
    return 0;
}