#include "semantic.h"

/* 
 * Semântica simples:
 * - Ao ver um ST_ASSIGN, insere a variável do lado esquerdo na tabela (tipo int).
 * - Ao ver um EX_ID (uso de variável), verifica se já foi declarada.
 */

static void verificaNo(TreeNode *t) {
    if (t == NULL) return;

    switch (t->nodekind) {
    case ND_STMT:
        if (t->kind.stmt == ST_ASSIGN) {
            /* child[0] deve ser o ID do lado esquerdo */
            TreeNode *idNode = t->child[0];
            if (idNode &&
                idNode->nodekind == ND_EXPR &&
                idNode->kind.expr == EX_ID &&
                idNode->name != NULL) {

                /* tenta inserir; se já existir no mesmo escopo, symtab avisa */
                symtab_insert(idNode->name, TYPE_INT, idNode->lineno);
            }
        }
        break;

    case ND_EXPR:
        if (t->kind.expr == EX_ID) {
            if (t->name != NULL) {
                Symbol *s = symtab_lookup(t->name);
                if (s == NULL) {
                    printf("ERRO SEMANTICO: identificador \"%s\" nao declarado - LINHA: %d\n",
                           t->name, t->lineno);
                    Error = 1;
                }
            }
        }
        break;

    default:
        break;
    }
}

/* travessia pré-ordem */
static void traverse(TreeNode *t) {
    if (t == NULL) return;

    /* pré-ordem: verifica o nó atual */
    verificaNo(t);

    /* percorre filhos */
    for (int i = 0; i < 3; i++) {
        traverse(t->child[i]);
    }

    /* percorre irmãos */
    traverse(t->sibling);
}

void semantic_check(TreeNode *syntaxTree) {
    symtab_init();       /* limpa tabela de símbolos */
    traverse(syntaxTree);

    /* aqui vocês podem depois acrescentar:
     * - regras para main, input, output
     * - checagem de tipos mais rigorosa, etc.
     */
}
