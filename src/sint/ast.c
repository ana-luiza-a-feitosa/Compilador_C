#include "ast.h"

static void printSpaces(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

TreeNode *newStmtNode(StmtKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    if (t == NULL) {
        fprintf(stderr, "Falta memoria em newStmtNode\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 3; i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ND_STMT;
    t->kind.stmt = kind;
    t->lineno = lineno;
    t->op = TK_ERROR;
    t->val = 0;
    t->name = NULL;
    return t;
}

TreeNode *newExprNode(ExprKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    if (t == NULL) {
        fprintf(stderr, "Falta memoria em newExprNode\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 3; i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ND_EXPR;
    t->kind.expr = kind;
    t->lineno = lineno;
    t->op = TK_ERROR;
    t->val = 0;
    t->name = NULL;
    return t;
}

TreeNode *newDeclNode(void) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    if (t == NULL) {
        fprintf(stderr, "Falta memoria em newDeclNode\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 3; i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ND_DECL;
    t->lineno = lineno;
    t->op = TK_ERROR;
    t->val = 0;
    t->name = NULL;
    return t;
}

/* Impressão bem simples (vocês podem aprimorar) */
void printAST(TreeNode *t, int indent) {
    while (t != NULL) {
        printSpaces(indent);
        switch (t->nodekind) {
        case ND_STMT:
            printf("Stmt node (kind=%d) - linha %d\n", t->kind.stmt, t->lineno);
            break;
        case ND_EXPR:
            printf("Expr node (kind=%d) - linha %d\n", t->kind.expr, t->lineno);
            break;
        case ND_DECL:
            printf("Decl node - linha %d\n", t->lineno);
            break;
        case ND_FUNC:
            printf("Func node - linha %d\n", t->lineno);
            break;
        default:
            printf("Node desconhecido\n");
        }

        for (int i = 0; i < 3; i++) {
            if (t->child[i] != NULL) {
                printAST(t->child[i], indent + 1);
            }
        }

        t = t->sibling;
    }
}
