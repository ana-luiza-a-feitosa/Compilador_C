/*
 * Funções utilitárias para o compilador C-
 */

#include "globals.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <direct.h>  // _getcwd
#else
  #include <unistd.h>  // getcwd
#endif

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
    n = (int)strlen(s) + 1;
    t = (char*)malloc(n);
    if (t == NULL)
        fprintf(listing, "Erro: sem memoria\n");
    else
        strcpy(t, s);
    return t;
}

/* ========== GERAÇÃO DE GRAPHVIZ (DOT) ========== */

static int nodeCounter = 0;

/* Função auxiliar para escapar caracteres especiais em strings DOT */
static void escapeString(char *dest, const char *src, int maxLen) {
    int i = 0, j = 0;
    if (!dest || !src || maxLen <= 0) return;

    while (src[i] != '\0' && j < maxLen - 1) {
        if (src[i] == '"' || src[i] == '\\') {
            if (j < maxLen - 2) dest[j++] = '\\';
        }
        dest[j++] = src[i++];
    }
    dest[j] = '\0';
}

/* Função auxiliar para obter o label do nó */
static void getNodeLabel(TreeNode *node, char *buffer, int bufsize) {
    char escaped[200];

    if (node == NULL) {
        snprintf(buffer, bufsize, "NULL");
        return;
    }

    if (node->nodekind == StmtK) {
        switch (node->kind.stmt) {
        case IfK:
            snprintf(buffer, bufsize, "if");
            break;
        case WhileK:
            snprintf(buffer, bufsize, "while");
            break;
        case AssignK:
            escapeString(escaped, node->attr.name ? node->attr.name : "", (int)sizeof(escaped));
            snprintf(buffer, bufsize, "= (%s)", escaped);
            break;
        case ReturnK:
            snprintf(buffer, bufsize, "return");
            break;
        case FunDeclK:
            escapeString(escaped, node->attr.name ? node->attr.name : "", (int)sizeof(escaped));
            snprintf(buffer, bufsize, "%s : %s",
                    escaped,
                    node->type == Integer ? "int" : "void");
            break;
        case VarDeclK:
            escapeString(escaped, node->attr.name ? node->attr.name : "", (int)sizeof(escaped));
            if (node->type == IntegerArray)
                snprintf(buffer, bufsize, "int %s[%d]", escaped, node->arraySize);
            else
                snprintf(buffer, bufsize, "int %s", escaped);
            break;
        case ParamK:
            escapeString(escaped, node->attr.name ? node->attr.name : "", (int)sizeof(escaped));
            if (node->type == IntegerArray)
                snprintf(buffer, bufsize, "%s[]", escaped);
            else
                snprintf(buffer, bufsize, "%s", escaped);
            break;
        case CallK:
            escapeString(escaped, node->attr.name ? node->attr.name : "", (int)sizeof(escaped));
            snprintf(buffer, bufsize, "%s()", escaped);
            break;
        case CompoundK:
            snprintf(buffer, bufsize, "compound");
            break;
        default:
            snprintf(buffer, bufsize, "?stmt?");
            break;
        }
    } else if (node->nodekind == ExpK) {
        switch (node->kind.exp) {
        case OpK:
            switch (node->attr.op) {
                case PLUS:  snprintf(buffer, bufsize, "+");  break;
                case MINUS: snprintf(buffer, bufsize, "-");  break;
                case TIMES: snprintf(buffer, bufsize, "*");  break;
                case OVER:  snprintf(buffer, bufsize, "/");  break;
                case LT:    snprintf(buffer, bufsize, "<");  break;
                case LE:    snprintf(buffer, bufsize, "<="); break;
                case GT:    snprintf(buffer, bufsize, ">");  break;
                case GE:    snprintf(buffer, bufsize, ">="); break;
                case EQ:    snprintf(buffer, bufsize, "=="); break;
                case NE:    snprintf(buffer, bufsize, "!="); break;
                default:    snprintf(buffer, bufsize, "?op?"); break;
            }
            break;
        case ConstK:
            snprintf(buffer, bufsize, "%d", node->attr.val);
            break;
        case IdK:
            escapeString(escaped, node->attr.name ? node->attr.name : "", (int)sizeof(escaped));
            snprintf(buffer, bufsize, "%s", escaped);
            break;
        case ArrIdK:
            escapeString(escaped, node->attr.name ? node->attr.name : "", (int)sizeof(escaped));
            snprintf(buffer, bufsize, "%s[]", escaped);
            break;
        default:
            snprintf(buffer, bufsize, "?exp?");
            break;
        }
    } else {
        snprintf(buffer, bufsize, "?node?");
    }
}

/* Gera o grafo em formato DOT recursivamente */
static void generateDOT(FILE *out, TreeNode *tree, int *counter) {
    if (tree == NULL) return;

    int myID = (*counter)++;
    char label[200];
    getNodeLabel(tree, label, (int)sizeof(label));

    /* Define a cor e forma do nó baseado no tipo */
    const char *shape = "ellipse";
    const char *color = "lightblue";

    if (tree->nodekind == StmtK) {
        switch (tree->kind.stmt) {
        case FunDeclK:
            color = "lightgreen";
            shape = "box";
            break;
        case IfK:
        case WhileK:
            color = "lightyellow";
            shape = "diamond";
            break;
        case AssignK:
            color = "lightcoral";
            break;
        case VarDeclK:
        case ParamK:
            color = "lightgray";
            shape = "box";
            break;
        case CallK:
            color = "plum";
            break;
        default:
            break;
        }
    } else if (tree->nodekind == ExpK) {
        if (tree->kind.exp == OpK) {
            color = "orange";
            shape = "circle";
        } else if (tree->kind.exp == ConstK) {
            color = "white";
            shape = "box";
        } else {
            color = "cyan";
        }
    }

    fprintf(out,
        "  node%d [label=\"%s\", shape=%s, style=filled, fillcolor=%s];\n",
        myID, label, shape, color);

    /* Processa filhos */
    for (int i = 0; i < MAXCHILDREN; i++) {
        if (tree->child[i] != NULL) {
            int childID = *counter;                 /* id do próximo nó antes da recursão */
            generateDOT(out, tree->child[i], counter);
            fprintf(out, "  node%d -> node%d [label=\"child%d\"];\n", myID, childID, i);
        }
    }

    /* Processa irmãos */
    if (tree->sibling != NULL) {
        int siblingID = *counter;
        generateDOT(out, tree->sibling, counter);
        fprintf(out, "  node%d -> node%d [label=\"sibling\", style=dashed];\n", myID, siblingID);
    }
}

/* Tenta executar o Graphviz (dot) e gerar ast.png */
static int tryGeneratePngWithGraphviz(void) {
#ifdef _WIN32
    /* 1) tenta dot no PATH */
    int ret = system("dot -Tpng ast.dot -o ast.png 2>nul");
    if (ret == 0) return 0;

    /* 2) fallback: caminho comum do Graphviz (ajuste se seu Graphviz estiver em outro lugar) */
    ret = system("\"C:\\Program Files\\Graphviz\\bin\\dot.exe\" -Tpng ast.dot -o ast.png 2>nul");
    return ret;
#else
    int ret = system("dot -Tpng ast.dot -o ast.png 2>/dev/null");
    return ret;
#endif
}

/* Função pública para imprimir a árvore */
void printTree(TreeNode *tree) {
    if (tree == NULL) {
        fprintf(listing, "(árvore vazia)\n");
        return;
    }

    /* Mostra CWD para você achar onde o arquivo está sendo criado */
    {
        char cwd[1024];
#ifdef _WIN32
        if (_getcwd(cwd, sizeof(cwd)) != NULL) {
#else
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
#endif
            fprintf(listing, "CWD (pasta atual): %s\n", cwd);
        }
    }

    /* Abre arquivo .dot */
    FILE *dotFile = fopen("ast.dot", "w");
    if (dotFile == NULL) {
        fprintf(listing, "ERRO: Não foi possível criar arquivo ast.dot\n");
        return;
    }

    /* Cabeçalho do arquivo DOT */
    fprintf(dotFile, "digraph AST {\n");
    fprintf(dotFile, "  rankdir=TB;\n");
    fprintf(dotFile, "  node [fontname=\"Arial\"];\n");
    fprintf(dotFile, "  edge [fontname=\"Arial\", fontsize=10];\n\n");

    /* Gera o grafo */
    nodeCounter = 0;
    generateDOT(dotFile, tree, &nodeCounter);

    /* Rodapé */
    fprintf(dotFile, "}\n");
    fclose(dotFile);

    fprintf(listing, "AST gerada no arquivo ast.dot\n");
    fprintf(listing, "Execute: dot -Tpng ast.dot -o ast.png\n");
    fprintf(listing, "Ou execute: dot -Tpdf ast.dot -o ast.pdf\n\n");

    /* Tenta gerar a imagem automaticamente */
    int ret = tryGeneratePngWithGraphviz();
    if (ret == 0) {
        fprintf(listing, "✓ Imagem ast.png gerada com sucesso!\n\n");
    } else {
        fprintf(listing, "→ Não consegui executar o Graphviz automaticamente.\n");
        fprintf(listing, "   Verifique se o 'dot' está no PATH (cmd: where dot; dot -V)\n");
        fprintf(listing, "   E rode manualmente: dot -Tpng ast.dot -o ast.png\n\n");
    }
}
