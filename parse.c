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
static TreeNode *simple_expression(TreeNode *);
static TreeNode *additive_expression(TreeNode *);
static TreeNode *term(TreeNode *);
static TreeNode *factor(void);
static TreeNode *call(char *);
static TreeNode *args(void);
static TreeNode *arg_list(void);

/* Função de erro sintático melhorada */
static void syntaxError(char *message) {
    fprintf(listing, "\nERRO SINTATICO: %s - LINHA: %d\n", message, lineno);
    Error = TRUE;
}

/* Match melhorado com mensagem de erro específica */
static void match(TokenType expected) {
    if (token == expected) {
        token = getToken();
    } else {
        char expectedStr[50], foundStr[50];
        
        /* Converte token esperado para string */
        switch(expected) {
            case SEMI: strcpy(expectedStr, "';'"); break;
            case LPAREN: strcpy(expectedStr, "'('"); break;
            case RPAREN: strcpy(expectedStr, "')'"); break;
            case LBRACKET: strcpy(expectedStr, "'['"); break;
            case RBRACKET: strcpy(expectedStr, "']'"); break;
            case LBRACE: strcpy(expectedStr, "'{'"); break;
            case RBRACE: strcpy(expectedStr, "'}'"); break;
            case ASSIGN: strcpy(expectedStr, "'='"); break;
            case COMMA: strcpy(expectedStr, "','"); break;
            case IF: strcpy(expectedStr, "'if'"); break;
            case ELSE: strcpy(expectedStr, "'else'"); break;
            case WHILE: strcpy(expectedStr, "'while'"); break;
            case RETURN: strcpy(expectedStr, "'return'"); break;
            case INT: strcpy(expectedStr, "'int'"); break;
            case VOID: strcpy(expectedStr, "'void'"); break;
            case ID: strcpy(expectedStr, "identificador"); break;
            case NUM: strcpy(expectedStr, "numero"); break;
            default: strcpy(expectedStr, "token desconhecido"); break;
        }
        
        /* Converte token encontrado para string */
        switch(token) {
            case SEMI: strcpy(foundStr, "';'"); break;
            case LPAREN: strcpy(foundStr, "'('"); break;
            case RPAREN: strcpy(foundStr, "')'"); break;
            case LBRACKET: strcpy(foundStr, "'['"); break;
            case RBRACKET: strcpy(foundStr, "']'"); break;
            case LBRACE: strcpy(foundStr, "'{'"); break;
            case RBRACE: strcpy(foundStr, "'}'"); break;
            case ASSIGN: strcpy(foundStr, "'='"); break;
            case COMMA: strcpy(foundStr, "','"); break;
            case ID: sprintf(foundStr, "'%s'", tokenString); break;
            case NUM: sprintf(foundStr, "'%s'", tokenString); break;
            case ENDFILE: strcpy(foundStr, "fim de arquivo"); break;
            default: sprintf(foundStr, "'%s'", tokenString); break;
        }
        
        fprintf(listing, "\nERRO SINTATICO: token inesperado %s, esperado %s - LINHA: %d\n", 
                foundStr, expectedStr, lineno);
        Error = TRUE;
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
        fprintf(listing, "\nERRO SINTATICO: comando inesperado '%s' - LINHA: %d\n", 
                tokenString, lineno);
        Error = TRUE;
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

TreeNode *expression(void) {
    TreeNode *t = NULL;
    
    if (token == ID) {
        char *identifier = copyString(tokenString);
        
        /* Verifica se é input ou output SEM parênteses */
        if (strcmp(tokenString, "input") == 0 || strcmp(tokenString, "output") == 0) {
            match(ID);
            if (token != LPAREN) {
                fprintf(listing, "\nERRO SINTATICO: funcao '%s' requer parenteses '()' - LINHA: %d\n", 
                        identifier, lineno);
                Error = TRUE;
                /* Se vier ASSIGN, é tentativa de atribuição a função */
                if (token == ASSIGN) {
                    fprintf(listing, "\nERRO SEMANTICO: nao e possivel atribuir valor a funcao '%s' - LINHA: %d\n", 
                            identifier, lineno);
                    match(ASSIGN);
                    /* Consome expressão até ponto e vírgula */
                    while (token != SEMI && token != ENDFILE) {
                        token = getToken();
                    }
                }
                /* Cria nó dummy para continuar */
                t = newExpNode(IdK);
                t->attr.name = identifier;
                return t;
            }
            /* Chamada de função obrigatória */
            t = call(identifier);
            t = simple_expression(t);
            return t;
        }
        
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
            /* Chamada de função - DEVE TER PARÊNTESES */
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
            
            /* Verifica se é input ou output SEM parênteses */
            if (strcmp(tokenString, "input") == 0 || strcmp(tokenString, "output") == 0) {
                match(ID);
                if (token != LPAREN) {
                    fprintf(listing, "\nERRO SINTATICO: funcao '%s' requer parenteses '()' - LINHA: %d\n", 
                            identifier, lineno);
                    Error = TRUE;
                    /* Se vier ASSIGN, consome até o fim da linha */
                    if (token == ASSIGN) {
                        fprintf(listing, "\nERRO SEMANTICO: nao e possivel atribuir valor a funcao '%s' - LINHA: %d\n", 
                                identifier, lineno);
                        while (token != SEMI && token != ENDFILE) {
                            token = getToken();
                        }
                    }
                    /* Cria nó dummy para continuar */
                    t = newExpNode(IdK);
                    t->attr.name = identifier;
                    return t;
                }
                /* Chamada de função obrigatória */
                t = call(identifier);
                return t;
            }
            
            match(ID);
            
            if (token == LPAREN) {
                /* Chamada de função */
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
        fprintf(listing, "\nERRO SINTATICO: token inesperado '%s' - LINHA: %d\n", 
                tokenString, lineno);
        Error = TRUE;
        token = getToken();
        break;
    }
    
    return t;
}

TreeNode *call(char *identifier) {
    TreeNode *t = newStmtNode(CallK);
    t->attr.name = identifier;
    match(LPAREN); /* OBRIGATÓRIO TER LPAREN */
    
    /* Verifica se output tem argumentos */
    if (strcmp(identifier, "output") == 0 && token == RPAREN) {
        fprintf(listing, "\nERRO SEMANTICO: funcao 'output' requer um argumento - LINHA: %d\n", lineno);
        Error = TRUE;
    }
    
    t->child[0] = args();
    match(RPAREN); /* OBRIGATÓRIO TER RPAREN */
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
        fprintf(listing, "\nERRO SINTATICO: fim de arquivo esperado - LINHA: %d\n", lineno);
    return t;
}