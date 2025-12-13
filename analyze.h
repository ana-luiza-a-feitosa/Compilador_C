/*
 * Interface do Analisador Semântico para C-
 */

#ifndef ANALYZE_H
#define ANALYZE_H

/* Função para construir a tabela de símbolos */
void buildSymtab(TreeNode *syntaxTree);

/* Função para verificação de tipos */
void typeCheck(TreeNode *syntaxTree);

#endif