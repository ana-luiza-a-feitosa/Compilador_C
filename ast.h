#ifndef _AST_H_
#define _AST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definições de Kind e Type conforme especificado 
typedef enum { 
    StmtK, ExpK, DeclK, ParamK 
} NodeKind;

typedef enum { 
    IfK, WhileK, AssignK, RetK, CompK, CallK, InputK, OutputK, NullK 
} StmtKind;

typedef enum { 
    OpK, ConstK, IdK, ArrayK
} ExpKind;

typedef enum { 
    VarDeclK, FuncDeclK, ArrayDeclK 
} DeclKind;

typedef enum { 
    ParamDeclK 
} ParamKind;

// Tipos de dados (C- só usa int e void, mais array e function)
typedef enum { 
    VOID, INTEGER, ARRAY, FUNCTION, UNKNOWN 
} ExpType; 

#define MAX_CHILDREN 3

// Estrutura do nó da AST
typedef struct treeNode {
    struct treeNode *child[MAX_CHILDREN];
    struct treeNode *sibling; // Para listas de declarações, parâmetros, etc.
    int lineno;

    NodeKind nodekind;
    union { StmtKind stmt; ExpKind exp; DeclKind decl; ParamKind param; } kind;

    // Atributos específicos
    ExpType type;       // Tipo determinado pela análise semântica [cite: 38]
    char *name;         // Para identificadores e nomes de funções
    int val;            // Para constantes
    char *op;           // Para operadores (ex: "+", "==")

    // ... Campos para a Geração de Código (ex: endereço, registrador)
    int temp_loc;       // Localização temporária para código de 3 endereços
} TreeNode;

// Funções de construção de nó
TreeNode *newNode(NodeKind kind);
TreeNode *newDeclNode(DeclKind kind, ExpType type, char *name);
TreeNode *newStmtNode(StmtKind kind);
TreeNode *newExpNode(ExpKind kind, ExpType type);

// Funções de impressão da AST (para o entregável) [cite: 50]
void printAST(TreeNode *tree, int level);

#endif