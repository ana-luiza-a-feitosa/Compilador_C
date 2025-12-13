#include "parser.h"

/* token corrente */
static Token tokenAtual;

/* ===== util ===== */

static void advance(void) { tokenAtual = getToken(); }

static void erroSint(const char *msg) {
    printf("ERRO SINTATICO: %s (token \"%s\") - LINHA: %d\n",
           msg, tokenAtual.lexema, tokenAtual.linha);
    Error = 1;
}

/* Consome token esperado, senão sinaliza erro e tenta recuperar avançando 1 token */
static void match(TokenType t) {
    if (tokenAtual.type == t) {
        advance();
    } else {
        erroSint("token inesperado");
        advance();
    }
}

static int isTypeSpec(TokenType t) {
    return (t == TK_INT || t == TK_VOID);
}

static int isRelop(TokenType t) {
    return (t == TK_LE || t == TK_LT || t == TK_GE || t == TK_GT || t == TK_EQ || t == TK_NE);
}

static int isStmtStart(TokenType t) {
    return (t == TK_LBRACE ||
            t == TK_IF ||
            t == TK_WHILE ||
            t == TK_RETURN ||
            t == TK_SEMI ||
            t == TK_LPAREN ||
            t == TK_ID ||
            t == TK_NUM);
}

/* ===== forward decls (BNF Louden) ===== */

static TreeNode *programa(void);
static TreeNode *declaracao_lista(void);
static TreeNode *declaracao(void);
static TreeNode *var_declaracao(TypeSpec tipo, char *name, int line);
static TreeNode *fun_declaracao(TypeSpec tipo, char *name, int line);
static TypeSpec  tipo_especificador(void);

static TreeNode *params(void);
static TreeNode *param_lista(void);
static TreeNode *param(void);

static TreeNode *composto_decl(void);
static TreeNode *local_declaracoes(void);
static TreeNode *statement_lista(void);
static TreeNode *statement(void);

static TreeNode *expressao_decl(void);
static TreeNode *selecao_decl(void);
static TreeNode *iteracao_decl(void);
static TreeNode *retorno_decl(void);

static TreeNode *expressao(void);
static TreeNode *simples_expressao(TreeNode *prefix); /* prefix pode ser NULL */
static TreeNode *soma_expressao(TreeNode *prefixTerm); /* prefixTerm pode ser NULL */
static TreeNode *termo(TreeNode *prefixFactor); /* prefixFactor pode ser NULL */
static TreeNode *fator(void);

static TreeNode *var_from_id(char *name, int line);  /* ID | ID[expr] */
static TreeNode *ativacao_from_id(char *name, int line); /* ID(args) */
static TreeNode *args(void);
static TreeNode *arg_lista(void);

/* ===== entry ===== */

TreeNode *parse(void) {
    advance();
    TreeNode *t = programa();
    if (tokenAtual.type != TK_EOF && tokenAtual.type != TK_ERROR) {
        erroSint("esperado EOF");
    }
    return t;
}

/* ===== grammar impl ===== */

/* 1) programa → declaração-lista */
static TreeNode *programa(void) {
    return declaracao_lista();
}

/* 2) declaração-lista → declaração-lista declaração | declaração
   Implementação: while houver tipo-especificador, parse declaração e encadeia em sibling */
static TreeNode *declaracao_lista(void) {
    TreeNode *head = NULL, *tail = NULL;

    while (isTypeSpec(tokenAtual.type)) {
        TreeNode *d = declaracao();
        if (!d) break;

        if (!head) head = d;
        else tail->sibling = d;

        tail = d;
        while (tail && tail->sibling) tail = tail->sibling;
    }

    return head;
}

/* 3) declaração → var-declaração | fun-declaração */
static TreeNode *declaracao(void) {
    TypeSpec tipo = tipo_especificador();

    if (tokenAtual.type != TK_ID) {
        erroSint("esperado ID apos tipo");
        return NULL;
    }

    char *name = strdup(tokenAtual.lexema);
    int line = tokenAtual.linha;
    match(TK_ID);

    if (tokenAtual.type == TK_LPAREN) {
        return fun_declaracao(tipo, name, line);
    }
    return var_declaracao(tipo, name, line);
}

/* 5) tipo-especificador → int | void */
static TypeSpec tipo_especificador(void) {
    if (tokenAtual.type == TK_INT) { match(TK_INT); return TY_INT; }
    if (tokenAtual.type == TK_VOID){ match(TK_VOID); return TY_VOID; }
    erroSint("esperado int ou void");
    return TY_INT;
}

/* 4) var-declaração → tipo ID ; | tipo ID [ NUM ] ; */
static TreeNode *var_declaracao(TypeSpec tipo, char *name, int line) {
    TreeNode *d = newDeclNode(DECL_VAR);
    d->type = tipo;
    d->name = name;
    d->lineno = line;
    d->arraySize = -1;

    if (tokenAtual.type == TK_LBRACKET) {
        match(TK_LBRACKET);
        if (tokenAtual.type != TK_NUM) {
            erroSint("esperado NUM no tamanho do array");
        } else {
            d->arraySize = atoi(tokenAtual.lexema);
            match(TK_NUM);
        }
        match(TK_RBRACKET);
    }

    match(TK_SEMI);
    return d;
}

/* 6) fun-declaração → tipo ID ( params ) composto-decl */
static TreeNode *fun_declaracao(TypeSpec tipo, char *name, int line) {
    TreeNode *f = newDeclNode(DECL_FUN);
    f->type = tipo;
    f->name = name;
    f->lineno = line;

    match(TK_LPAREN);
    f->child[0] = params();
    match(TK_RPAREN);

    f->child[1] = composto_decl();
    return f;
}

/* 7) params → param-lista | void */
static TreeNode *params(void) {
    if (tokenAtual.type == TK_VOID) {
        /* atenção: em Louden, "void" aqui significa "sem parâmetros" */
        match(TK_VOID);
        return NULL;
    }
    return param_lista();
}

/* 8) param-lista → param-lista , param | param */
static TreeNode *param_lista(void) {
    TreeNode *head = param();
    TreeNode *tail = head;

    while (tokenAtual.type == TK_COMMA) {
        match(TK_COMMA);
        TreeNode *p = param();
        if (tail) tail->sibling = p;
        else head = p;
        tail = p;
    }

    return head;
}

/* 9) param → tipo ID | tipo ID [ ] */
static TreeNode *param(void) {
    TypeSpec tipo = tipo_especificador();
    if (tokenAtual.type != TK_ID) {
        erroSint("esperado ID em parametro");
        return NULL;
    }

    TreeNode *p = newDeclNode(DECL_PARAM);
    p->type = tipo;
    p->name = strdup(tokenAtual.lexema);
    p->lineno = tokenAtual.linha;
    p->arraySize = -1;

    match(TK_ID);

    if (tokenAtual.type == TK_LBRACKET) {
        match(TK_LBRACKET);
        match(TK_RBRACKET);
        p->arraySize = 0; /* parâmetro array */
    }

    return p;
}

/* 10) composto-decl → { local-declarações statement-lista } */
static TreeNode *composto_decl(void) {
    TreeNode *c = newStmtNode(ST_COMPOUND);
    c->lineno = tokenAtual.linha;

    match(TK_LBRACE);
    c->child[0] = local_declaracoes();
    c->child[1] = statement_lista();
    match(TK_RBRACE);

    return c;
}

/* 11) local-declarações → local-declarações var-declaração | vazio */
static TreeNode *local_declaracoes(void) {
    TreeNode *head = NULL, *tail = NULL;

    while (isTypeSpec(tokenAtual.type)) {
        TypeSpec tipo = tipo_especificador();

        if (tokenAtual.type != TK_ID) {
            erroSint("esperado ID em declaracao local");
            break;
        }

        char *name = strdup(tokenAtual.lexema);
        int line = tokenAtual.linha;
        match(TK_ID);

        TreeNode *d = var_declaracao(tipo, name, line);

        if (!head) head = d;
        else tail->sibling = d;
        tail = d;

        while (tail && tail->sibling) tail = tail->sibling;
    }

    return head;
}

/* 12) statement-lista → statement-lista statement | vazio */
static TreeNode *statement_lista(void) {
    TreeNode *head = NULL, *tail = NULL;

    while (isStmtStart(tokenAtual.type)) {
        TreeNode *s = statement();
        if (!s) break;

        if (!head) head = s;
        else tail->sibling = s;

        tail = s;
        while (tail && tail->sibling) tail = tail->sibling;
    }

    return head;
}

/* 13) statement → expressão-decl | composto-decl | seleção-decl | iteração-decl | retorno-decl */
static TreeNode *statement(void) {
    if (tokenAtual.type == TK_LBRACE) return composto_decl();
    if (tokenAtual.type == TK_IF) return selecao_decl();
    if (tokenAtual.type == TK_WHILE) return iteracao_decl();
    if (tokenAtual.type == TK_RETURN) return retorno_decl();
    return expressao_decl();
}

/* 14) expressão-decl → expressão ; | ; */
static TreeNode *expressao_decl(void) {
    TreeNode *s = newStmtNode(ST_EXPR);
    s->lineno = tokenAtual.linha;

    if (tokenAtual.type == TK_SEMI) {
        match(TK_SEMI);
        return s;
    }

    s->child[0] = expressao();
    match(TK_SEMI);
    return s;
}

/* 15) seleção-decl → if ( expressão ) statement | if ( expressão ) statement else statement */
static TreeNode *selecao_decl(void) {
    TreeNode *s = newStmtNode(ST_IF);
    s->lineno = tokenAtual.linha;

    match(TK_IF);
    match(TK_LPAREN);
    s->child[0] = expressao();
    match(TK_RPAREN);

    s->child[1] = statement();

    if (tokenAtual.type == TK_ELSE) {
        match(TK_ELSE);
        s->child[2] = statement();
    }

    return s;
}

/* 16) iteração-decl → while ( expressão ) statement */
static TreeNode *iteracao_decl(void) {
    TreeNode *s = newStmtNode(ST_WHILE);
    s->lineno = tokenAtual.linha;

    match(TK_WHILE);
    match(TK_LPAREN);
    s->child[0] = expressao();
    match(TK_RPAREN);

    s->child[1] = statement();
    return s;
}

/* 17) retorno-decl → return ; | return expressão ; */
static TreeNode *retorno_decl(void) {
    TreeNode *s = newStmtNode(ST_RETURN);
    s->lineno = tokenAtual.linha;

    match(TK_RETURN);

    if (tokenAtual.type == TK_SEMI) {
        match(TK_SEMI);
        return s;
    }

    s->child[0] = expressao();
    match(TK_SEMI);
    return s;
}

/* 18) expressão → var = expressão | simples-expressão
   Implementação robusta:
   - Se começa com ID, pode ser:
       a) atribuição (var '=' ...)
       b) expressão normal iniciando com ID (x + y, x < y, call(...), etc.)
   - Fazemos parsing do "prefixo" baseado no ID e depois decidimos.
*/
static TreeNode *expressao(void) {
    if (tokenAtual.type == TK_ID) {
        /* capturar ID */
        char *name = strdup(tokenAtual.lexema);
        int line = tokenAtual.linha;
        match(TK_ID);

        /* decidir se é call, var simples ou var indexada */
        TreeNode *prefix = NULL;
        if (tokenAtual.type == TK_LPAREN) {
            prefix = ativacao_from_id(name, line);  /* EX_CALL */
            /* call não pode ser l-value para atribuição */
            return simples_expressao(prefix);
        } else {
            prefix = var_from_id(name, line);       /* EX_ID ou EX_INDEX */
            /* se vier '=' então é atribuição */
            if (tokenAtual.type == TK_ASSIGN) {
                TreeNode *a = newExprNode(EX_ASSIGN);
                a->lineno = tokenAtual.linha;
                a->child[0] = prefix;
                match(TK_ASSIGN);
                a->child[1] = expressao();
                return a;
            }
            /* senão é uma simples-expressão começando por um prefixo já lido */
            return simples_expressao(prefix);
        }
    }

    /* caso geral (não começa com ID) */
    return simples_expressao(NULL);
}

/* 20) simples-expressão → soma-expressão relacional soma-expressão | soma-expressão */
static TreeNode *simples_expressao(TreeNode *prefix) {
    /* soma-expressão usa termo/fator. Se prefix != NULL, ele deve ser um fator inicial. */
    TreeNode *left = NULL;

    if (prefix) {
        /* prefix é um "fator" inicial. monta termo/soma a partir dele */
        left = soma_expressao(prefix);
    } else {
        left = soma_expressao(NULL);
    }

    if (isRelop(tokenAtual.type)) {
        TreeNode *op = newExprNode(EX_OP);
        op->op = tokenAtual.type;
        op->lineno = tokenAtual.linha;
        match(tokenAtual.type);

        op->child[0] = left;
        op->child[1] = soma_expressao(NULL);
        return op;
    }

    return left;
}

/* 22-23) soma-expressão → soma-expressão soma termo | termo ; soma → + | - */
static TreeNode *soma_expressao(TreeNode *prefixFactor) {
    TreeNode *t = termo(prefixFactor);

    while (tokenAtual.type == TK_PLUS || tokenAtual.type == TK_MINUS) {
        TreeNode *op = newExprNode(EX_OP);
        op->op = tokenAtual.type;
        op->lineno = tokenAtual.linha;
        match(tokenAtual.type);

        op->child[0] = t;
        op->child[1] = termo(NULL);
        t = op;
    }

    return t;
}

/* 24-25) termo → termo mult fator | fator ; mult → * | / */
static TreeNode *termo(TreeNode *prefixFactor) {
    TreeNode *t = NULL;

    if (prefixFactor) t = prefixFactor;
    else t = fator();

    while (tokenAtual.type == TK_TIMES || tokenAtual.type == TK_OVER) {
        TreeNode *op = newExprNode(EX_OP);
        op->op = tokenAtual.type;
        op->lineno = tokenAtual.linha;
        match(tokenAtual.type);

        op->child[0] = t;
        op->child[1] = fator();
        t = op;
    }

    return t;
}

/* 26) fator → ( expressão ) | var | ativação | NUM */
static TreeNode *fator(void) {
    if (tokenAtual.type == TK_LPAREN) {
        match(TK_LPAREN);
        TreeNode *e = expressao();
        match(TK_RPAREN);
        return e;
    }

    if (tokenAtual.type == TK_NUM) {
        TreeNode *c = newExprNode(EX_CONST);
        c->val = atoi(tokenAtual.lexema);
        c->lineno = tokenAtual.linha;
        match(TK_NUM);
        return c;
    }

    if (tokenAtual.type == TK_ID) {
        /* ID pode virar var ou ativação */
        char *name = strdup(tokenAtual.lexema);
        int line = tokenAtual.linha;
        match(TK_ID);

        if (tokenAtual.type == TK_LPAREN) {
            return ativacao_from_id(name, line);
        }
        return var_from_id(name, line);
    }

    erroSint("esperado fator");
    advance();
    return NULL;
}

/* 19) var → ID | ID [ expressão ] */
static TreeNode *var_from_id(char *name, int line) {
    if (tokenAtual.type == TK_LBRACKET) {
        TreeNode *idx = newExprNode(EX_INDEX);
        idx->name = name;
        idx->lineno = line;

        match(TK_LBRACKET);
        idx->child[0] = expressao();
        match(TK_RBRACKET);
        return idx;
    }

    TreeNode *id = newExprNode(EX_ID);
    id->name = name;
    id->lineno = line;
    return id;
}

/* 27) ativação → ID ( args ) */
static TreeNode *ativacao_from_id(char *name, int line) {
    TreeNode *c = newExprNode(EX_CALL);
    c->name = name;
    c->lineno = line;

    match(TK_LPAREN);
    c->child[0] = args();
    match(TK_RPAREN);

    return c;
}

/* 28) args → arg-lista | vazio */
static TreeNode *args(void) {
    if (tokenAtual.type == TK_RPAREN) return NULL;
    return arg_lista();
}

/* 29) arg-lista → arg-lista , expressão | expressão */
static TreeNode *arg_lista(void) {
    TreeNode *head = expressao();
    TreeNode *tail = head;

    while (tokenAtual.type == TK_COMMA) {
        match(TK_COMMA);
        TreeNode *e = expressao();
        if (tail) tail->sibling = e;
        else head = e;
        tail = e;
    }

    return head;
}
