#include "scanner.h"
#include "globals.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* O Error deve existir em algum lugar (globals.c por ex). */
extern int Error;

/* ========= Fonte ========= */

int openSource(const char *filename) {
    source = fopen(filename, "r");
    return source != NULL;
}

void closeSource(void) {
    if (source) {
        fclose(source);
        source = NULL;
    }
}

/* ========= Controle de linha ========= */
static int lineno = 1;

static int nextChar(void) {
    if (!source) return EOF;
    return fgetc(source);
}

static void ungetChar(int c) {
    if (c != EOF && source) ungetc(c, source);
}

static int isLetterOnly(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static TokenType reservedLookup(const char *s) {
    if (strcmp(s, "else") == 0)   return TK_ELSE;
    if (strcmp(s, "if") == 0)     return TK_IF;
    if (strcmp(s, "int") == 0)    return TK_INT;
    if (strcmp(s, "return") == 0) return TK_RETURN;
    if (strcmp(s, "void") == 0)   return TK_VOID;
    if (strcmp(s, "while") == 0)  return TK_WHILE;

    /* input/output NÃO são palavras-chave no Louden: são IDs predefinidos */
    return TK_ID;
}

static void lexError(const char *lex, int line) {
    printf("ERRO LEXICO: \"%s\" - LINHA: %d\n", lex, line);
    Error = 1;
}

Token getToken(void) {
    Token tk;
    tk.lexema[0] = '\0';
    tk.linha = lineno;
    tk.type = TK_ERROR;

    int c;

    /* ========= 1) pula espaços em branco ========= */
    while (1) {
        c = nextChar();
        if (c == ' ' || c == '\t' || c == '\r') continue;
        if (c == '\n') { lineno++; continue; }
        break;
    }

    tk.linha = lineno;

    /* ========= 2) EOF ========= */
    if (c == EOF) {
        tk.type = TK_EOF;
        strcpy(tk.lexema, "EOF");
        return tk;
    }

    /* ========= 3) comentários do tipo C: /* ... *\/  ========= */
    if (c == '/') {
        int d = nextChar();
        if (d == '*') {
            int prev = 0;
            while (1) {
                int x = nextChar();
                if (x == EOF) {
                    lexError("comentario nao fechado", lineno);
                    tk.type = TK_ERROR;
                    strcpy(tk.lexema, "/*");
                    return tk;
                }
                if (x == '\n') lineno++;

                if (prev == '*' && x == '/') break;
                prev = x;
            }
            /* depois do comentário, pega o próximo token */
            return getToken();
        } else {
            /* '/' sozinho é operador de divisão */
            ungetChar(d);
            tk.type = TK_OVER;
            strcpy(tk.lexema, "/");
            return tk;
        }
    }

    /* ========= 4) ID = letra letra* (Louden puro) ========= */
    if (isLetterOnly(c)) {
        int k = 0;
        tk.lexema[k++] = (char)c;

        while (1) {
            int d = nextChar();
            if (isLetterOnly(d)) {
                if (k < (int)sizeof(tk.lexema) - 1)
                    tk.lexema[k++] = (char)d;
            } else {
                ungetChar(d);
                break;
            }
        }

        tk.lexema[k] = '\0';
        tk.type = reservedLookup(tk.lexema);
        return tk;
    }

    /* ========= 5) NUM = dígito dígito* ========= */
    if (isdigit((unsigned char)c)) {
        int k = 0;
        tk.lexema[k++] = (char)c;

        while (1) {
            int d = nextChar();
            if (isdigit((unsigned char)d)) {
                if (k < (int)sizeof(tk.lexema) - 1)
                    tk.lexema[k++] = (char)d;
            } else {
                ungetChar(d);
                break;
            }
        }

        tk.lexema[k] = '\0';
        tk.type = TK_NUM;
        return tk;
    }

    /* ========= 6) operadores e delimitadores ========= */
    switch (c) {
        case '+': tk.type = TK_PLUS; strcpy(tk.lexema, "+"); return tk;
        case '-': tk.type = TK_MINUS; strcpy(tk.lexema, "-"); return tk;
        case '*': tk.type = TK_TIMES; strcpy(tk.lexema, "*"); return tk;
        case ';': tk.type = TK_SEMI; strcpy(tk.lexema, ";"); return tk;
        case ',': tk.type = TK_COMMA; strcpy(tk.lexema, ","); return tk;
        case '(': tk.type = TK_LPAREN; strcpy(tk.lexema, "("); return tk;
        case ')': tk.type = TK_RPAREN; strcpy(tk.lexema, ")"); return tk;
        case '[': tk.type = TK_LBRACKET; strcpy(tk.lexema, "["); return tk;
        case ']': tk.type = TK_RBRACKET; strcpy(tk.lexema, "]"); return tk;
        case '{': tk.type = TK_LBRACE; strcpy(tk.lexema, "{"); return tk;
        case '}': tk.type = TK_RBRACE; strcpy(tk.lexema, "}"); return tk;

        case '=': {
            int d = nextChar();
            if (d == '=') { tk.type = TK_EQ; strcpy(tk.lexema, "=="); }
            else { ungetChar(d); tk.type = TK_ASSIGN; strcpy(tk.lexema, "="); }
            return tk;
        }

        case '!': {
            int d = nextChar();
            if (d == '=') { tk.type = TK_NE; strcpy(tk.lexema, "!="); return tk; }
            /* '!' sozinho NÃO existe no C- do Louden */
            ungetChar(d);
            tk.type = TK_ERROR;
            strcpy(tk.lexema, "!");
            lexError(tk.lexema, lineno);
            return tk;
        }

        case '<': {
            int d = nextChar();
            if (d == '=') { tk.type = TK_LE; strcpy(tk.lexema, "<="); }
            else { ungetChar(d); tk.type = TK_LT; strcpy(tk.lexema, "<"); }
            return tk;
        }

        case '>': {
            int d = nextChar();
            if (d == '=') { tk.type = TK_GE; strcpy(tk.lexema, ">="); }
            else { ungetChar(d); tk.type = TK_GT; strcpy(tk.lexema, ">"); }
            return tk;
        }

        default:
            tk.type = TK_ERROR;
            tk.lexema[0] = (char)c;
            tk.lexema[1] = '\0';
            lexError(tk.lexema, lineno);
            return tk;
    }
}
