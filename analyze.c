#include "globals.h"

static int location = 0;
static char *currentFunctionName = NULL;

static void semanticError(TreeNode *t, char *message) {
    fprintf(listing, "ERRO SEMANTICO: %s - LINHA: %d\n", message, t->lineno);
    Error = 1;
}

static void traverse(TreeNode *t,
                    void (*preProc)(TreeNode *),
                    void (*postProc)(TreeNode *)) {
    if (t != NULL) {
        preProc(t);
        for (int i = 0; i < MAX_CHILDREN; i++)
            traverse(t->child[i], preProc, postProc);
        postProc(t);
        traverse(t->sibling, preProc, postProc);
    }
}

static void nullProc(TreeNode *t) {
    if (t == NULL) return;
}

static void insertNode(TreeNode *t) {
    BucketList l;
    char msg[100];
    
    switch (t->nodekind) {
    case DeclK:
        switch (t->kind.decl) {
        case FunK:
            if (st_lookup_current(t->attr.name) != NULL) {
                sprintf(msg, "funcao '%s' ja declarada", t->attr.name);
                semanticError(t, msg);
            } else {
                st_insert(t->attr.name, t->lineno, location++, Function, 0);
                st_push_scope(t->attr.name);
                currentFunctionName = t->attr.name;
            }
            break;
        case VarK:
        case ArrVarK:
            if (st_lookup_current(t->attr.name) != NULL) {
                sprintf(msg, "variavel '%s' ja declarada", t->attr.name);
                semanticError(t, msg);
            } else {
                st_insert(t->attr.name, t->lineno, location++, t->type, t->arraySize);
            }
            break;
        case ParamK:
        case ArrParamK:
            if (strcmp(t->attr.name, "") != 0) {
                if (st_lookup_current(t->attr.name) != NULL) {
                    sprintf(msg, "parametro '%s' ja declarado", t->attr.name);
                    semanticError(t, msg);
                } else {
                    st_insert(t->attr.name, t->lineno, location++, t->type, 0);
                }
            }
            break;
        }
        break;
    case ExpK:
        switch (t->kind.exp) {
        case IdK:
        case ArrIdK:
            l = st_lookup(t->attr.name);
            if (l == NULL) {
                sprintf(msg, "identificador '%s' nao declarado", t->attr.name);
                semanticError(t, msg);
            } else {
                t->type = l->type;
            }
            break;
        default:
            break;
        }
        break;
    case StmtK:
        switch (t->kind.stmt) {
        case CallK:
            l = st_lookup(t->attr.name);
            if (l == NULL) {
                sprintf(msg, "funcao '%s' nao declarada", t->attr.name);
                semanticError(t, msg);
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

static void afterNode(TreeNode *t) {
    if (t == NULL) return;
    
    if (t->nodekind == DeclK && t->kind.decl == FunK) {
        st_pop_scope();
        currentFunctionName = NULL;
    }
}

static void checkNode(TreeNode *t) {
    BucketList l;
    
    switch (t->nodekind) {
    case ExpK:
        switch (t->kind.exp) {
        case OpK:
            if (t->child[0] && t->child[0]->type == Array) {
                semanticError(t, "operacao invalida com array");
            }
            if (t->child[1] && t->child[1]->type == Array) {
                semanticError(t, "operacao invalida com array");
            }
            t->type = Integer;
            break;
        case ConstK:
            t->type = Integer;
            break;
        case AssignK:
            if (t->child[0] && t->child[0]->type == Array) {
                if (t->child[0]->kind.exp != ArrIdK) {
                    semanticError(t, "atribuicao invalida a array");
                }
            }
            if (t->child[1] && t->child[1]->type == Array) {
                semanticError(t, "atribuicao invalida de array");
            }
            break;
        default:
            break;
        }
        break;
    case StmtK:
        switch (t->kind.stmt) {
        case ReturnK:
            if (currentFunctionName != NULL) {
                BucketList func = st_lookup(currentFunctionName);
                if (func && func->type == Void && t->child[0] != NULL) {
                    semanticError(t, "funcao void nao pode retornar valor");
                }
                if (func && func->type != Void && t->child[0] == NULL) {
                    semanticError(t, "funcao deve retornar valor");
                }
            }
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
    st_set_scope("global");
    
    st_insert("input", 0, location++, Integer, 0);
    st_insert("output", 0, location++, Void, 0);
    
    traverse(syntaxTree, insertNode, afterNode);
    
    BucketList mainFunc = st_lookup("main");
    if (mainFunc == NULL) {
        fprintf(listing, "ERRO SEMANTICO: funcao 'main' nao declarada\n");
        Error = 1;
    } else if (mainFunc->type != Integer && mainFunc->type != Void) {
        fprintf(listing, "ERRO SEMANTICO: 'main' deve retornar int ou void\n");
        Error = 1;
    }
}

void typeCheck(TreeNode *syntaxTree) {
    traverse(syntaxTree, checkNode, nullProc);
}