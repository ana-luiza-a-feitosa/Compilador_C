#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

/* Gera código intermediário em três endereços */
void codegen(TreeNode *syntaxTree, const char *filename);

#endif /* CODEGEN_H */
