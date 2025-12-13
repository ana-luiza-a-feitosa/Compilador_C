#include "globals.h"

static int tempCounter = 0;
static int labelCounter = 0;
static FILE *code;

static char *newTemp(void) {
    char *temp = (char *)malloc(10);
    sprintf(temp, "t%d", tempCounter++);
    return temp;
}

static char *newLabel(void) {
    char *label = (char *)malloc(10);
    sprintf(label, "L%d", labelCounter++);
    return label;
}

static void emitCode(char *s) {
    fprintf(code, "%s\n", s);
}

static char *cGen(TreeNode *tree);

static char *cGenExp(TreeNode *tree) {
    char *result = NULL;
    char line[256];
    
    switch (tree->kind.exp) {
    case ConstK:
        result = (char *)malloc(20);
        sprintf(result, "%d", tree->attr.val);
        break;
        
    case IdK:
        result = strdup(tree->attr.name);
        break;
        
    case ArrIdK: {
        char *index = cGen(tree->child[0]);
        result = newTemp();
        sprintf(line, "%s = %s[%s]", result, tree->attr.name, index);
        emitCode(line);
        break;
    }
        
    case OpK: {
        char *left = cGen(tree->child[0]);
        char *right = cGen(tree->child[1]);
        result = newTemp();
        char *op = "";
        
        switch (tree->attr.op) {
        case PLUS: op = "+"; break;
        case MINUS: op = "-"; break;
        case TIMES: op = "*"; break;
        case OVER: op = "/"; break;
        case LT: op = "<"; break;
        case LE: op = "<="; break;
        case GT: op = ">"; break;
        case GE: op = ">="; break;
        case EQ: op = "=="; break;
        case NE: op = "!="; break;
        default: op = "?"; break;
        }
        
        sprintf(line, "%s = %s %s %s", result, left, op, right);
        emitCode(line);
        break;
    }
        
    case AssignK: {
        char *value = cGen(tree->child[tree->child[0] ? 1 : 0]);
        
        if (tree->child[0] && tree->child[0]->kind.exp == ArrIdK) {
            char *index = cGen(tree->child[0]->child[0]);
            sprintf(line, "%s[%s] = %s", tree->child[0]->attr.name, index, value);
        } else {
            sprintf(line, "%s = %s", tree->attr.name, value);
        }
        emitCode(line);
        result = value;
        break;
    }
        
    default:
        break;
    }
    
    return result;
}

static char *cGenStmt(TreeNode *tree) {
    char line[256];
    char *result = NULL;
    
    switch (tree->kind.stmt) {
    case IfK: {
        char *cond = cGen(tree->child[0]);
        char *labelElse = newLabel();
        char *labelEnd = newLabel();
        
        sprintf(line, "if %s == 0 goto %s", cond, labelElse);
        emitCode(line);
        
        cGen(tree->child[1]);
        
        if (tree->child[2] != NULL) {
            sprintf(line, "goto %s", labelEnd);
            emitCode(line);
        }
        
        sprintf(line, "%s:", labelElse);
        emitCode(line);
        
        if (tree->child[2] != NULL) {
            cGen(tree->child[2]);
            sprintf(line, "%s:", labelEnd);
            emitCode(line);
        }
        break;
    }
        
    case WhileK: {
        char *labelStart = newLabel();
        char *labelEnd = newLabel();
        
        sprintf(line, "%s:", labelStart);
        emitCode(line);
        
        char *cond = cGen(tree->child[0]);
        sprintf(line, "if %s == 0 goto %s", cond, labelEnd);
        emitCode(line);
        
        cGen(tree->child[1]);
        
        sprintf(line, "goto %s", labelStart);
        emitCode(line);
        
        sprintf(line, "%s:", labelEnd);
        emitCode(line);
        break;
    }
        
    case ReturnK:
        if (tree->child[0] != NULL) {
            char *value = cGen(tree->child[0]);
            sprintf(line, "return %s", value);
        } else {
            sprintf(line, "return");
        }
        emitCode(line);
        break;
        
    case CallK: {
        if (strcmp(tree->attr.name, "input") == 0) {
            result = newTemp();
            sprintf(line, "%s = input", result);
            emitCode(line);
        } else if (strcmp(tree->attr.name, "output") == 0) {
            if (tree->child[0] != NULL) {
                char *arg = cGen(tree->child[0]);
                sprintf(line, "output %s", arg);
                emitCode(line);
            }
        } else {
            TreeNode *args = tree->child[0];
            int argCount = 0;
            
            while (args != NULL) {
                char *arg = cGen(args);
                sprintf(line, "param %s", arg);
                emitCode(line);
                argCount++;
                args = args->sibling;
            }
            
            result = newTemp();
            sprintf(line, "%s = call %s, %d", result, tree->attr.name, argCount);
            emitCode(line);
        }
        break;
    }
        
    case CompoundK:
        cGen(tree->child[0]);
        cGen(tree->child[1]);
        break;
        
    default:
        break;
    }
    
    return result;
}

static char *cGen(TreeNode *tree) {
    if (tree == NULL) return NULL;
    
    char *result = NULL;
    char line[256];
    
    switch (tree->nodekind) {
    case StmtK:
        result = cGenStmt(tree);
        break;
        
    case ExpK:
        result = cGenExp(tree);
        break;
        
    case DeclK:
        if (tree->kind.decl == FunK) {
            sprintf(line, "\nfunc %s:", tree->attr.name);
            emitCode(line);
            
            cGen(tree->child[1]);
        } else if (tree->kind.decl == VarK || tree->kind.decl == ArrVarK) {
            if (tree->kind.decl == ArrVarK) {
                sprintf(line, "array %s[%d]", tree->attr.name, tree->arraySize);
            } else {
                sprintf(line, "var %s", tree->attr.name);
            }
            emitCode(line);
        }
        break;
        
    default:
        break;
    }
    
    if (tree->sibling != NULL) {
        cGen(tree->sibling);
    }
    
    return result;
}

void codeGen(TreeNode *syntaxTree, char *codefile) {
    code = fopen(codefile, "w");
    if (code == NULL) {
        fprintf(listing, "Erro ao criar arquivo de codigo\n");
        return;
    }
    
    fprintf(code, "=== CODIGO INTERMEDIARIO (3 ENDERECOS) ===\n\n");
    
    cGen(syntaxTree);
    
    fclose(code);
}