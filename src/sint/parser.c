#include "parser.h"

/* token corrente do parser */
static Token tokenAtual;

/* protótipos internos baseados na gramática reduzida */
static void match(TokenType esperado);
static void advance(void);

static TreeNode *programa(void);
static TreeNode *stmt_list(void);
static TreeNode *stmt(void);
static TreeNode *assign_stmt(void);
static TreeNode *expression(void);
static TreeNode *term(void);
static TreeNode *factor(void);

/* Obtém o próximo token do scanner */
static void advance(void) {
    tokenAtual = getToken();
}

/* Emite erro sintático */
static void erro_sintatico(const char *msg) {
    printf("ERRO SINTATICO: %s (token \"%s\") - LINHA: %d\n",
           msg, tokenAtual.lexema, tokenAtual.linha);
    Error = 1;
}

/* Consome o token esperado ou sinaliza erro sintático */
static void match(TokenType esperado) {
    if (tokenAtual.type == esperado) {
        advance();
    } else {
        erro_sintatico("token inesperado");
        /* estratégia simples: tenta avançar mesmo assim */
        advance();
    }
}

/* fator → NUM | ID | '(' expr ')' */
static TreeNode *factor(void) {
    TreeNode *t = NULL;

    switch (tokenAtual.type) {
    case TK_NUM: {
        t = newExprNode(EX_CONST);
        t->val = atoi(tokenAtual.lexema);
        match(TK_NUM);
        break;
    }
    case TK_ID: {
        t = newExprNode(EX_ID);
        t->name = strdup(tokenAtual.lexema);
        match(TK_ID);
        break;
    }
    case TK_LPAREN: {
        match(TK_LPAREN);
        t = expression();
        match(TK_RPAREN);
        break;
    }
    default:
        erro_sintatico("esperado NUM, ID ou '('");
        advance();
        break;
    }

    return t;
}

/* termo → fator { (*|/) fator } */
static TreeNode *term(void) {
    TreeNode *t = factor();

    while (tokenAtual.type == TK_TIMES || tokenAtual.type == TK_OVER) {
        TreeNode *p = newExprNode(EX_OP);
        p->op = tokenAtual.type;
        match(tokenAtual.type);        /* consome * ou / */
        p->child[0] = t;
        p->child[1] = factor();
        t = p;
    }

    return t;
}

/* expr → termo { (+|-) termo } */
static TreeNode *expression(void) {
    TreeNode *t = term();

    while (tokenAtual.type == TK_PLUS || tokenAtual.type == TK_MINUS) {
        TreeNode *p = newExprNode(EX_OP);
        p->op = tokenAtual.type;
        match(tokenAtual.type);        /* consome + ou - */
        p->child[0] = t;
        p->child[1] = term();
        t = p;
    }

    return t;
}

/* atribuição → ID '=' expr */
static TreeNode *assign_stmt(void) {
    /* lado esquerdo (ID) */
    if (tokenAtual.type != TK_ID) {
        erro_sintatico("esperado identificador no inicio da atribuicao");
        return NULL;
    }

    TreeNode *idNode = newExprNode(EX_ID);
    idNode->name = strdup(tokenAtual.lexema);
    match(TK_ID);

    match(TK_ASSIGN);

    TreeNode *rhs = expression();

    TreeNode *assignNode = newStmtNode(ST_ASSIGN);
    assignNode->child[0] = idNode; /* lhs */
    assignNode->child[1] = rhs;    /* rhs */

    return assignNode;
}

/* stmt → atribuição ';' */
static TreeNode *stmt(void) {
    TreeNode *s = assign_stmt();
    match(TK_SEMI);
    return s;
}

/* stmt_list → { stmt } */
static TreeNode *stmt_list(void) {
    TreeNode *head = NULL;
    TreeNode *tail = NULL;

    /* nesta gramática, todo comando começa com ID */
    while (tokenAtual.type == TK_ID) {
        TreeNode *s = stmt();
        if (s == NULL) break;

        if (head == NULL) {
            head = s;
        } else {
            tail->sibling = s;
        }
        tail = s;
    }

    return head;
}

/* programa → stmt_list EOF */
static TreeNode *programa(void) {
    TreeNode *t = stmt_list();

    if (tokenAtual.type != TK_EOF && tokenAtual.type != TK_ERROR) {
        erro_sintatico("esperado EOF ao final do programa");
    }

    return t;
}

/* ponto de entrada externo */
TreeNode *parse(void) {
    advance();           /* lê primeiro token */
    TreeNode *t = programa();
    return t;
}
