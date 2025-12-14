/*
 * Scanner (Analisador Léxico) para C-
 */

#include "globals.h"
#include "util.h"
#include "scan.h"

typedef enum {
    START, INCOMMENT, INNUM, INID, INASSIGN, INNE, INLE, INGE, DONE
} StateType;

char tokenString[MAXTOKENLEN + 1];

#define BUFLEN 256
static char lineBuf[BUFLEN];
static int linepos = 0;
static int bufsize = 0;
static int EOF_flag = FALSE;

/* Para rastrear comentários não fechados */
static int commentStartLine = 0;

static struct {
    char *str;
    TokenType tok;
} reservedWords[MAXRESERVED] = {
    {"else", ELSE}, {"if", IF}, {"int", INT},
    {"return", RETURN}, {"void", VOID}, {"while", WHILE}
};

static int getNextChar(void) {
    if (!(linepos < bufsize)) {
        lineno++;
        if (fgets(lineBuf, BUFLEN - 1, source)) {
            if (EchoSource) fprintf(listing, "%4d: %s", lineno, lineBuf);
            bufsize = strlen(lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        } else {
            EOF_flag = TRUE;
            return EOF;
        }
    } else
        return lineBuf[linepos++];
}

static void ungetNextChar(void) {
    if (!EOF_flag) linepos--;
}

static TokenType reservedLookup(char *s) {
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return reservedWords[i].tok;
    return ID;
}

TokenType getToken(void) {
    int tokenStringIndex = 0;
    TokenType currentToken;
    StateType state = START;
    int save;
    
    while (state != DONE) {
        int c = getNextChar();
        save = TRUE;
        
        switch (state) {
        case START:
            if (isdigit(c))
                state = INNUM;
            else if (isalpha(c))
                state = INID;
            else if (c == '=')
                state = INASSIGN;
            else if (c == '!') 
                state = INNE;
            else if (c == '<')
                state = INLE;
            else if (c == '>')
                state = INGE;
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
                save = FALSE;
            else if (c == '/') {
                save = FALSE;
                int c2 = getNextChar();
                if (c2 == '*') {
                    state = INCOMMENT;
                    commentStartLine = lineno; /* Salva linha onde comentário começou */
                } else {
                    ungetNextChar();
                    state = DONE;
                    currentToken = OVER;
                }
            } else {
                state = DONE;
                switch (c) {
                case EOF:
                    save = FALSE;
                    currentToken = ENDFILE;
                    break;
                case '+':
                    currentToken = PLUS;
                    break;
                case '-':
                    currentToken = MINUS;
                    break;
                case '*':
                    currentToken = TIMES;
                    break;
                case '(':
                    currentToken = LPAREN;
                    break;
                case ')':
                    currentToken = RPAREN;
                    break;
                case ';':
                    currentToken = SEMI;
                    break;
                case ',':
                    currentToken = COMMA;
                    break;
                case '[':
                    currentToken = LBRACKET;
                    break;
                case ']':
                    currentToken = RBRACKET;
                    break;
                case '{':
                    currentToken = LBRACE;
                    break;
                case '}':
                    currentToken = RBRACE;
                    break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;
            
        case INCOMMENT:
            save = FALSE;
            if (c == EOF) {
                /* ERRO: Comentário não fechado - chegou ao fim do arquivo */
                state = DONE;
                currentToken = ENDFILE;
                fprintf(listing, "\nERRO LEXICO: Comentario nao fechado iniciado na linha %d\n", 
                        commentStartLine);
                Error = TRUE;
            } else if (c == '*') {
                int c2 = getNextChar();
                if (c2 == '/') {
                    state = START; /* Comentário fechado corretamente */
                } else if (c2 == EOF) {
                    /* ERRO: EOF dentro de possível fechamento de comentário */
                    state = DONE;
                    currentToken = ENDFILE;
                    fprintf(listing, "\nERRO LEXICO: Comentario nao fechado iniciado na linha %d\n", 
                            commentStartLine);
                    Error = TRUE;
                } else {
                    ungetNextChar();
                }
            }
            break;
            
        case INASSIGN:
            state = DONE;
            if (c == '=')
                currentToken = EQ;
            else {
                ungetNextChar();
                save = FALSE;
                currentToken = ASSIGN;
            }
            break;
            
        case INNE:
            state = DONE;
            if (c == '=')
                currentToken = NE;
            else {
                ungetNextChar();
                save = FALSE;
                currentToken = ERROR;
            }
            break;
            
        case INLE:
            state = DONE;
            if (c == '=')
                currentToken = LE;
            else {
                ungetNextChar();
                save = FALSE;
                currentToken = LT;
            }
            break;
            
        case INGE:
            state = DONE;
            if (c == '=')
                currentToken = GE;
            else {
                ungetNextChar();
                save = FALSE;
                currentToken = GT;
            }
            break;
            
        case INNUM:
            if (!isdigit(c)) {
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = NUM;
            }
            break;
            
        case INID:
            if (!isalpha(c)) {
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = ID;
            }
            break;
            
        case DONE:
        default:
            fprintf(listing, "ERRO LEXICO: estado invalido\n");
            state = DONE;
            currentToken = ERROR;
            break;
        }
        
        if ((save) && (tokenStringIndex <= MAXTOKENLEN))
            tokenString[tokenStringIndex++] = (char)c;
        if (state == DONE) {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }
    
    if (TraceScan) {
        fprintf(listing, "\t%d: ", lineno);
        printToken(currentToken, tokenString);
    }
    
    return currentToken;
}