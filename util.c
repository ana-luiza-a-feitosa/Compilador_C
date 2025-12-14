/*
 * Funções utilitárias para o compilador C-
 */

#include "globals.h"
#include "util.h"

void printToken(TokenType token, const char *tokenString) {
    switch (token) {
    case IF:
    case ELSE:
    case INT:
    case RETURN:
    case VOID:
    case WHILE:
        fprintf(listing, "palavra-chave: %s\n", tokenString);
        break;
    case ASSIGN: fprintf(listing, "=\n"); break;
    case EQ: fprintf(listing, "==\n"); break;
    case NE: fprintf(listing, "!=\n"); break;
    case LT: fprintf(listing, "<\n"); break;
    case LE: fprintf(listing, "<=\n"); break;
    case GT: fprintf(listing, ">\n"); break;
    case GE: fprintf(listing, ">=\n"); break;
    case PLUS: fprintf(listing, "+\n"); break;
    case MINUS: fprintf(listing, "-\n"); break;
    case TIMES: fprintf(listing, "*\n"); break;
    case OVER: fprintf(listing, "/\n"); break;
    case LPAREN: fprintf(listing, "(\n"); break;
    case RPAREN: fprintf(listing, ")\n"); break;
    case LBRACKET: fprintf(listing, "[\n"); break;
    case RBRACKET: fprintf(listing, "]\n"); break;
    case LBRACE: fprintf(listing, "{\n"); break;
    case RBRACE: fprintf(listing, "}\n"); break;
    case SEMI: fprintf(listing, ";\n"); break;
    case COMMA: fprintf(listing, ",\n"); break;
    case ENDFILE: fprintf(listing, "EOF\n"); break;
    case NUM:
        fprintf(listing, "NUM: %s\n", tokenString);
        break;
    case ID:
        fprintf(listing, "ID: %s\n", tokenString);
        break;
    case ERROR:
        fprintf(listing, "ERRO LEXICO: '%s' - LINHA: %d\n", tokenString, lineno);
        break;
    default:
        fprintf(listing, "Token desconhecido: %d\n", token);
    }
}

TreeNode *newStmtNode(StmtKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Erro: sem memoria\n");
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = StmtK;
        t->kind.stmt = kind;
        t->lineno = lineno;
        t->type = Void;
        t->arraySize = 0;
    }
    return t;
}

TreeNode *newExpNode(ExpKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Erro: sem memoria\n");
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = ExpK;
        t->kind.exp = kind;
        t->lineno = lineno;
        t->type = Void;
        t->arraySize = 0;
    }
    return t;
}

char *copyString(char *s) {
    int n;
    char *t;
    if (s == NULL) return NULL;
    n = strlen(s) + 1;
    t = malloc(n);
    if (t == NULL)
        fprintf(listing, "Erro: sem memoria\n");
    else
        strcpy(t, s);
    return t;
}

static int indentno = 0;
static int nodeCounter = 0;

#define INDENT indentno += 2
#define UNINDENT indentno -= 2

static void printSpaces(void) {
    int i;
    for (i = 0; i < indentno; i++)
        fprintf(listing, " ");
}

/* Gera arquivo Graphviz (.dot) */
static FILE *dotFile = NULL;

static void printDotNode(TreeNode *tree, int parentId, int childNum) {
    if (tree == NULL) return;
    
    int myId = nodeCounter++;
    char label[100];
    
    if (tree->nodekind == StmtK) {
        switch (tree->kind.stmt) {
        case IfK: strcpy(label, "IF"); break;
        case WhileK: strcpy(label, "WHILE"); break;
        case AssignK: strcpy(label, ":="); break;
        case ReturnK: strcpy(label, "RETURN"); break;
        case FunDeclK: sprintf(label, "FUNC\\n%s", tree->attr.name); break;
        case VarDeclK: sprintf(label, "VAR\\n%s", tree->attr.name); break;
        case ParamK: sprintf(label, "PARAM\\n%s", tree->attr.name); break;
        case CallK: sprintf(label, "CALL\\n%s", tree->attr.name); break;
        case CompoundK: strcpy(label, "BLOCK"); break;
        default: strcpy(label, "?"); break;
        }
    } else if (tree->nodekind == ExpK) {
        switch (tree->kind.exp) {
        case OpK:
            switch(tree->attr.op) {
                case PLUS: strcpy(label, "+"); break;
                case MINUS: strcpy(label, "-"); break;
                case TIMES: strcpy(label, "*"); break;
                case OVER: strcpy(label, "/"); break;
                case LT: strcpy(label, "<"); break;
                case LE: strcpy(label, "<="); break;
                case GT: strcpy(label, ">"); break;
                case GE: strcpy(label, ">="); break;
                case EQ: strcpy(label, "=="); break;
                case NE: strcpy(label, "!="); break;
                default: strcpy(label, "?"); break;
            }
            break;
        case ConstK: sprintf(label, "%d", tree->attr.val); break;
        case IdK: sprintf(label, "%s", tree->attr.name); break;
        case ArrIdK: sprintf(label, "%s[]", tree->attr.name); break;
        default: strcpy(label, "?"); break;
        }
    }
    
    fprintf(dotFile, "  node%d [label=\"%s\"];\n", myId, label);
    
    if (parentId >= 0) {
        fprintf(dotFile, "  node%d -> node%d;\n", parentId, myId);
    }
    
    int i;
    for (i = 0; i < MAXCHILDREN; i++) {
        if (tree->child[i] != NULL) {
            printDotNode(tree->child[i], myId, i);
        }
    }
    
    if (tree->sibling != NULL) {
        printDotNode(tree->sibling, parentId, childNum + 1);
    }
}

/* Função para gerar arquivo .dot */
void printTreeDot(TreeNode *tree, const char *filename) {
    dotFile = fopen(filename, "w");
    if (dotFile == NULL) {
        fprintf(listing, "Erro ao criar arquivo .dot\n");
        return;
    }
    
    fprintf(dotFile, "digraph AST {\n");
    fprintf(dotFile, "  node [shape=circle, style=filled, fillcolor=lightblue];\n");
    fprintf(dotFile, "  edge [color=black];\n");
    
    nodeCounter = 0;
    printDotNode(tree, -1, 0);
    
    fprintf(dotFile, "}\n");
    fclose(dotFile);
    
    fprintf(listing, "\nArquivo Graphviz gerado: %s\n", filename);
    fprintf(listing, "Para visualizar, execute: dot -Tpng %s -o ast.png\n", filename);
}

/* Impressão textual simplificada */
void printTree(TreeNode *tree) {
    int i;
    INDENT;
    while (tree != NULL) {
        printSpaces();
        if (tree->nodekind == StmtK) {
            switch (tree->kind.stmt) {
            case IfK:
                fprintf(listing, "If\n");
                break;
            case WhileK:
                fprintf(listing, "While\n");
                break;
            case AssignK:
                fprintf(listing, "Assign: %s\n", tree->attr.name);
                break;
            case ReturnK:
                fprintf(listing, "Return\n");
                break;
            case FunDeclK:
                fprintf(listing, "Function: %s (tipo: %s)\n", 
                        tree->attr.name,
                        tree->type == Integer ? "int" : "void");
                break;
            case VarDeclK:
                if (tree->type == IntegerArray)
                    fprintf(listing, "Var: %s[%d] (int)\n", 
                            tree->attr.name, tree->arraySize);
                else
                    fprintf(listing, "Var: %s (int)\n", tree->attr.name);
                break;
            case ParamK:
                if (tree->type == IntegerArray)
                    fprintf(listing, "Param: %s[] (int)\n", tree->attr.name);
                else
                    fprintf(listing, "Param: %s (%s)\n", 
                            tree->attr.name,
                            tree->type == Integer ? "int" : "void");
                break;
            case CallK:
                fprintf(listing, "Call: %s\n", tree->attr.name);
                break;
            case CompoundK:
                fprintf(listing, "Compound Statement\n");
                break;
            default:
                fprintf(listing, "Stmt desconhecido\n");
                break;
            }
        } else if (tree->nodekind == ExpK) {
            switch (tree->kind.exp) {
            case OpK:
                fprintf(listing, "Op: ");
                printToken(tree->attr.op, "\0");
                break;
            case ConstK:
                fprintf(listing, "Const: %d\n", tree->attr.val);
                break;
            case IdK:
                fprintf(listing, "Id: %s\n", tree->attr.name);
                break;
            case ArrIdK:
                fprintf(listing, "Array: %s\n", tree->attr.name);
                break;
            default:
                fprintf(listing, "Exp desconhecida\n");
                break;
            }
        } else
            fprintf(listing, "No desconhecido\n");
        for (i = 0; i < MAXCHILDREN; i++)
            printTree(tree->child[i]);
        tree = tree->sibling;
    }
    UNINDENT;
}