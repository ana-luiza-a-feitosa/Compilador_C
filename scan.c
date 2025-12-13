#include "globals.h"

typedef enum {
    START, INNUM, INID, INCOMMENT, INASSIGN, INLT, INGT, INNE, DONE
} StateType;

char tokenString[MAX_TOKEN_LEN + 1];
static char lineBuf[256];
static int linepos = 0;
static int bufsize = 0;
static int EOF_flag = 0;

struct {
    char *str;
    TokenType tok;
} reservedWords[MAX_RESERVED] = {
    {"if", IF}, {"else", ELSE}, {"while", WHILE}, {"return", RETURN},
    {"int", INT}, {"void", VOID}, {"input", INPUT}, {"output", OUTPUT}
};

static int getNextChar(void) {
    if (linepos >= bufsize) {
        lineno++;
        if (fgets(lineBuf, 255, source)) {
            bufsize = strlen(lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        } else {
            EOF_flag = 1;
            return EOF;
        }
    }
    return lineBuf[linepos++];
}

static void ungetNextChar(void) {
    if (!EOF_flag) linepos--;
}

static TokenType reservedLookup(char *s) {
    for (int i = 0; i < MAX_RESERVED; i++)
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
        save = 1;

        switch (state) {
        case START:
            if (isdigit(c))
                state = INNUM;
            else if (isalpha(c))
                state = INID;
            else if (c == '=')
                state = INASSIGN;
            else if (c == '<')
                state = INLT;
            else if (c == '>')
                state = INGT;
            else if (c == '!')
                state = INNE;
            else if (c == '/') {
                save = 0;
                int next = getNextChar();
                if (next == '*') {
                    state = INCOMMENT;
                } else {
                    ungetNextChar();
                    state = DONE;
                    currentToken = OVER;
                }
            } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                save = 0;
            else {
                state = DONE;
                switch (c) {
                case EOF: save = 0; currentToken = ENDFILE; break;
                case '+': currentToken = PLUS; break;
                case '-': currentToken = MINUS; break;
                case '*': currentToken = TIMES; break;
                case '(': currentToken = LPAREN; break;
                case ')': currentToken = RPAREN; break;
                case '[': currentToken = LBRACKET; break;
                case ']': currentToken = RBRACKET; break;
                case '{': currentToken = LBRACE; break;
                case '}': currentToken = RBRACE; break;
                case ';': currentToken = SEMI; break;
                case ',': currentToken = COMMA; break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;
        case INCOMMENT:
            save = 0;
            if (c == EOF) {
                state = DONE;
                currentToken = ENDFILE;
            } else if (c == '*') {
                int next = getNextChar();
                if (next == '/')
                    state = START;
                else
                    ungetNextChar();
            }
            break;
        case INASSIGN:
            state = DONE;
            if (c == '=')
                currentToken = EQ;
            else {
                ungetNextChar();
                save = 0;
                currentToken = ASSIGN;
            }
            break;
        case INLT:
            state = DONE;
            if (c == '=')
                currentToken = LE;
            else {
                ungetNextChar();
                save = 0;
                currentToken = LT;
            }
            break;
        case INGT:
            state = DONE;
            if (c == '=')
                currentToken = GE;
            else {
                ungetNextChar();
                save = 0;
                currentToken = GT;
            }
            break;
        case INNE:
            state = DONE;
            if (c == '=')
                currentToken = NE;
            else {
                currentToken = ERROR;
            }
            break;
        case INNUM:
            if (!isdigit(c)) {
                ungetNextChar();
                save = 0;
                state = DONE;
                currentToken = NUM;
            }
            break;
        case INID:
            if (!isalnum(c)) {
                ungetNextChar();
                save = 0;
                state = DONE;
                currentToken = ID;
            }
            break;
        case DONE:
        default:
            state = DONE;
            currentToken = ERROR;
            break;
        }

        if (save && tokenStringIndex < MAX_TOKEN_LEN)
            tokenString[tokenStringIndex++] = (char)c;
        
        if (state == DONE) {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }

    return currentToken;
}