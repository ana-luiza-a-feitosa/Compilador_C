#ifndef UTIL_H
#define UTIL_H

#include "globals.h"  // ← DEVE TER ESTA LINHA!

void printToken(TokenType, const char *);
TreeNode *newStmtNode(StmtKind);
TreeNode *newExpNode(ExpKind);
char *copyString(char *);
void printTree(TreeNode *);

#endif