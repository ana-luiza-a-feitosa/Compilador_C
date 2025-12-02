#include "codegen.h"

static int tempCount = 0;

static char *newTemp(void) {
    char buf[16];
    sprintf(buf, "t%d", tempCount++);
    return strdup(buf);
}

/* Gera código para expressão e devolve o "nome" onde o resultado ficou */
static char *genExpr(TreeNode *t, FILE *out) {
    if (t == NULL) return NULL;

    switch (t->kind.expr) {
    case EX_CONST: {
        char *tmp = newTemp();
        fprintf(out, "%s = %d\n", tmp, t->val);
        return tmp;
    }
    case EX_ID: {
        /* apenas retorna o nome da variável (não gera código) */
        if (t->name)
            return strdup(t->name);
        else
            return NULL;
    }
    case EX_OP: {
        /* nó binário: child[0] op child[1] */
        char *left  = genExpr(t->child[0], out);
        char *right = genExpr(t->child[1], out);
        char *tmp   = newTemp();

        char opChar = '?';
        switch (t->op) {
        case TK_PLUS:  opChar = '+'; break;
        case TK_MINUS: opChar = '-'; break;
        case TK_TIMES: opChar = '*'; break;
        case TK_OVER:  opChar = '/'; break;
        default:       opChar = '?'; break;
        }

        if (left && right) {
            fprintf(out, "%s = %s %c %s\n", tmp, left, opChar, right);
        }

        if (left)  free(left);
        if (right) free(right);

        return tmp;
    }
    default:
        /* outros tipos de expressão (EX_CALL, etc.) ainda não tratados */
        return NULL;
    }
}

/* Gera código para comandos */
static void genStmt(TreeNode *t, FILE *out) {
    while (t != NULL) {
        if (t->nodekind == ND_STMT) {
            switch (t->kind.stmt) {
            case ST_ASSIGN: {
                /* child[0] = ID, child[1] = expr */
                TreeNode *idNode  = t->child[0];
                TreeNode *exprNode = t->child[1];

                if (idNode && idNode->kind.expr == EX_ID && idNode->name != NULL) {
                    char *rhs = genExpr(exprNode, out);
                    if (rhs) {
                        fprintf(out, "%s = %s\n", idNode->name, rhs);
                        free(rhs);
                    }
                }
                break;
            }
            case ST_RETURN:
                /* ainda não implementado */
                /* Exemplo futuro: 
                 *   temp = genExpr(child[0], out);
                 *   fprintf(out, "return %s\n", temp);
                 */
                break;
            default:
                /* outros tipos de stmt (if, while, etc.) podem ser adicionados depois */
                break;
            }

            /* gerar código para filhos que sejam statements (por exemplo, blocos) */
            for (int i = 0; i < 3; i++) {
                TreeNode *c = t->child[i];
                if (c && c->nodekind == ND_STMT) {
                    genStmt(c, out);
                }
            }
        }

        /* próximo comando no mesmo nível */
        t = t->sibling;
    }
}

void codegen(TreeNode *syntaxTree, const char *filename) {
    if (syntaxTree == NULL) return;

    FILE *out = fopen(filename, "w");
    if (!out) {
        fprintf(stderr, "Nao foi possivel criar %s\n", filename);
        return;
    }

    genStmt(syntaxTree, out);

    fclose(out);
}
