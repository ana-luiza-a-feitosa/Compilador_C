#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_RESERVED 8
#define MAX_TOKEN_LEN 256
#define MAX_CHILDREN 3

typedef enum {
    // Palavras reservadas
    IF, ELSE, WHILE, RETURN, INT, VOID, INPUT, OUTPUT,
    // Tokens especiais
    ASSIGN, EQ, LT, GT, LE, GE, NE, PLUS, MINUS, TIMES, OVER,
    LPAREN, RPAREN, LBRACKET, RBRACKET, LBRACE, RBRACE,
    SEMI, COMMA,
    // Outros
    ID, NUM, ERROR, ENDFILE
} TokenType;

typedef enum { StmtK, ExpK, DeclK } NodeKind;
typedef enum { IfK, WhileK, ReturnK, CompoundK, CallK } StmtKind;
typedef enum { OpK, ConstK, IdK, AssignK, ArrIdK } ExpKind;
typedef enum { VarK, FunK, ParamK, ArrVarK, ArrParamK } DeclKind;
typedef enum { Void, Integer, Array, Function } ExpType;

typedef struct treeNode {
    struct treeNode *child[MAX_CHILDREN];
    struct treeNode *sibling;
    int lineno;
    NodeKind nodekind;
    union { StmtKind stmt; ExpKind exp; DeclKind decl; } kind;
    union {
        TokenType op;
        int val;
        char *name;
    } attr;
    ExpType type;
    int arraySize;
} TreeNode;

extern FILE *source;
extern FILE *listing;
extern int lineno;
extern char tokenString[MAX_TOKEN_LEN + 1];
extern TreeNode *syntaxTree;
extern int Error;

/* Symbol table structures */
typedef struct LineListRec {
    int lineno;
    struct LineListRec *next;
} *LineList;

typedef struct BucketListRec {
    char *name;
    LineList lines;
    int memloc;
    ExpType type;
    int arraySize;
    char *scope;
    struct BucketListRec *next;
} *BucketList;

/* Function prototypes */
TokenType getToken(void);
TreeNode *parse(void);
void buildSymtab(TreeNode *);
void typeCheck(TreeNode *);
void printSymTab(FILE *);
void printTree(TreeNode *);
void codeGen(TreeNode *, char *);

BucketList st_lookup(char *);
BucketList st_lookup_current(char *);
void st_insert(char *, int, int, ExpType, int);
void st_push_scope(char *);
void st_pop_scope(void);
void st_set_scope(char *);
char *st_get_scope(void);

#endif