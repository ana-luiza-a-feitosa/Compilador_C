#include "scanner.h"

static FILE *source = NULL;

typedef struct {
    const char *lex;
    TokenType  type;
} Keyword;

static Keyword keywords[] = {
    { "if",     TK_IF },
    { "else",   TK_ELSE },
    { "while",  TK_WHILE },
    { "return", TK_RETURN },
    { "int",    TK_INT },
    { "void",   TK_VOID },
    { "input",  TK_INPUT },
    { "output", TK_OUTPUT },
    { NULL,     TK_ID } /* sentinela */
};

void abrirFonte(const char *nomeArquivo) {
    source = fopen(nomeArquivo, "r");
    if (!source) {
        fprintf(stderr, "Nao foi possivel abrir o arquivo %s\n", nomeArquivo);
        exit(EXIT_FAILURE);
    }
    lineno = 1;
}

/* devolve o último char lido para o fluxo */
static void ungetch(int c) {
    if (c != EOF) {
        ungetc(c, source);
    }
}

int getLinhaAtual(void) {
    return lineno;
}

static TokenType palavraChaveOuID(char *lex) {
    for (int i = 0; keywords[i].lex != NULL; i++) {
        if (strcmp(lex, keywords[i].lex) == 0) {
            return keywords[i].type;
        }
    }
    return TK_ID;
}

Token getToken(void) {
    Token tk;
    tk.lexema[0] = '\0';
    tk.linha = lineno;
    tk.type  = TK_EOF;

    if (!source) {
        fprintf(stderr, "Fonte nao aberta em abrirFonte().\n");
        exit(EXIT_FAILURE);
    }

    int c;

    /* ignora espaços e quebras de linha */
    while ((c = fgetc(source)) != EOF) {
        if (c == ' ' || c == '\t' || c == '\r') {
            continue;
        } else if (c == '\n') {
            lineno++;
        } else {
            break;
        }
    }

    tk.linha = lineno;

    if (c == EOF) {
        tk.type = TK_EOF;
        strcpy(tk.lexema, "EOF");
        return tk;
    }

    /* identificadores ou palavras-chave */
    if (isalpha(c) || c == '_') {
        int i = 0;
        tk.lexema[i++] = (char)c;
        while ((c = fgetc(source)) != EOF &&
               (isalnum(c) || c == '_')) {
            if (i < (int)sizeof(tk.lexema) - 1) {
                tk.lexema[i++] = (char)c;
            }
        }
        tk.lexema[i] = '\0';
        ungetch(c);

        tk.type = palavraChaveOuID(tk.lexema);
        return tk;
    }

    /* números inteiros */
    if (isdigit(c)) {
        int i = 0;
        tk.lexema[i++] = (char)c;
        while ((c = fgetc(source)) != EOF && isdigit(c)) {
            if (i < (int)sizeof(tk.lexema) - 1) {
                tk.lexema[i++] = (char)c;
            }
        }
        tk.lexema[i] = '\0';
        ungetch(c);

        tk.type = TK_NUM;
        return tk;
    }

    /* operadores e símbolos simples / compostos */
    switch (c) {
    case '+':
        tk.type = TK_PLUS;  strcpy(tk.lexema, "+");  break;
    case '-':
        tk.type = TK_MINUS; strcpy(tk.lexema, "-");  break;
    case '*':
        tk.type = TK_TIMES; strcpy(tk.lexema, "*");  break;
    case '/': {
        int next = fgetc(source);
        if (next == '/') { /* comentário de linha */
            while ((c = fgetc(source)) != EOF && c != '\n') {}
            if (c == '\n') lineno++;
            return getToken();
        } else if (next == '*') { /* comentário de bloco */
            int prev = 0;
            while ((c = fgetc(source)) != EOF) {
                if (c == '\n') lineno++;
                if (prev == '*' && c == '/') break;
                prev = c;
            }
            return getToken();
        } else {
            ungetch(next);
            tk.type = TK_OVER;
            strcpy(tk.lexema, "/");
        }
        break;
    }
    case '<':
        tk.type = TK_LT;    strcpy(tk.lexema, "<");  break;
    case '>':
        tk.type = TK_GT;    strcpy(tk.lexema, ">");  break;
    case '=': {
        int next = fgetc(source);
        if (next == '=') {
            tk.type = TK_EQ;
            strcpy(tk.lexema, "==");
        } else {
            ungetch(next);
            tk.type = TK_ASSIGN;
            strcpy(tk.lexema, "=");
        }
        break;
    }
    case '(':
        tk.type = TK_LPAREN;  strcpy(tk.lexema, "(");  break;
    case ')':
        tk.type = TK_RPAREN;  strcpy(tk.lexema, ")");  break;
    case '{':
        tk.type = TK_LBRACE;  strcpy(tk.lexema, "{");  break;
    case '}':
        tk.type = TK_RBRACE;  strcpy(tk.lexema, "}");  break;
    case ';':
        tk.type = TK_SEMI;    strcpy(tk.lexema, ";");  break;
    case ',':
        tk.type = TK_COMMA;   strcpy(tk.lexema, ",");  break;
    default:
        tk.type = TK_ERROR;
        tk.lexema[0] = (char)c;
        tk.lexema[1] = '\0';
        printf("ERRO LEXICO: \"%s\" - LINHA: %d\n", tk.lexema, tk.linha);
        Error = 1;
        break;
    }

    return tk;
}
