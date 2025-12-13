/*
 * Gerador de Código Intermediário (3 Endereços) para C-
 */

#include "globals.h"
#include "util.h"
#include "cgen.h"

static int tempCounter = 0;
static int labelCounter = 0;

/* Gera novo temporário */
static char *newTemp(void) {
    static char temp[20];
    sprintf(temp, "t%d", tempCounter++);
    return copyString(temp);
}

/* Gera novo label */
static char *newLabel(void) {
    static char label[20];
    sprintf(label, "L%d", labelCounter++);
    return copyString(label);
}

/* Emite instrução de três endereços */
static void emitCode(char *op, char *arg1, char *arg2, char *result) {
    fprintf(listing, "(%s, %s, %s, %s)\n", op, arg1, arg2, result);
}

/* Geração de código para expressões - retorna temporário com resultado */
static char *cGen(TreeNode *tree) {
    if (tree == NULL) return "-";
    
    char *t1, *t2, *t3;
    char opStr[10];
    char numStr[20];
    
    switch (tree->nodekind) {
    case ExpK:
        switch (tree->kind.exp) {
        case ConstK:
            sprintf(numStr, "%d", tree->attr.val);
            t1 = newTemp();
            emitCode("CONST", numStr, "-", t1);
            return t1;
            
        case IdK:
            return tree->attr.name;
            
        case ArrIdK:
            t1 = cGen(tree->child[0]); /* índice */
            t2 = newTemp();
            emitCode("ARR_LOAD", tree->attr.name, t1, t2);
            return t2;
            
        case OpK:
            t1 = cGen(tree->child[0]);
            t2 = cGen(tree->child[1]);
            t3 = newTemp();
            
            switch (tree->attr.op) {
            case PLUS: strcpy(opStr, "+"); break;
            case MINUS: strcpy(opStr, "-"); break;
            case TIMES: strcpy(opStr, "*"); break;
            case OVER: strcpy(opStr, "/"); break;
            case LT: strcpy(opStr, "<"); break;
            case LE: strcpy(opStr, "<="); break;
            case GT: strcpy(opStr, ">"); break;
            case GE: strcpy(opStr, ">="); break;
            case EQ: strcpy(opStr, "=="); break;
            case NE: strcpy(opStr, "!="); break;
            default: strcpy(opStr, "?"); break;
            }
            
            emitCode(opStr, t1, t2, t3);
            return t3;
            
        default:
            return "-";
        }
        break;
        
    case StmtK:
        switch (tree->kind.stmt) {
        case AssignK:
            if (tree->child[0] != NULL && tree->child[0]->kind.exp == ArrIdK) {
                /* Atribuição a array: arr[i] = expr */
                t1 = cGen(tree->child[0]->child[0]); /* índice */
                t2 = cGen(tree->child[1]); /* valor */
                emitCode("ARR_STORE", t2, t1, tree->attr.name);
            } else {
                /* Atribuição simples: var = expr */
                t1 = cGen(tree->child[1]);
                emitCode("=", t1, "-", tree->attr.name);
            }
            return "-";
            
        case IfK:
            {
                char *labelElse = newLabel();
                char *labelEnd = newLabel();
                
                t1 = cGen(tree->child[0]); /* condição */
                emitCode("IF_FALSE", t1, "-", labelElse);
                
                /* Bloco then */
                if (tree->child[1] != NULL)
                    cGen(tree->child[1]);
                
                emitCode("GOTO", "-", "-", labelEnd);
                emitCode("LABEL", "-", "-", labelElse);
                
                /* Bloco else */
                if (tree->child[2] != NULL)
                    cGen(tree->child[2]);
                
                emitCode("LABEL", "-", "-", labelEnd);
            }
            return "-";
            
        case WhileK:
            {
                char *labelStart = newLabel();
                char *labelEnd = newLabel();
                
                emitCode("LABEL", "-", "-", labelStart);
                t1 = cGen(tree->child[0]); /* condição */
                emitCode("IF_FALSE", t1, "-", labelEnd);
                
                /* Corpo do loop */
                if (tree->child[1] != NULL)
                    cGen(tree->child[1]);
                
                emitCode("GOTO", "-", "-", labelStart);
                emitCode("LABEL", "-", "-", labelEnd);
            }
            return "-";
            
        case ReturnK:
            if (tree->child[0] != NULL) {
                t1 = cGen(tree->child[0]);
                emitCode("RETURN", t1, "-", "-");
            } else {
                emitCode("RETURN", "-", "-", "-");
            }
            return "-";
            
        case CallK:
            {
                /* Processar argumentos */
                TreeNode *arg = tree->child[0];
                int argCount = 0;
                while (arg != NULL) {
                    t1 = cGen(arg);
                    emitCode("PARAM", t1, "-", "-");
                    argCount++;
                    arg = arg->sibling;
                }
                
                t1 = newTemp();
                char argCountStr[20];
                sprintf(argCountStr, "%d", argCount);
                emitCode("CALL", tree->attr.name, argCountStr, t1);
                return t1;
            }
            
        case FunDeclK:
            {
                char *typeStr = (tree->type == Integer) ? "int" : "void";
                emitCode("FUN", typeStr, tree->attr.name, "-");
                
                /* Parâmetros */
                TreeNode *param = tree->child[0];
                while (param != NULL) {
                    char *pType = (param->type == Integer) ? "int" : 
                                  (param->type == IntegerArray) ? "int[]" : "void";
                    emitCode("ARG", pType, param->attr.name, tree->attr.name);
                    param = param->sibling;
                }
                
                /* Corpo da função */
                if (tree->child[1] != NULL)
                    cGen(tree->child[1]);
                
                emitCode("ENDFUN", tree->attr.name, "-", "-");
            }
            return "-";
            
        case VarDeclK:
            {
                char *typeStr = (tree->type == IntegerArray) ? "int[]" : "int";
                if (tree->type == IntegerArray) {
                    char sizeStr[20];
                    sprintf(sizeStr, "%d", tree->arraySize);
                    emitCode("VAR", typeStr, tree->attr.name, sizeStr);
                } else {
                    emitCode("VAR", typeStr, tree->attr.name, "-");
                }
            }
            return "-";
            
        case CompoundK:
            if (tree->child[0] != NULL) /* declarações locais */
                cGen(tree->child[0]);
            if (tree->child[1] != NULL) /* lista de statements */
                cGen(tree->child[1]);
            return "-";
            
        default:
            return "-";
        }
        break;
        
    default:
        return "-";
    }
    
    return "-";
}

/* Gera código para árvore completa */
void codeGen(TreeNode *syntaxTree) {
    TreeNode *t = syntaxTree;
    
    while (t != NULL) {
        cGen(t);
        t = t->sibling;
    }
    
    emitCode("HALT", "-", "-", "-");
}