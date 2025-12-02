#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtab.h"

/* Percorre a AST, preenche tabela de símbolos e realiza checagens básicas */
void semantic_check(TreeNode *syntaxTree);

#endif /* SEMANTIC_H */
