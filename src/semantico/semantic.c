#include "semantic.h"
#include "symtab.h"
#include "globals.h"
#include <stdio.h>

static void traverse(TreeNode *t) {
  if (!t) return;

  /* exemplo mínimo: só caminha na árvore */
  for (int i = 0; i < 3; i++) traverse(t->child[i]);
  traverse(t->sibling);
}

void semantic_check(TreeNode *syntaxTree) {
  symtab_init();
  traverse(syntaxTree);

  /* Aqui depois você implementa:
     - input/output predefinidos
     - main por último
     - declaradas antes do uso
     - checagem de tipos/escopo
  */
}