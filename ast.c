#include "ast.h"

// Implementação das funções de construção de nó
TreeNode *newNode(NodeKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    if (t == NULL) {
        fprintf(stderr, "Erro de alocação de memória para nó da AST.\n");
        exit(1);
    }
    int i;
    for (i = 0; i < MAX_CHILDREN; i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = kind;
    t->lineno = -1; // Será definido pelo Parser

    t->type = UNKNOWN;
    t->name = NULL;
    t->val = 0;
    t->op = NULL;
    t->temp_loc = -1;

    return t;
}

TreeNode *newDeclNode(DeclKind kind, ExpType type, char *name) {
    TreeNode *t = newNode(DeclK);
    if (t != NULL) {
        t->kind.decl = kind;
        t->type = type;
        if (name != NULL) t->name = strdup(name);
    }
    return t;
}

TreeNode *newStmtNode(StmtKind kind) {
    TreeNode *t = newNode(StmtK);
    if (t != NULL) t->kind.stmt = kind;
    return t;
}

TreeNode *newExpNode(ExpKind kind, ExpType type) {
    TreeNode *t = newNode(ExpK);
    if (t != NULL) {
        t->kind.exp = kind;
        t->type = type;
    }
    return t;
}

// Função auxiliar para imprimir espaçamento
static void printSpaces(int level) {
    for (int i = 0; i < level; i++) {
        fprintf(stdout, "  ");
    }
}

// Função de impressão da AST (simples, em formato textual) [cite: 50]
void printAST(TreeNode *tree, int level) {
    if (tree == NULL) return;

    printSpaces(level);

    // Imprime o tipo de nó principal
    switch (tree->nodekind) {
        case DeclK: fprintf(stdout, "DECLARACAO: "); break;
        case StmtK: fprintf(stdout, "COMANDO: "); break;
        case ExpK:  fprintf(stdout, "EXPRESSAO: "); break;
        case ParamK:fprintf(stdout, "PARAMETRO: "); break;
    }

    // Imprime o subtipo de nó
    switch (tree->nodekind) {
        case DeclK: 
            switch (tree->kind.decl) {
                case VarDeclK: fprintf(stdout, "VAR (id: %s, tipo: %s)\n", tree->name, (tree->type == INTEGER ? "int" : "void")); break;
                case FuncDeclK: fprintf(stdout, "FUNCAO (id: %s, tipo: %s)\n", tree->name, (tree->type == INTEGER ? "int" : "void")); break;
                case ArrayDeclK: fprintf(stdout, "ARRAY (id: %s, tipo: int)\n", tree->name); break;
            }
            break;
        case StmtK:
            switch (tree->kind.stmt) {
                case IfK: fprintf(stdout, "IF\n"); break;
                case WhileK: fprintf(stdout, "WHILE\n"); break;
                case AssignK: fprintf(stdout, "ATRIBUICAO\n"); break;
                case RetK: fprintf(stdout, "RETURN\n"); break;
                case CompK: fprintf(stdout, "BLOCO\n"); break;
                case CallK: fprintf(stdout, "CHAMADA FUNCAO (id: %s)\n", tree->name); break;
                case InputK: fprintf(stdout, "INPUT\n"); break;
                case OutputK: fprintf(stdout, "OUTPUT\n"); break;
                case NullK: fprintf(stdout, "NULO\n"); break;
            }
            break;
        case ExpK:
            switch (tree->kind.exp) {
                case OpK: fprintf(stdout, "OP (%s)\n", tree->op); break;
                case ConstK: fprintf(stdout, "CONST (%d)\n", tree->val); break;
                case IdK: fprintf(stdout, "ID (%s)\n", tree->name); break;
                case ArrayK: fprintf(stdout, "ARRAY INDEX (%s)\n", tree->name); break;
            }
            break;
        case ParamK: 
            fprintf(stdout, "PARAMETRO (id: %s, tipo: %s)\n", tree->name, (tree->type == INTEGER ? "int" : "void"));
            break;
    }

    // Recursão nos filhos
    for (int i = 0; i < MAX_CHILDREN; i++) {
        printAST(tree->child[i], level + 1);
    }
    // Recursão no irmão (sibling)
    printAST(tree->sibling, level);
}