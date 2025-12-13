#include "ast.h"
#include <stdlib.h>
#include <string.h>

static TreeNode *newNode(void) {
    TreeNode *t = (TreeNode*)calloc(1, sizeof(TreeNode));
    for (int i = 0; i < 3; i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->lineno = 0;
    t->op = TK_ERROR;
    t->val = 0;
    t->name = NULL;
    t->type = TY_INT;
    t->arraySize = -1;
    return t;
}

TreeNode *newStmtNode(StmtKind kind) {
    TreeNode *t = newNode();
    t->nodekind = ND_STMT;
    t->kind.stmt = kind;
    return t;
}

TreeNode *newExprNode(ExprKind kind) {
    TreeNode *t = newNode();
    t->nodekind = ND_EXPR;
    t->kind.expr = kind;
    return t;
}

TreeNode *newDeclNode(DeclKind kind) {
    TreeNode *t = newNode();
    t->nodekind = ND_DECL;
    t->kind.decl = kind;
    return t;
}
