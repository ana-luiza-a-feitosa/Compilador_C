/*
 * Analisador Semântico para C-
 */

#include "globals.h"
#include "symtab.h"
#include "util.h"
#include "analyze.h"

static char *currentScope = "global";
static int location = 0;
static int hasMain = 0;  /* Flag para verificar se main existe */

/* Lista temporária para rastrear variáveis declaradas */
typedef struct SymbolRec {
    char *name;
    char *scope;
    ExpType type;
    struct SymbolRec *next;
} SymbolRec;

static SymbolRec *symbolList = NULL;

/* Adiciona símbolo à lista */
static void addSymbol(char *name, char *scope, ExpType type) {
    SymbolRec *rec = (SymbolRec *)malloc(sizeof(SymbolRec));
    rec->name = copyString(name);
    rec->scope = copyString(scope);
    rec->type = type;
    rec->next = symbolList;
    symbolList = rec;
}

/* Verifica se símbolo foi declarado */
static int symbolExists(char *name, char *scope) {
    SymbolRec *rec = symbolList;
    while (rec != NULL) {
        /* Procura no escopo atual ou global */
        if (strcmp(rec->name, name) == 0) {
            if (strcmp(rec->scope, scope) == 0 || strcmp(rec->scope, "global") == 0) {
                return 1;
            }
        }
        rec = rec->next;
    }
    return 0;
}

/* Inserir funções built-in (input e output) */
static void insertBuiltins(void) {
    st_insert("input", 0, location++, Integer, "global");
    st_insert("output", 0, location++, Void, "global");
    addSymbol("input", "global", Integer);
    addSymbol("output", "global", Void);
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
            /* Verifica se é a função main */
            if (strcmp(t->attr.name, "main") == 0) {
                hasMain = 1;
                
                /* Verifica se main é void (tipo de retorno) */
                if (t->type != Void) {
                    fprintf(listing, "ERRO SEMANTICO: funcao 'main' deve retornar 'void', nao '%s' - LINHA: %d\n",
                            t->type == Integer ? "int" : "outro tipo",
                            t->lineno);
                    Error = TRUE;
                }
                /* Nota: Verificação de parâmetros já é feita no parser */
            }
            
            /* Sempre inserimos funções no escopo global */
            st_insert(t->attr.name, t->lineno, location++, t->type, "global");
            addSymbol(t->attr.name, "global", t->type);
            currentScope = t->attr.name;
            break;
        case VarDeclK:
            /* Insere variável no escopo atual */
            st_insert(t->attr.name, t->lineno, location++, t->type, currentScope);
            addSymbol(t->attr.name, currentScope, t->type);
            break;
        case ParamK:
            if (t->attr.name != NULL) {
                /* Insere parâmetro no escopo da função */
                st_insert(t->attr.name, t->lineno, location++, t->type, currentScope);
                addSymbol(t->attr.name, currentScope, t->type);
            }
            break;
        case AssignK:
            /* Verifica se a variável foi declarada */
            if (!symbolExists(t->attr.name, currentScope)) {
                fprintf(listing, "ERRO SEMANTICO: identificador '%s' nao declarado - LINHA: %d\n",
                        t->attr.name, t->lineno);
                Error = TRUE;
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
            /* Verifica se a variável foi declarada */
            if (!symbolExists(t->attr.name, currentScope)) {
                fprintf(listing, "ERRO SEMANTICO: identificador '%s' nao declarado - LINHA: %d\n",
                        t->attr.name, t->lineno);
                Error = TRUE;
            } else {
                /* Apenas registra uso */
                st_insert(t->attr.name, t->lineno, 0, t->type, currentScope);
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
            /* Verifica se a função foi declarada */
            if (!symbolExists(t->attr.name, "global")) {
                fprintf(listing, "ERRO SEMANTICO: funcao '%s' nao declarada - LINHA: %d\n",
                        t->attr.name, t->lineno);
                Error = TRUE;
            }
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
    symbolList = NULL; /* Reset lista de símbolos */
    hasMain = 0;       /* Reset flag de main */
    insertBuiltins();
    traverse(syntaxTree, insertNode, afterNode);
    currentScope = "global";
    
    /* VERIFICA SE MAIN EXISTE */
    if (!hasMain) {
        fprintf(listing, "\nERRO SEMANTICO: programa deve conter uma funcao 'void main(void)'\n");
        Error = TRUE;
    }
}

void typeCheck(TreeNode *syntaxTree) {
    traverse(syntaxTree, afterNode, checkNode);
}