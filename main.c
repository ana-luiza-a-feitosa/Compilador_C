#include "ast.h"
#include "symbol_table.h"
#include "semantic_analyzer.h"
#include "code_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variáveis globais definidas em outros módulos
extern FILE *yyin;
extern TreeNode *root_ast;
extern int yyparse(void);
extern int yylineno; // Usada pelo Flex

// Variável para rastrear erros
int Error = 0;

// Função principal
int main(int argc, char *argv[]) {
    FILE *input_file;
    
    // 1. Verificação de argumentos
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nome_do_arquivo_cminus>\n", argv[0]);
        exit(1);
    }

    // 2. Abertura do arquivo de entrada
    input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", argv[1]);
        exit(1);
    }

    yyin = input_file;

    fprintf(stdout, "--- Compilador C- Iniciado ---\n\n");
    
    // 3. Fase Léxica e Sintática (Scanner & Parser)
    fprintf(stdout, "## 1. Analisador Sintático e Construcao da AST\n");
    if (yyparse() == 0 && root_ast != NULL) {
        fprintf(stdout, "Analise Sintatica CONCLUIDA com sucesso.\n");
    } else {
        fprintf(stderr, "Analise Sintatica FALHOU.\n");
        Error = 1;
    }

    if (!Error) {
        // 4. Fase Semântica
        fprintf(stdout, "\n## 2. Analisador Semantico\n");
        semantic_analysis(root_ast);

        if (Error) {
            fprintf(stderr, "Compilacao interrompida devido a ERROS SEMANTICOS.\n");
        } else {
            fprintf(stdout, "Analise Semantica CONCLUIDA com sucesso.\n");

            // 5. Geração de Código Intermediário
            fprintf(stdout, "\n## 3. Gerador de Codigo Intermediario (3 Enderecos)\n");
            generate_code(root_ast);
            fprintf(stdout, "Geracao de Codigo CONCLUIDA com sucesso.\n");

            // 6. Impressão dos Entregáveis (Saídas Requisitadas)
            fprintf(stdout, "\n========================================\n");
            fprintf(stdout, "      ENTREGAVEIS FINAIS DO COMPILADOR      \n");
            fprintf(stdout, "========================================\n\n");
            
            // Requisito 1: Tabela de Símbolos
            fprintf(stdout, "### A. Tabela de Simbolos Completa\n");
            print_symbol_table(); // Chama a função de impressão da TS

            // Requisito 2: AST
            fprintf(stdout, "\n### B. Arvore Sintatica Abstrata (AST)\n");
            printAST(root_ast, 0); // Chama a função de impressão da AST

            // Requisito 3: Código Intermediário
            fprintf(stdout, "\n### C. Codigo Intermediario (3 Enderecos)\n");
            print_tac(); // Chama a função de impressão do TAC
            
            fprintf(stdout, "\n--- Compilacao Finalizada com Sucesso ---\n");
        }
    }

    fclose(input_file);
    return Error;
}

// A função yyerror é redefinida aqui (o bison chama esta função em caso de erro)
void yyerror(const char *s) {
    // Melhor recuperação possível: o Bison tenta recuperar sozinho, aqui apenas exibimos a mensagem exata
    fprintf(stderr, "ERRO SINTATICO: token inesperado proximo a '%s' (esperado '...') LINHA: %d\n", yytext, yylineno);
    Error = 1;
}