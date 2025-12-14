/*
 * Interface das funções utilitárias
 */

#ifndef UTIL_H
#define UTIL_H

void printToken(TokenType, const char *);
TreeNode *newStmtNode(StmtKind);
TreeNode *newExpNode(ExpKind);
char *copyString(char *);
void printTree(TreeNode *);
void printTreeDot(TreeNode *, const char *);

#endif