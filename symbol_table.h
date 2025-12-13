#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include "ast.h"
#include <stdbool.h>

#define TABLE_SIZE 211 // Um número primo

// Estrutura do item na Tabela de Símbolos
typedef struct Symbol {
    char *name;
    ExpType type; 
    int scope_level; 
    // ... Informações adicionais (ex: se é função, se é parâmetro)
    struct Symbol *next; // Para tratamento de colisões (hash table)
} Symbol;

// Estrutura para um único escopo (bloco ou função)
typedef struct Scope {
    Symbol *hash_table[TABLE_SIZE]; // Hash table de entrada do escopo
    int level;                      // Nível de aninhamento
    struct Scope *parent;           // Ponteiro para o escopo pai (implementação de pilha)
    struct Scope *next;             // Próximo na lista de escopos
} Scope;

// Variáveis globais
extern Scope *current_scope; 
extern int scope_counter;

// Funções da Tabela de Símbolos
void enter_scope(); 
void leave_scope(); 
void insert_symbol(char *name, ExpType type, int lineno, bool is_func); // Para declaração [cite: 37]
Symbol *lookup_symbol_current_scope(char *name); // Busca no escopo atual (para checar duplicatas)
Symbol *lookup_symbol(char *name); // Busca do escopo atual para fora (para checar uso) [cite: 36]
void print_symbol_table(); // Imprimir a TS completa [cite: 49]

#endif