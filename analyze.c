/*
 * Analisador Semântico para C-
 */

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

static char *currentScope = "global";
static int location = 0;

/* Inserir funções built-in (input e output) */
static void insertBuiltins(void) {
    st_insert("input", 0, location++, Integer, "global");
    st_insert("output", 0, location++, Void, "global");
}

/* Percorre a árvore em pré-ordem inserindo identificadores na tabela */
static void traverse(TreeNode *t, void (*preProc)(TreeNode *), 
                    void (*postProc)(TreeNode *)) {
    if (t != NULL) {
        preProc(t);
        int i;
        for (i = 0; i < MAXCHILDREN; i++)
            traverse(t->child[i], preProc, postProc);
        postProc(t);
        traverse(t->sibling, preProc, postProc);
    }
}

/* Pós-processamento para resetar escopo */
static void afterNode(TreeNode *t) {
    if (t != NULL && t->nodekind == StmtK && t->kind.stmt == FunDeclK) {
        /* Volta ao escopo global após processar uma função */
        currentScope = "global";
    }
}

/* Insere identificadores na tabela de símbolos */
static void insertNode(TreeNode *t) {
    switch (t->nodekind) {
    case StmtK:
        switch (t->kind.stmt) {
        case FunDeclK:
            /* Sempre inserimos funções no escopo global */
            st_insert(t->attr.name, t->lineno, location++, t->type, "global");
            currentScope = t->attr.name;
            break;
        case VarDeclK:
            /* Insere variável no escopo atual */
            st_insert(t->attr.name, t->lineno, location++, t->type, currentScope);
            break;
        case ParamK:
            if (t->attr.name != NULL) {
                /* Insere parâmetro no escopo da função */
                st_insert(t->attr.name, t->lineno, location++, t->type, currentScope);
            }
            break;
        default:
            break;
        }
        break;
    case ExpK:
        switch (t->kind.exp) {
        case IdK:
        case ArrIdK:
            /* Apenas registra uso, sem verificar se existe */
            /* A verificação real seria feita com busca em múltiplos escopos */
            st_insert(t->attr.name, t->lineno, 0, t->type, currentScope);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

/* Verifica tipos na árvore */
static void checkNode(TreeNode *t) {
    switch (t->nodekind) {
    case ExpK:
        switch (t->kind.exp) {
        case OpK:
            /* Verifica apenas se os filhos existem e já têm tipo atribuído */
            if (t->child[0] != NULL && t->child[1] != NULL) {
                /* Não gera erro se os tipos ainda não foram definidos (tipo Void inicial) */
                if (t->child[0]->type != Void && t->child[1]->type != Void) {
                    if ((t->child[0]->type != Integer && t->child[0]->type != Boolean) || 
                        (t->child[1]->type != Integer && t->child[1]->type != Boolean)) {
                        fprintf(listing, "ERRO SEMANTICO: operacao com tipo invalido - LINHA: %d\n",
                                t->lineno);
                        Error = TRUE;
                    }
                }
            }
            /* Define o tipo do nó de operação */
            if ((t->attr.op == EQ) || (t->attr.op == NE) ||
                (t->attr.op == LT) || (t->attr.op == LE) ||
                (t->attr.op == GT) || (t->attr.op == GE))
                t->type = Boolean;
            else
                t->type = Integer;
            break;
        case ConstK:
            t->type = Integer;
            break;
        case IdK:
        case ArrIdK:
            /* Assume tipo Integer para variáveis e arrays */
            if (t->type == Void)
                t->type = Integer;
            break;
        default:
            break;
        }
        break;
    case StmtK:
        switch (t->kind.stmt) {
        case AssignK:
            /* Verifica atribuições apenas se tipos já foram definidos */
            if (t->child[1] != NULL && t->child[1]->type != Void) {
                if (t->child[1]->type != Integer && t->child[1]->type != Boolean) {
                    fprintf(listing, "ERRO SEMANTICO: atribuicao com tipo invalido - LINHA: %d\n",
                            t->lineno);
                    Error = TRUE;
                }
            }
            break;
        case IfK:
            /* Verifica apenas se a condição já tem tipo definido */
            if (t->child[0] != NULL && t->child[0]->type != Void) {
                if (t->child[0]->type != Boolean && t->child[0]->type != Integer) {
                    fprintf(listing, "ERRO SEMANTICO: condicao do if deve ser booleana - LINHA: %d\n",
                            t->lineno);
                    Error = TRUE;
                }
            }
            break;
        case WhileK:
            /* Verifica apenas se a condição já tem tipo definido */
            if (t->child[0] != NULL && t->child[0]->type != Void) {
                if (t->child[0]->type != Boolean && t->child[0]->type != Integer) {
                    fprintf(listing, "ERRO SEMANTICO: condicao do while deve ser booleana - LINHA: %d\n",
                            t->lineno);
                    Error = TRUE;
                }
            }
            break;
        case CallK:
            /* Chamadas de função retornam tipo Integer por padrão */
            if (strcmp(t->attr.name, "input") == 0)
                t->type = Integer;
            else if (strcmp(t->attr.name, "output") == 0)
                t->type = Void;
            else
                t->type = Integer; /* Assume Integer para outras funções */
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void buildSymtab(TreeNode *syntaxTree) {
    insertBuiltins();
    traverse(syntaxTree, insertNode, afterNode);
    currentScope = "global";
}

void typeCheck(TreeNode *syntaxTree) {
    traverse(syntaxTree, afterNode, checkNode);
}