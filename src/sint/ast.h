#ifndef AST_H
#define AST_H

#include "globals.h"
#include "scanner.h"

/* Tipos de nós */
typedef enum {
    ND_STMT,
    ND_EXPR,
    ND_DECL,
    ND_FUNC
} NodeKind;

typedef enum {
    ST_IF,
    ST_WHILE,
    ST_RETURN,
    ST_COMPOUND,
    ST_ASSIGN
} StmtKind;

typedef enum {
    EX_OP,
    EX_CONST,
    EX_ID,
    EX_CALL
} ExprKind;

typedef struct treeNode {
    struct treeNode *child[3];
    struct treeNode *sibling;

    int lineno;
    NodeKind nodekind;
    union { StmtKind stmt; ExprKind expr; } kind;

    /* infos extras */
    TokenType op;  /* para EX_OP */
    int       val; /* para EX_CONST */
    char     *name;/* para IDs e funções */
} TreeNode;

/* Funções utilitárias de criação */
TreeNode *newStmtNode(StmtKind kind);
TreeNode *newExprNode(ExprKind kind);
TreeNode *newDeclNode(void);

/* Impressão simples da AST (texto) */
void printAST(TreeNode *t, int indent);

#endif /* AST_H */
