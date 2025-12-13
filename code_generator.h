// code_generator.h
#ifndef _CODE_GENERATOR_H_
#define _CODE_GENERATOR_H_

#include "ast.h"
#include "symbol_table.h"

// Estrutura de uma instrução de 3 endereços
typedef struct ThreeAddressCode {
    char *op;       // Operação (ex: "+", "=", "goto")
    char *arg1;     // Argumento 1 (operandos ou labels)
    char *arg2;     // Argumento 2
    char *result;   // Resultado (variável temporária ou destino)
    struct ThreeAddressCode *next;
} TAC;

extern TAC *code_head; // Lista ligada das instruções
extern int temp_counter; // Contador para variáveis temporárias (t1, t2...)
extern int label_counter; // Contador para labels (L1, L2...)

char *new_temp();
char *new_label();
void emit_tac(char *op, char *arg1, char *arg2, char *result);
void generate_code(TreeNode *tree); // Travessia da AST para geração de código [cite: 43]
void print_tac(); // Imprimir o código intermediário [cite: 51]

#endif