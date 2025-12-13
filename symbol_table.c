#include "symbol_table.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variáveis globais
Scope *current_scope = NULL; 
int scope_counter = 0;

// Implementação simples de Hash Function (djb2)
static unsigned int hash(char *key) {
    unsigned int hash_val = 5381;
    int c;
    while ((c = *key++)) {
        hash_val = ((hash_val << 5) + hash_val) + c; // hash * 33 + c
    }
    return hash_val % TABLE_SIZE;
}

// Função para entrar em um novo escopo (empilha)
void enter_scope() {
    Scope *new_scope = (Scope *)malloc(sizeof(Scope));
    if (new_scope == NULL) {
        fprintf(stderr, "Erro de alocacao para novo escopo.\n");
        exit(1);
    }
    
    // Inicializa a Hash Table
    for (int i = 0; i < TABLE_SIZE; i++) {
        new_scope->hash_table[i] = NULL;
    }
    
    scope_counter++;
    new_scope->level = scope_counter;
    new_scope->parent = current_scope; // O novo escopo aponta para o anterior (pai)
    current_scope = new_scope; // Atualiza o topo da pilha
    
    // Opcional: Imprimir evento de escopo
    // fprintf(stdout, "-> Entrou no Escopo %d\n", current_scope->level);
}

// Função para sair do escopo atual (desempilha)
void leave_scope() {
    if (current_scope == NULL) return; // Não deveria acontecer
    
    Scope *old_scope = current_scope;
    current_scope = current_scope->parent; // Volta para o escopo pai
    
    // Opcional: Liberar memória dos símbolos no escopo (não implementado para simplificar)
    // free(old_scope); 
    // fprintf(stdout, "<- Saiu do Escopo %d\n", old_scope->level);
}

// Insere um novo símbolo no escopo atual
void insert_symbol(char *name, ExpType type, int lineno, bool is_func) {
    if (current_scope == NULL) {
        // Garantir que haja um escopo global antes de inserir
        enter_scope(); 
    }
    
    unsigned int h = hash(name);
    
    // Cria novo nó de símbolo
    Symbol *new_symbol = (Symbol *)malloc(sizeof(Symbol));
    if (new_symbol == NULL) {
        fprintf(stderr, "Erro de alocacao para novo simbolo.\n");
        exit(1);
    }

    new_symbol->name = strdup(name);
    new_symbol->type = type;
    new_symbol->scope_level = current_scope->level;
    
    // Insere no início da lista encadeada (tratamento de colisão)
    new_symbol->next = current_scope->hash_table[h];
    current_scope->hash_table[h] = new_symbol;
}

// Procura um símbolo *somente* no escopo atual (útil para verificar duplicatas)
Symbol *lookup_symbol_current_scope(char *name) {
    if (current_scope == NULL) return NULL;
    
    unsigned int h = hash(name);
    Symbol *s = current_scope->hash_table[h];
    
    while (s != NULL) {
        if (strcmp(s->name, name) == 0) {
            return s;
        }
        s = s->next;
    }
    return NULL;
}

// Procura um símbolo do escopo atual para fora (útil para verificar uso)
Symbol *lookup_symbol(char *name) {
    Scope *scope_ptr = current_scope;
    
    while (scope_ptr != NULL) {
        unsigned int h = hash(name);
        Symbol *s = scope_ptr->hash_table[h];
        
        while (s != NULL) {
            if (strcmp(s->name, name) == 0) {
                return s;
            }
            s = s->next;
        }
        scope_ptr = scope_ptr->parent; // Sobe para o escopo pai
    }
    return NULL;
}

// Função auxiliar para converter ExpType em string
static char *type_to_string(ExpType t) {
    switch (t) {
        case VOID: return "void";
        case INTEGER: return "int";
        case ARRAY: return "int[]";
        case FUNCTION: return "func";
        default: return "unknown";
    }
}

// Função para imprimir a Tabela de Símbolos completa
void print_symbol_table() {
    // Precisamos percorrer todos os escopos que já existiram
    // (Esta implementação simples só consegue imprimir o escopo atual e seus pais)
    
    Scope *scope_ptr = current_scope;
    
    while (scope_ptr != NULL) {
        fprintf(stdout, "\nNivel de Escopo: %d\n", scope_ptr->level);
        fprintf(stdout, "----------------------------------\n");
        fprintf(stdout, "Nome\t\tTipo\t\tEscopo\n");
        fprintf(stdout, "----------------------------------\n");

        for (int i = 0; i < TABLE_SIZE; i++) {
            Symbol *s = scope_ptr->hash_table[i];
            while (s != NULL) {
                fprintf(stdout, "%s\t\t%s\t\t%d\n", 
                        s->name, 
                        type_to_string(s->type), 
                        s->scope_level);
                s = s->next;
            }
        }
        scope_ptr = scope_ptr->parent;
    }
    fprintf(stdout, "----------------------------------\n");
}