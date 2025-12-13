%{
#include "ast.h"
#include "symbol_table.h"
#include "semantic_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylineno;
extern char *yytext;
extern TreeNode *root_ast;

// Declaração da função de erro sintático (implementada em main.c)
void yyerror(const char *s);

// Variável para controle de erros
extern int Error;
%}

// Definições da união de tipos
%union {
    int val;
    char *name;
    char *op;
    TreeNode *node;
    ExpType type;
    int array_size;
}

// TOKENs com tipos (vindos do cminus.l)
%token <val> NUM
%token <name> ID
%token <op> PLUS MINUS TIMES OVER
%token <op> LT LE GT GE EQ NE 
%token ASSIGN SEMI COMMA LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE 
%token ELSE IF INT RETURN VOID WHILE INPUT OUTPUT

// Não-terminais com tipos (referências à AST)
%type <node> program declaration_list declaration var_declaration fun_declaration 
%type <node> params param_list param compound_stmt statement_list 
%type <node> statement expression_stmt selection_stmt iteration_stmt return_stmt 
%type <node> expression simple_expression additive_expression term factor call args arg_list
%type <node> var

// Outros tipos
%type <type> type_specifier 

// Precedência de operadores (para expressões)
%left PLUS MINUS
%left TIMES OVER
%right ASSIGN

%%

/* ------------------- 1. Programação ------------------- */

program : declaration_list { root_ast = $1; };

declaration_list : declaration_list declaration { 
                    // Encadear como irmãos (sibling)
                    $$ = $1; 
                    if ($1 != NULL) {
                        TreeNode *temp = $1;
                        while (temp->sibling != NULL) temp = temp->sibling;
                        temp->sibling = $2;
                    } else {
                        $$ = $2;
                    }
                 } 
                 | declaration { $$ = $1; };

declaration : var_declaration { $$ = $1; }
            | fun_declaration { $$ = $1; };

/* ------------------- 2. Declarações ------------------- */

var_declaration : type_specifier ID SEMI { 
                    $$ = newDeclNode(VarDeclK, $1, $2);
                    $$->lineno = yylineno;
                 }
                | type_specifier ID LBRACKET NUM RBRACKET SEMI { 
                    $$ = newDeclNode(ArrayDeclK, $1, $2); 
                    $$->lineno = yylineno;
                    
                    // O tamanho do array como filho (ConstK)
                    $$->child[0] = newExpNode(ConstK, INTEGER);
                    $$->child[0]->val = $4;
                    $$->child[0]->lineno = yylineno;
                };

fun_declaration : type_specifier ID LPAREN params RPAREN compound_stmt { 
                    $$ = newDeclNode(FuncDeclK, $1, $2);
                    $$->lineno = yylineno;
                    $$->child[0] = $4; // Parâmetros
                    $$->child[1] = $6; // Corpo da função
                };

type_specifier : INT { $$ = INTEGER; }
               | VOID { $$ = VOID; };

/* ------------------- 3. Parâmetros e Argumentos ------------------- */

params : param_list { $$ = $1; }
       | VOID { $$ = NULL; }; // Lista de parâmetros vazia

param_list : param_list COMMA param { 
                $$ = $1;
                TreeNode *temp = $1;
                while (temp->sibling != NULL) temp = temp->sibling;
                temp->sibling = $3;
           }
           | param { $$ = $1; };

param : type_specifier ID { 
            $$ = newNode(ParamK);
            $$->kind.param = ParamDeclK;
            $$->type = $1;
            $$->name = strdup($2);
            $$->lineno = yylineno;
        }
      | type_specifier ID LBRACKET RBRACKET { 
            $$ = newNode(ParamK);
            $$->kind.param = ParamDeclK;
            $$->type = ARRAY; // Passagem de array é tratada como array
            $$->name = strdup($2);
            $$->lineno = yylineno;
        };

args : arg_list { $$ = $1; }
     | /* vazio */ { $$ = NULL; };

arg_list : arg_list COMMA expression { 
            $$ = $1;
            TreeNode *temp = $1;
            while (temp->sibling != NULL) temp = temp->sibling;
            temp->sibling = $3;
         }
         | expression { $$ = $1; };

/* ------------------- 4. Comandos (Statements) ------------------- */

compound_stmt : LBRACE { enter_scope(); } declaration_list statement_list RBRACE { 
    $$ = newStmtNode(CompK);
    $$->lineno = yylineno;
    $$->child[0] = $3; // Declarações Locais
    $$->child[1] = $4; // Lista de Comandos
    leave_scope(); 
};

statement_list : statement_list statement { 
                    $$ = $1;
                    if ($1 != NULL) {
                        TreeNode *temp = $1;
                        while (temp->sibling != NULL) temp = temp->sibling;
                        temp->sibling = $2;
                    } else {
                        $$ = $2;
                    }
                 }
               | /* vazio */ { $$ = NULL; };

statement : expression_stmt 
          | compound_stmt
          | selection_stmt
          | iteration_stmt
          | return_stmt ;

expression_stmt : expression SEMI { $$ = $1; }
                | SEMI { $$ = newStmtNode(NullK); $$->lineno = yylineno; }; // Comando nulo

selection_stmt : IF LPAREN expression RPAREN statement ELSE statement {
                    $$ = newStmtNode(IfK);
                    $$->lineno = yylineno;
                    $$->child[0] = $3; // Condição
                    $$->child[1] = $5; // Bloco THEN
                    $$->child[2] = $7; // Bloco ELSE
                }
               | IF LPAREN expression RPAREN statement %prec ELSE { // %prec ELSE resolve o dangling else
                    $$ = newStmtNode(IfK);
                    $$->lineno = yylineno;
                    $$->child[0] = $3; // Condição
                    $$->child[1] = $5; // Bloco THEN
                    $$->child[2] = newStmtNode(NullK); // Bloco ELSE Nulo
                };

iteration_stmt : WHILE LPAREN expression RPAREN statement {
                    $$ = newStmtNode(WhileK);
                    $$->lineno = yylineno;
                    $$->child[0] = $3; // Condição
                    $$->child[1] = $5; // Corpo
                };

return_stmt : RETURN expression SEMI {
                $$ = newStmtNode(RetK);
                $$->lineno = yylineno;
                $$->child[0] = $2; // Valor de retorno
            }
            | RETURN SEMI {
                $$ = newStmtNode(RetK);
                $$->lineno = yylineno;
            };

/* ------------------- 5. Expressões ------------------- */

expression : var ASSIGN expression { 
                $$ = newStmtNode(AssignK);
                $$->lineno = yylineno;
                $$->child[0] = $1; // Variável (ID ou ID[expr])
                $$->child[1] = $3; // Expressão
            }
           | simple_expression { $$ = $1; };

simple_expression : additive_expression LT additive_expression { 
                        $$ = newExpNode(OpK, INTEGER); $$->op = strdup("<");
                        $$->lineno = yylineno;
                        $$->child[0] = $1; $$->child[1] = $3;
                    }
                    | additive_expression LE additive_expression {
                        $$ = newExpNode(OpK, INTEGER); $$->op = strdup("<=");
                        $$->lineno = yylineno;
                        $$->child[0] = $1; $$->child[1] = $3;
                    }
                    | additive_expression GT additive_expression {
                        $$ = newExpNode(OpK, INTEGER); $$->op = strdup(">");
                        $$->lineno = yylineno;
                        $$->child[0] = $1; $$->child[1] = $3;
                    }
                    | additive_expression GE additive_expression {
                        $$ = newExpNode(OpK, INTEGER); $$->op = strdup(">=");
                        $$->lineno = yylineno;
                        $$->child[0] = $1; $$->child[1] = $3;
                    }
                    | additive_expression EQ additive_expression {
                        $$ = newExpNode(OpK, INTEGER); $$->op = strdup("==");
                        $$->lineno = yylineno;
                        $$->child[0] = $1; $$->child[1] = $3;
                    }
                    | additive_expression NE additive_expression {
                        $$ = newExpNode(OpK, INTEGER); $$->op = strdup("!=");
                        $$->lineno = yylineno;
                        $$->child[0] = $1; $$->child[1] = $3;
                    }
                  | additive_expression { $$ = $1; };

additive_expression : additive_expression PLUS term { 
                        $$ = newExpNode(OpK, INTEGER); $$->op = strdup("+");
                        $$->lineno = yylineno;
                        $$->child[0] = $1; $$->child[1] = $3;
                    }
                    | additive_expression MINUS term {
                        $$ = newExpNode(OpK, INTEGER); $$->op = strdup("-");
                        $$->lineno = yylineno;
                        $$->child[0] = $1; $$->child[1] = $3;
                    }
                    | term { $$ = $1; };

term : term TIMES factor {
        $$ = newExpNode(OpK, INTEGER); $$->op = strdup("*");
        $$->lineno = yylineno;
        $$->child[0] = $1; $$->child[1] = $3;
    }
    | term OVER factor {
        $$ = newExpNode(OpK, INTEGER); $$->op = strdup("/");
        $$->lineno = yylineno;
        $$->child[0] = $1; $$->child[1] = $3;
    }
    | factor { $$ = $1; };

factor : LPAREN expression RPAREN { $$ = $2; }
       | call { $$ = $1; }
       | var { $$ = $1; }
       | NUM {
            $$ = newExpNode(ConstK, INTEGER);
            $$->lineno = yylineno;
            $$->val = $1;
        };

var : ID LBRACKET expression RBRACKET { // Acesso a array
        $$ = newExpNode(ArrayK, UNKNOWN);
        $$->lineno = yylineno;
        $$->name = strdup($1);
        $$->child[0] = $3; // Índice
    }
    | ID { 
        $$ = newExpNode(IdK, UNKNOWN); 
        $$->lineno = yylineno;
        $$->name = strdup($1); 
    };

call : ID LPAREN args RPAREN {
        $$ = newStmtNode(CallK);
        $$->lineno = yylineno;
        $$->name = strdup($1);
        $$->child[0] = $3; // Argumentos
    }
    | INPUT LPAREN RPAREN {
        $$ = newStmtNode(InputK);
        $$->lineno = yylineno;
    }
    | OUTPUT LPAREN expression RPAREN {
        $$ = newStmtNode(OutputK);
        $$->lineno = yylineno;
        $$->child[0] = $3;
    };

%%