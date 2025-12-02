#ifndef SCANNER_H
#define SCANNER_H

#include "globals.h"

/* Tipos de token da linguagem C- (ajustem se precisarem de mais) */
typedef enum {
    TK_IF, TK_ELSE, TK_WHILE, TK_RETURN,
    TK_INT, TK_VOID,
    TK_INPUT, TK_OUTPUT,
    TK_ID, TK_NUM,
    TK_PLUS, TK_MINUS, TK_TIMES, TK_OVER,
    TK_LT, TK_GT, TK_EQ, TK_ASSIGN,
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE,
    TK_SEMI, TK_COMMA,
    TK_EOF,
    TK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char      lexema[64];
    int       linha;
} Token;

/* Abre o arquivo fonte .c- */
void abrirFonte(const char *nomeArquivo);

/* Retorna o próximo token */
Token getToken(void);

/* Consulta a linha atual (útil para AST/erros) */
int getLinhaAtual(void);

#endif /* SCANNER_H */
