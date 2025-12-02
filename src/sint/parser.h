#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "scanner.h"

/* Ponto de entrada do parser: devolve a AST da unidade de compilação */
TreeNode *parse(void);

#endif /* PARSER_H */
