#include "globals.h"

void printToken(TokenType token, const char *tokenString) {
    switch (token) {
    case IF:
    case ELSE:
    case WHILE:
    case RETURN:
    case INT:
    case VOID:
    case INPUT:
    case OUTPUT:
        fprintf(listing, "palavra-chave: %s\n", tokenString);
        break;
    case ASSIGN: fprintf(listing, "=\n"); break;
    case EQ: fprintf(listing, "==\n"); break;
    case LT: fprintf(listing, "<\n"); break;
    case GT: fprintf(listing, ">\n"); break;
    case LE: fprintf(listing, "<=\n"); break;
    case GE: fprintf(listing, ">=\n"); break;
    case NE: fprintf(listing, "!=\n"); break;
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
        fprintf(listing, "NUM, val= %s\n", tokenString);
        break;
    case ID:
        fprintf(listing, "ID, nome= %s\n", tokenString);
        break;
    case ERROR:
        fprintf(listing, "ERRO LEXICO: '%s' - LINHA: %d\n", tokenString, lineno);
        break;
    default:
        fprintf(listing, "Token desconhecido: %d\n", token);
    }
}

static int indentno = 0;

#define INDENT indentno += 2
#define UNINDENT indentno -= 2

static void printSpaces(void) {
    for (int i = 0; i < indentno; i++)
        fprintf(listing, " ");
}

void printTree(TreeNode *tree) {
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
            case ReturnK:
                fprintf(listing, "Return\n");
                break;
            case CompoundK:
                fprintf(listing, "Compound Statement\n");
                break;
            case CallK:
                fprintf(listing, "Call: %s\n", tree->attr.name);
                break;
            default:
                fprintf(listing, "Unknown StmtNode kind\n");
                break;
            }
        } else if (tree->nodekind == ExpK) {
            switch (tree->kind.exp) {
            case OpK:
                fprintf(listing, "Op: ");
                switch (tree->attr.op) {
                case PLUS: fprintf(listing, "+\n"); break;
                case MINUS: fprintf(listing, "-\n"); break;
                case TIMES: fprintf(listing, "*\n"); break;
                case OVER: fprintf(listing, "/\n"); break;
                case LT: fprintf(listing, "<\n"); break;
                case LE: fprintf(listing, "<=\n"); break;
                case GT: fprintf(listing, ">\n"); break;
                case GE: fprintf(listing, ">=\n"); break;
                case EQ: fprintf(listing, "==\n"); break;
                case NE: fprintf(listing, "!=\n"); break;
                default: fprintf(listing, "?\n"); break;
                }
                break;
            case ConstK:
                fprintf(listing, "Const: %d\n", tree->attr.val);
                break;
            case IdK:
                fprintf(listing, "Id: %s\n", tree->attr.name);
                break;
            case ArrIdK:
                fprintf(listing, "Array Id: %s\n", tree->attr.name);
                break;
            case AssignK:
                fprintf(listing, "Assign: %s\n", 
                        tree->attr.name ? tree->attr.name : "(array)");
                break;
            default:
                fprintf(listing, "Unknown ExpNode kind\n");
                break;
            }
        } else if (tree->nodekind == DeclK) {
            switch (tree->kind.decl) {
            case FunK:
                fprintf(listing, "Function Declaration: %s, type: %s\n",
                        tree->attr.name,
                        tree->type == Integer ? "int" : "void");
                break;
            case VarK:
                fprintf(listing, "Variable Declaration: %s, type: int\n",
                        tree->attr.name);
                break;
            case ArrVarK:
                fprintf(listing, "Array Variable Declaration: %s[%d], type: int\n",
                        tree->attr.name, tree->arraySize);
                break;
            case ParamK:
                fprintf(listing, "Parameter: %s, type: %s\n",
                        tree->attr.name,
                        tree->type == Integer ? "int" : "void");
                break;
            case ArrParamK:
                fprintf(listing, "Array Parameter: %s[], type: int\n",
                        tree->attr.name);
                break;
            default:
                fprintf(listing, "Unknown DeclNode kind\n");
                break;
            }
        } else {
            fprintf(listing, "Unknown node kind\n");
        }
        
        for (int i = 0; i < MAX_CHILDREN; i++)
            printTree(tree->child[i]);
        tree = tree->sibling;
    }
    UNINDENT;
}