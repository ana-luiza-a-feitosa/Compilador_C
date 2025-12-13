#ifndef SCANNER_H
#define SCANNER_H

#include "globals.h"

typedef enum {
  /* keywords */
  TK_ELSE, TK_IF, TK_INT, TK_RETURN, TK_VOID, TK_WHILE,

  /* multichar ops */
  TK_LE, TK_LT, TK_GE, TK_GT, TK_EQ, TK_NE,

  /* symbols */
  TK_ASSIGN,      /* = */
  TK_PLUS, TK_MINUS, TK_TIMES, TK_OVER,
  TK_SEMI, TK_COMMA,
  TK_LPAREN, TK_RPAREN,
  TK_LBRACKET, TK_RBRACKET,
  TK_LBRACE, TK_RBRACE,

  /* id, num, eof, error */
  TK_ID, TK_NUM,
  TK_EOF,
  TK_ERROR
} TokenType;

typedef struct {
  TokenType type;
  char lexema[128];
  int linha;
} Token;

Token getToken(void);
int openSource(const char *filename);
void closeSource(void);

#endif
