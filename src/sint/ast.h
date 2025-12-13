#ifndef AST_H
#define AST_H

#include <stdio.h>

typedef enum {
    ND_STMT,
    ND_EXPR,
    ND_DECL
} NodeKind;

typedef enum {
    ST_COMPOUND,
    ST_IF,
    ST_WHILE,
    ST_RETURN,
    ST_EXPR
} StmtKind;

typedef enum {
    EX_OP,
    EX_CONST,
    EX_ID,
    EX_CALL,
    EX_ASSIGN,
    EX_INDEX
} ExprKind;

typedef enum {
    DECL_VAR,
    DECL_FUN,
    DECL_PARAM
} DeclKind;

typedef enum {
    TY_INT,
    TY_VOID
} TypeSpec;

/* TokenType vem do scanner.h */
#include "scanner.h"

typedef struct treeNode {
    struct treeNode *child[3];
    struct treeNode *sibling;

    int lineno;
    NodeKind nodekind;

    union {
        StmtKind stmt;
        ExprKind expr;
        DeclKind decl;
    } kind;

    /* atributos comuns */
    TokenType op;     /* EX_OP */
    int val;          /* EX_CONST */
    char *name;       /* ID / CALL / DECL */
    TypeSpec type;    /* DECL_* */
    int arraySize;    /* DECL_VAR (>=0 se array), DECL_PARAM (0 se array param), senão -1 */
} TreeNode;

/* construtores */
TreeNode *newStmtNode(StmtKind kind);
TreeNode *newExprNode(ExprKind kind);
TreeNode *newDeclNode(DeclKind kind);

#endif
