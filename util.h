#ifndef UTIL_H
#define UTIL_H

#include "globals.h"

void printToken(TokenType, const char *);
TreeNode *newStmtNode(StmtKind);
TreeNode *newExpNode(ExpKind);
char *copyString(char *);
void printTree(TreeNode *);
void printTreeDot(TreeNode *, const char *, const char *);

#endif