#include "globals.h"

static TokenType token;
TokenType getToken(void);

static TreeNode *declaration_list(void);
static TreeNode *declaration(void);
static TreeNode *var_declaration(void);
static TreeNode *fun_declaration(void);
static TreeNode *params(void);
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
static TreeNode *simple_expression(TreeNode *);
static TreeNode *additive_expression(TreeNode *);
static TreeNode *term(TreeNode *);
static TreeNode *factor(void);
static TreeNode *call(char *);
static TreeNode *args(void);

static void syntaxError(char *message) {
    fprintf(listing, "ERRO SINTATICO: %s - LINHA: %d\n", message, lineno);
    Error = 1;
}

static void match(TokenType expected) {
    if (token == expected)
        token = getToken();
    else {
        char msg[100];
        sprintf(msg, "token inesperado, esperado outro token");
        syntaxError(msg);
    }
}

static TreeNode *newStmtNode(StmtKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    if (t == NULL) {
        fprintf(listing, "Erro de memoria\n");
        exit(1);
    }
    for (int i = 0; i < MAX_CHILDREN; i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
    return t;
}

static TreeNode *newExpNode(ExpKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    if (t == NULL) {
        fprintf(listing, "Erro de memoria\n");
        exit(1);
    }
    for (int i = 0; i < MAX_CHILDREN; i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
    return t;
}

static TreeNode *newDeclNode(DeclKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    if (t == NULL) {
        fprintf(listing, "Erro de memoria\n");
        exit(1);
    }
    for (int i = 0; i < MAX_CHILDREN; i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = DeclK;
    t->kind.decl = kind;
    t->lineno = lineno;
    return t;
}

TreeNode *parse(void) {
    token = getToken();
    TreeNode *t = declaration_list();
    if (token != ENDFILE)
        syntaxError("Codigo apos fim do programa");
    return t;
}

static TreeNode *declaration_list(void) {
    TreeNode *t = declaration();
    TreeNode *p = t;
    while (token != ENDFILE) {
        TreeNode *q = declaration();
        if (q != NULL) {
            if (t == NULL) {
                t = p = q;
            } else {
                p->sibling = q;
                p = q;
            }
        } else break;
    }
    return t;
}

static TreeNode *declaration(void) {
    TreeNode *t = NULL;
    TokenType type_token = token;
    
    if (token == INT || token == VOID) {
        match(token);
        if (token == ID) {
            char *name = strdup(tokenString);
            match(ID);
            
            if (token == LPAREN) {
                t = newDeclNode(FunK);
                t->attr.name = name;
                t->type = (type_token == INT) ? Integer : Void;
                match(LPAREN);
                t->child[0] = params();
                match(RPAREN);
                t->child[1] = compound_stmt();
            } else {
                t = newDeclNode(VarK);
                t->attr.name = name;
                t->type = Integer;
                
                if (token == LBRACKET) {
                    match(LBRACKET);
                    if (token == NUM) {
                        t->kind.decl = ArrVarK;
                        t->arraySize = atoi(tokenString);
                        t->type = Array;
                        match(NUM);
                    }
                    match(RBRACKET);
                }
                match(SEMI);
            }
        }
    }
    return t;
}

static TreeNode *params(void) {
    if (token == VOID) {
        match(VOID);
        if (token == RPAREN) return NULL;
    }
    TreeNode *t = param();
    TreeNode *p = t;
    while (token == COMMA) {
        match(COMMA);
        TreeNode *q = param();
        if (q != NULL) {
            p->sibling = q;
            p = q;
        }
    }
    return t;
}

static TreeNode *param(void) {
    TreeNode *t = newDeclNode(ParamK);
    if (token == INT) {
        t->type = Integer;
        match(INT);
    } else if (token == VOID) {
        t->type = Void;
        match(VOID);
    }
    
    if (token == ID) {
        t->attr.name = strdup(tokenString);
        match(ID);
        if (token == LBRACKET) {
            match(LBRACKET);
            match(RBRACKET);
            t->kind.decl = ArrParamK;
            t->type = Array;
        }
    }
    return t;
}

static TreeNode *compound_stmt(void) {
    match(LBRACE);
    TreeNode *t = newStmtNode(CompoundK);
    t->child[0] = local_declarations();
    t->child[1] = statement_list();
    match(RBRACE);
    return t;
}

static TreeNode *local_declarations(void) {
    TreeNode *t = NULL;
    TreeNode *p = NULL;
    
    while (token == INT || token == VOID) {
        TreeNode *q = var_declaration();
        if (t == NULL) {
            t = p = q;
        } else {
            p->sibling = q;
            p = q;
        }
    }
    return t;
}

static TreeNode *var_declaration(void) {
    TreeNode *t = newDeclNode(VarK);
    if (token == INT) {
        t->type = Integer;
        match(INT);
    } else {
        match(VOID);
        t->type = Void;
    }
    
    if (token == ID) {
        t->attr.name = strdup(tokenString);
        match(ID);
        if (token == LBRACKET) {
            match(LBRACKET);
            if (token == NUM) {
                t->kind.decl = ArrVarK;
                t->arraySize = atoi(tokenString);
                t->type = Array;
                match(NUM);
            }
            match(RBRACKET);
        }
    }
    match(SEMI);
    return t;
}

static TreeNode *statement_list(void) {
    TreeNode *t = NULL;
    TreeNode *p = NULL;
    
    while (token != RBRACE && token != ENDFILE) {
        TreeNode *q = statement();
        if (q != NULL) {
            if (t == NULL) {
                t = p = q;
            } else {
                p->sibling = q;
                p = q;
            }
        } else break;
    }
    return t;
}

static TreeNode *statement(void) {
    TreeNode *t = NULL;
    switch (token) {
    case IF: t = selection_stmt(); break;
    case WHILE: t = iteration_stmt(); break;
    case RETURN: t = return_stmt(); break;
    case LBRACE: t = compound_stmt(); break;
    default: t = expression_stmt(); break;
    }
    return t;
}

static TreeNode *selection_stmt(void) {
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

static TreeNode *iteration_stmt(void) {
    TreeNode *t = newStmtNode(WhileK);
    match(WHILE);
    match(LPAREN);
    t->child[0] = expression();
    match(RPAREN);
    t->child[1] = statement();
    return t;
}

static TreeNode *return_stmt(void) {
    TreeNode *t = newStmtNode(ReturnK);
    match(RETURN);
    if (token != SEMI)
        t->child[0] = expression();
    match(SEMI);
    return t;
}

static TreeNode *expression_stmt(void) {
    TreeNode *t = NULL;
    if (token != SEMI)
        t = expression();
    match(SEMI);
    return t;
}

static TreeNode *expression(void) {
    TreeNode *t = NULL;
    if (token == ID) {
        char *name = strdup(tokenString);
        match(ID);
        
        if (token == ASSIGN) {
            t = newExpNode(AssignK);
            t->attr.name = name;
            match(ASSIGN);
            t->child[0] = expression();
        } else if (token == LBRACKET) {
            TreeNode *arr = newExpNode(ArrIdK);
            arr->attr.name = name;
            match(LBRACKET);
            arr->child[0] = expression();
            match(RBRACKET);
            
            if (token == ASSIGN) {
                t = newExpNode(AssignK);
                t->child[0] = arr;
                match(ASSIGN);
                t->child[1] = expression();
            } else {
                t = simple_expression(arr);
            }
        } else if (token == LPAREN) {
            t = call(name);
            t = simple_expression(t);
        } else {
            TreeNode *id = newExpNode(IdK);
            id->attr.name = name;
            t = simple_expression(id);
        }
    } else {
        t = simple_expression(NULL);
    }
    return t;
}

static TreeNode *simple_expression(TreeNode *k) {
    TreeNode *t = additive_expression(k);
    
    if (token == LT || token == LE || token == GT || 
        token == GE || token == EQ || token == NE) {
        TreeNode *p = newExpNode(OpK);
        p->child[0] = t;
        p->attr.op = token;
        match(token);
        p->child[1] = additive_expression(NULL);
        t = p;
    }
    return t;
}

static TreeNode *additive_expression(TreeNode *k) {
    TreeNode *t = term(k);
    
    while (token == PLUS || token == MINUS) {
        TreeNode *p = newExpNode(OpK);
        p->child[0] = t;
        p->attr.op = token;
        match(token);
        p->child[1] = term(NULL);
        t = p;
    }
    return t;
}

static TreeNode *term(TreeNode *k) {
    TreeNode *t = (k != NULL) ? k : factor();
    
    while (token == TIMES || token == OVER) {
        TreeNode *p = newExpNode(OpK);
        p->child[0] = t;
        p->attr.op = token;
        match(token);
        p->child[1] = factor();
        t = p;
    }
    return t;
}

static TreeNode *factor(void) {
    TreeNode *t = NULL;
    
    switch (token) {
    case NUM:
        t = newExpNode(ConstK);
        t->attr.val = atoi(tokenString);
        match(NUM);
        break;
    case ID: {
        char *name = strdup(tokenString);
        match(ID);
        if (token == LPAREN) {
            t = call(name);
        } else if (token == LBRACKET) {
            t = newExpNode(ArrIdK);
            t->attr.name = name;
            match(LBRACKET);
            t->child[0] = expression();
            match(RBRACKET);
        } else {
            t = newExpNode(IdK);
            t->attr.name = name;
        }
        break;
    }
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

static TreeNode *call(char *name) {
    TreeNode *t = newStmtNode(CallK);
    t->attr.name = name;
    match(LPAREN);
    if (token != RPAREN)
        t->child[0] = args();
    match(RPAREN);
    return t;
}

static TreeNode *args(void) {
    TreeNode *t = expression();
    TreeNode *p = t;
    
    while (token == COMMA) {
        match(COMMA);
        TreeNode *q = expression();
        if (q != NULL) {
            p->sibling = q;
            p = q;
        }
    }
    return t;
}