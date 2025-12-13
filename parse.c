/*
 * Parser (Analisador Sintático) para C-
 */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token;

static TreeNode *declaration_list(void);
static TreeNode *declaration(void);
static TreeNode *var_declaration(void);
static TreeNode *fun_declaration(void);
static TreeNode *params(void);
static TreeNode *param_list(void);
static TreeNode *param(void);
static TreeNode *compound_stmt(void);
static TreeNode *local_declarations(void);
static TreeNode *statement_list(void);
static TreeNode *statement(void);
static TreeNode *expression_stmt(void);
static TreeNode *selection_stmt(void);
static TreeNode *iteration_stmt(void);
static TreeNode *return_stmt(void);
static TreeNode *expression(void);
static TreeNode *var(void);
static TreeNode *simple_expression(TreeNode *);
static TreeNode *additive_expression(TreeNode *);
static TreeNode *term(TreeNode *);
static TreeNode *factor(void);
static TreeNode *call(char *);
static TreeNode *args(void);
static TreeNode *arg_list(void);

static void syntaxError(char *message) {
    fprintf(listing, "\nERRO SINTATICO: %s - LINHA: %d\n", message, lineno);
    Error = TRUE;
}

static void match(TokenType expected) {
    if (token == expected) token = getToken();
    else {
        syntaxError("token inesperado");
        printToken(token, tokenString);
        fprintf(listing, "      ");
    }
}

TreeNode *declaration_list(void) {
    TreeNode *t = declaration();
    TreeNode *p = t;
    while (token != ENDFILE) {
        TreeNode *q = declaration();
        if (q != NULL) {
            if (t == NULL) t = p = q;
            else {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode *declaration(void) {
    TreeNode *t = NULL;
    ExpType type;
    char *identifier;
    
    if (token == INT) type = Integer;
    else if (token == VOID) type = Void;
    else {
        syntaxError("tipo esperado (int ou void)");
        return NULL;
    }
    match(token);
    
    if (token == ID) {
        identifier = copyString(tokenString);
        match(ID);
    } else {
        syntaxError("identificador esperado");
        return NULL;
    }
    
    if (token == LPAREN) {
        t = fun_declaration();
        if (t != NULL) {
            t->type = type;
            t->attr.name = identifier;
        }
    } else {
        t = var_declaration();
        if (t != NULL) {
            t->type = type;
            t->attr.name = identifier;
        }
    }
    
    return t;
}

TreeNode *var_declaration(void) {
    TreeNode *t = newStmtNode(VarDeclK);
    
    if (token == LBRACKET) {
        match(LBRACKET);
        if (token == NUM) {
            t->arraySize = atoi(tokenString);
            t->type = IntegerArray;
            match(NUM);
        }
        match(RBRACKET);
    }
    match(SEMI);
    return t;
}

TreeNode *fun_declaration(void) {
    TreeNode *t = newStmtNode(FunDeclK);
    match(LPAREN);
    t->child[0] = params();
    match(RPAREN);
    t->child[1] = compound_stmt();
    return t;
}

TreeNode *params(void) {
    TreeNode *t = NULL;
    if (token == VOID) {
        match(VOID);
    } else {
        t = param_list();
    }
    return t;
}

TreeNode *param_list(void) {
    TreeNode *t = param();
    TreeNode *p = t;
    while (token == COMMA) {
        match(COMMA);
        TreeNode *q = param();
        if (q != NULL) {
            if (t == NULL) t = p = q;
            else {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode *param(void) {
    TreeNode *t = newStmtNode(ParamK);
    
    if (token == INT) t->type = Integer;
    else if (token == VOID) t->type = Void;
    match(token);
    
    if (token == ID) {
        t->attr.name = copyString(tokenString);
        match(ID);
    }
    
    if (token == LBRACKET) {
        match(LBRACKET);
        match(RBRACKET);
        t->type = IntegerArray;
    }
    
    return t;
}

TreeNode *compound_stmt(void) {
    TreeNode *t = newStmtNode(CompoundK);
    match(LBRACE);
    t->child[0] = local_declarations();
    t->child[1] = statement_list();
    match(RBRACE);
    return t;
}

TreeNode *local_declarations(void) {
    TreeNode *t = NULL;
    TreeNode *p;
    
    while (token == INT || token == VOID) {
        TreeNode *q = declaration();
        if (q != NULL) {
            if (t == NULL) t = p = q;
            else {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode *statement_list(void) {
    TreeNode *t = statement();
    TreeNode *p = t;
    
    while (token != RBRACE && token != ENDFILE) {
        TreeNode *q = statement();
        if (q != NULL) {
            if (t == NULL) t = p = q;
            else {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode *statement(void) {
    TreeNode *t = NULL;
    switch (token) {
    case IF: t = selection_stmt(); break;
    case WHILE: t = iteration_stmt(); break;
    case RETURN: t = return_stmt(); break;
    case LBRACE: t = compound_stmt(); break;
    case ID:
    case LPAREN:
    case NUM:
    case SEMI:
        t = expression_stmt();
        break;
    default:
        syntaxError("comando inesperado");
        token = getToken();
        break;
    }
    return t;
}

TreeNode *expression_stmt(void) {
    TreeNode *t = NULL;
    if (token == SEMI) {
        match(SEMI);
    } else {
        t = expression();
        match(SEMI);
    }
    return t;
}

TreeNode *selection_stmt(void) {
    TreeNode *t = newStmtNode(IfK);
    match(IF);
    match(LPAREN);
    t->child[0] = expression();
    match(RPAREN);
    t->child[1] = statement();
    if (token == ELSE) {
        match(ELSE);
        t->child[2] = statement();
    }
    return t;
}

TreeNode *iteration_stmt(void) {
    TreeNode *t = newStmtNode(WhileK);
    match(WHILE);
    match(LPAREN);
    t->child[0] = expression();
    match(RPAREN);
    t->child[1] = statement();
    return t;
}

TreeNode *return_stmt(void) {
    TreeNode *t = newStmtNode(ReturnK);
    match(RETURN);
    if (token != SEMI) {
        t->child[0] = expression();
    }
    match(SEMI);
    return t;
}

/* 
 * TRECHO CORRIGIDO da função expression() no parse.c
 * Substitua a função expression() inteira por esta versão
 */

TreeNode *expression(void) {
    TreeNode *t = NULL;
    
    if (token == ID) {
        char *identifier = copyString(tokenString);
        match(ID);
        
        if (token == LBRACKET) {
            /* Array: pode ser acesso ou atribuição */
            TreeNode *arr = newExpNode(ArrIdK);
            arr->attr.name = identifier;
            match(LBRACKET);
            arr->child[0] = expression();
            match(RBRACKET);
            
            if (token == ASSIGN) {
                /* Atribuição a array: arr[i] = expr */
                t = newStmtNode(AssignK);
                t->attr.name = identifier;
                t->child[0] = arr;
                match(ASSIGN);
                t->child[1] = expression();
            } else {
                /* Acesso a array: arr[i] em expressão */
                t = simple_expression(arr);
            }
        } else if (token == ASSIGN) {
            /* Atribuição simples: var = expr */
            t = newStmtNode(AssignK);
            t->attr.name = identifier;
            match(ASSIGN);
            t->child[1] = expression();
        } else if (token == LPAREN) {
            /* Chamada de função */
            t = call(identifier);
            t = simple_expression(t);
        } else {
            /* Identificador simples em expressão */
            TreeNode *id = newExpNode(IdK);
            id->attr.name = identifier;
            t = simple_expression(id);
        }
    } else {
        t = simple_expression(NULL);
    }
    
    return t;
}

TreeNode *var(void) {
    TreeNode *t = NULL;
    if (token == ID) {
        t = newExpNode(IdK);
        t->attr.name = copyString(tokenString);
        match(ID);
        
        if (token == LBRACKET) {
            t->kind.exp = ArrIdK;
            match(LBRACKET);
            t->child[0] = expression();
            match(RBRACKET);
        }
    }
    return t;
}

TreeNode *simple_expression(TreeNode *k) {
    TreeNode *t = additive_expression(k);
    
    if (token == LE || token == LT || token == GT || 
        token == GE || token == EQ || token == NE) {
        TreeNode *p = newExpNode(OpK);
        p->attr.op = token;
        p->child[0] = t;
        t = p;
        match(token);
        t->child[1] = additive_expression(NULL);
    }
    
    return t;
}

TreeNode *additive_expression(TreeNode *k) {
    TreeNode *t = term(k);
    
    while (token == PLUS || token == MINUS) {
        TreeNode *p = newExpNode(OpK);
        p->attr.op = token;
        p->child[0] = t;
        t = p;
        match(token);
        p->child[1] = term(NULL);
    }
    
    return t;
}

TreeNode *term(TreeNode *k) {
    TreeNode *t = (k != NULL) ? k : factor();
    
    while (token == TIMES || token == OVER) {
        TreeNode *p = newExpNode(OpK);
        p->attr.op = token;
        p->child[0] = t;
        t = p;
        match(token);
        p->child[1] = factor();
    }
    
    return t;
}

TreeNode *factor(void) {
    TreeNode *t = NULL;
    
    switch (token) {
    case NUM:
        t = newExpNode(ConstK);
        t->attr.val = atoi(tokenString);
        match(NUM);
        break;
    case ID:
        {
            char *identifier = copyString(tokenString);
            match(ID);
            
            if (token == LPAREN) {
                t = call(identifier);
            } else if (token == LBRACKET) {
                t = newExpNode(ArrIdK);
                t->attr.name = identifier;
                match(LBRACKET);
                t->child[0] = expression();
                match(RBRACKET);
            } else {
                t = newExpNode(IdK);
                t->attr.name = identifier;
            }
        }
        break;
    case LPAREN:
        match(LPAREN);
        t = expression();
        match(RPAREN);
        break;
    default:
        syntaxError("token inesperado");
        token = getToken();
        break;
    }
    
    return t;
}

TreeNode *call(char *identifier) {
    TreeNode *t = newStmtNode(CallK);
    t->attr.name = identifier;
    match(LPAREN);
    t->child[0] = args();
    match(RPAREN);
    return t;
}

TreeNode *args(void) {
    if (token != RPAREN) {
        return arg_list();
    }
    return NULL;
}

TreeNode *arg_list(void) {
    TreeNode *t = expression();
    TreeNode *p = t;
    
    while (token == COMMA) {
        match(COMMA);
        TreeNode *q = expression();
        if (q != NULL) {
            if (t == NULL) t = p = q;
            else {
                p->sibling = q;
                p = q;
            }
        }
    }
    
    return t;
}

TreeNode *parse(void) {
    TreeNode *t;
    token = getToken();
    t = declaration_list();
    if (token != ENDFILE)
        syntaxError("fim de arquivo esperado");
    return t;
}
