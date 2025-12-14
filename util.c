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

/* Função auxiliar para obter a cor do nó baseado no tipo */
static const char* getNodeColor(TreeNode *tree) {
    if (tree->nodekind == StmtK) {
        switch (tree->kind.stmt) {
        case FunDeclK: return "lightgreen";
        case IfK:
        case WhileK: return "lightyellow";
        case AssignK: return "lightcoral";
        case ReturnK: return "pink";
        case VarDeclK:
        case ParamK: return "lightgray";
        case CallK: return "plum";
        case CompoundK: return "wheat";
        default: return "lightblue";
        }
    } else if (tree->nodekind == ExpK) {
        if (tree->kind.exp == OpK) return "orange";
        if (tree->kind.exp == ConstK) return "white";
        return "lightcyan";
    }
    return "lightblue";
}

/* Função auxiliar para obter a forma do nó */
static const char* getNodeShape(TreeNode *tree) {
    if (tree->nodekind == StmtK) {
        switch (tree->kind.stmt) {
        case FunDeclK:
        case VarDeclK:
        case ParamK: return "box";
        case IfK:
        case WhileK: return "diamond";
        default: return "ellipse";
        }
    } else if (tree->nodekind == ExpK) {
        if (tree->kind.exp == OpK) return "circle";
        if (tree->kind.exp == ConstK) return "box";
        return "ellipse";
    }
    return "ellipse";
}

/* Função auxiliar para escapar caracteres especiais em labels */
static void escapeLabel(char *dest, const char *src, int maxLen) {
    int i = 0, j = 0;
    while (src[i] != '\0' && j < maxLen - 1) {
        /* Escapa caracteres especiais do Graphviz */
        if (src[i] == '"') {
            dest[j++] = '\\';
            dest[j++] = '"';
        } else if (src[i] == '\\') {
            dest[j++] = '\\';
            dest[j++] = '\\';
        } else if (src[i] == '\n') {
            dest[j++] = '\\';
            dest[j++] = 'n';
        } else {
            dest[j++] = src[i];
        }
        i++;
    }
    dest[j] = '\0';
}

/* Função auxiliar para obter label do nó */
static void getNodeLabel(TreeNode *tree, char *label, int maxLen) {
    char temp[100];
    
    if (tree == NULL) {
        snprintf(label, maxLen, "NULL");
        return;
    }
    
    if (tree->nodekind == StmtK) {
        switch (tree->kind.stmt) {
        case IfK: 
            snprintf(label, maxLen, "if"); 
            break;
        case WhileK: 
            snprintf(label, maxLen, "while"); 
            break;
        case AssignK: 
            snprintf(label, maxLen, "="); 
            break;
        case ReturnK: 
            snprintf(label, maxLen, "return"); 
            break;
        case FunDeclK:
            if (tree->attr.name != NULL) {
                escapeLabel(temp, tree->attr.name, sizeof(temp));
                snprintf(label, maxLen, "%s : %s", temp,
                        tree->type == Integer ? "int" : "void");
            } else {
                snprintf(label, maxLen, "func : %s", 
                        tree->type == Integer ? "int" : "void");
            }
            break;
        case VarDeclK:
            if (tree->attr.name != NULL) {
                escapeLabel(temp, tree->attr.name, sizeof(temp));
                if (tree->type == IntegerArray)
                    snprintf(label, maxLen, "int %s[%d]", temp, tree->arraySize);
                else
                    snprintf(label, maxLen, "int %s", temp);
            } else {
                snprintf(label, maxLen, "int var");
            }
            break;
        case ParamK:
            if (tree->attr.name != NULL) {
                escapeLabel(temp, tree->attr.name, sizeof(temp));
                if (tree->type == IntegerArray)
                    snprintf(label, maxLen, "%s[]", temp);
                else
                    snprintf(label, maxLen, "%s", temp);
            } else {
                snprintf(label, maxLen, "param");
            }
            break;
        case CallK:
            if (tree->attr.name != NULL) {
                escapeLabel(temp, tree->attr.name, sizeof(temp));
                snprintf(label, maxLen, "%s", temp);
            } else {
                snprintf(label, maxLen, "call");
            }
            break;
        case CompoundK: 
            snprintf(label, maxLen, "compound"); 
            break;
        default: 
            snprintf(label, maxLen, "?stmt?"); 
            break;
        }
    } else if (tree->nodekind == ExpK) {
        switch (tree->kind.exp) {
        case OpK:
            switch(tree->attr.op) {
                case PLUS: snprintf(label, maxLen, "+"); break;
                case MINUS: snprintf(label, maxLen, "-"); break;
                case TIMES: snprintf(label, maxLen, "*"); break;
                case OVER: snprintf(label, maxLen, "/"); break;
                case LT: snprintf(label, maxLen, "<"); break;
                case LE: snprintf(label, maxLen, "<="); break;
                case GT: snprintf(label, maxLen, ">"); break;
                case GE: snprintf(label, maxLen, ">="); break;
                case EQ: snprintf(label, maxLen, "=="); break;
                case NE: snprintf(label, maxLen, "!="); break;
                default: snprintf(label, maxLen, "?op?"); break;
            }
            break;
        case ConstK: 
            snprintf(label, maxLen, "%d", tree->attr.val); 
            break;
        case IdK:
            if (tree->attr.name != NULL) {
                escapeLabel(temp, tree->attr.name, sizeof(temp));
                snprintf(label, maxLen, "%s", temp);
            } else {
                snprintf(label, maxLen, "id");
            }
            break;
        case ArrIdK:
            if (tree->attr.name != NULL) {
                escapeLabel(temp, tree->attr.name, sizeof(temp));
                snprintf(label, maxLen, "%s[]", temp);
            } else {
                snprintf(label, maxLen, "arr");
            }
            break;
        default: 
            snprintf(label, maxLen, "?exp?"); 
            break;
        }
    } else {
        snprintf(label, maxLen, "?node?");
    }
}

/* Função recursiva para imprimir nós - CORRIGIDA */
static void printDotNode(TreeNode *tree, int parentId) {
    if (tree == NULL) return;
    
    int myId = nodeCounter++;
    char label[100];
    
    getNodeLabel(tree, label, sizeof(label));
    
    /* Cria o nó com estilo */
    fprintf(dotFile, "  node%d [label=\"%s\", shape=%s, style=filled, fillcolor=%s];\n", 
            myId, label, getNodeShape(tree), getNodeColor(tree));
    
    /* Conecta ao pai se houver */
    if (parentId >= 0) {
        fprintf(dotFile, "  node%d -> node%d;\n", parentId, myId);
    }
    
    /* IMPORTANTE: Processa APENAS os filhos, NÃO os irmãos aqui */
    int i;
    for (i = 0; i < MAXCHILDREN; i++) {
        if (tree->child[i] != NULL) {
            printDotNode(tree->child[i], myId);
        }
    }
    
    /* CRÍTICO: Processa irmãos com o MESMO pai, não como filhos deste nó */
    if (tree->sibling != NULL) {
        printDotNode(tree->sibling, parentId);
    }
}

/* Função para gerar arquivo .dot */
void printTreeDot(TreeNode *tree, const char *dotFilename, const char *pngFilename) {
    dotFile = fopen(dotFilename, "w");
    if (dotFile == NULL) {
        fprintf(listing, "Erro ao criar arquivo .dot\n");
        return;
    }
    
    fprintf(dotFile, "digraph AST {\n");
    fprintf(dotFile, "  rankdir=TB;\n");
    fprintf(dotFile, "  node [fontname=\"Arial\", fontsize=12];\n");
    fprintf(dotFile, "  edge [color=black, penwidth=1.5];\n\n");
    
    nodeCounter = 0;
    printDotNode(tree, -1);
    
    fprintf(dotFile, "}\n");
    fclose(dotFile);
    
    fprintf(listing, "\n=== GRAPHVIZ ===\n");
    fprintf(listing, "Arquivo DOT gerado: %s\n", dotFilename);
    fprintf(listing, "Para visualizar:\n");
    fprintf(listing, "  PNG:  dot -Tpng %s -o %s\n", dotFilename, pngFilename);
    fprintf(listing, "  PDF:  dot -Tpdf %s -o ast.pdf\n", dotFilename);
    fprintf(listing, "  SVG:  dot -Tsvg %s -o ast.svg\n\n", dotFilename);
    
    /* Tenta gerar PNG automaticamente */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "dot -Tpng %s -o %s 2>nul", dotFilename, pngFilename);
    int ret = system(cmd);
    if (ret == 0) {
        fprintf(listing, "✓ Arquivo %s gerado automaticamente!\n\n", pngFilename);
    }
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