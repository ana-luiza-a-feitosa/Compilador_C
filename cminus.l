%option noyywrap
%option yylineno
%{
#include "cminus.tab.h" // Arquivo gerado pelo Bison
#include <stdio.h>
#include <string.h>

// Variável para rastrear a linha atual (yylineno é automático)
extern int yylineno; 

// Função de erro léxico [cite: 21, 22]
void yyerror(const char *s); 
void lexical_error(char *lexeme) {
    fprintf(stderr, "ERRO LEXICO: '%s' LINHA: %d\n", lexeme, yylineno);
    // exit(1); // Interromper a compilação [cite: 21]
}

%}

// Definições Regulares (Regex)
DIGIT [0-9]
LETTER [a-zA-Z]
ID {LETTER}({LETTER}|{DIGIT})*
NUM {DIGIT}+

%%

// Palavras-chave [cite: 20]
"else"      { return ELSE; }
"if"        { return IF; }
"int"       { return INT; }
"return"    { return RETURN; }
"void"      { return VOID; }
"while"     { return WHILE; }
"input"     { return INPUT; } 
"output"    { return OUTPUT; } 

// Números
{NUM}       { yylval.val = atoi(yytext); return NUM; }

// Identificadores [cite: 20]
{ID}        { yylval.name = strdup(yytext); return ID; }

// Operadores e Delimitadores [cite: 20]
"+"         { yylval.op = strdup(yytext); return PLUS; }
"-"         { yylval.op = strdup(yytext); return MINUS; }
"*"         { yylval.op = strdup(yytext); return TIMES; }
"/"         { yylval.op = strdup(yytext); return OVER; }

"<="        { yylval.op = strdup(yytext); return LE; }
"<"         { yylval.op = strdup(yytext); return LT; }
">="        { yylval.op = strdup(yytext); return GE; }
">"         { yylval.op = strdup(yytext); return GT; }
"=="        { yylval.op = strdup(yytext); return EQ; }
"!="        { yylval.op = strdup(yytext); return NE; }
"="         { return ASSIGN; }

";"         { return SEMI; }
","         { return COMMA; }
"("         { return LPAREN; }
")"         { return RPAREN; }
"["         { return LBRACKET; }
"]"         { return RBRACKET; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }

// Comentários C-style (ignorar)
"/*"([^*]|\*+[^*/])*\*+"/"  { /* ignora */ }

// Espaços em branco (ignorar)
[ \t\r]+    { /* ignora */ }

// Nova linha (yylineno atualiza automaticamente)
\n          { /* ignora */ }

// Erro Léxico
.           { lexical_error(yytext); }

%%