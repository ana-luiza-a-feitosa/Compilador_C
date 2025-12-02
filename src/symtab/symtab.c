#include "symtab.h"

#define HASH_SIZE 211

static Symbol *tabela[HASH_SIZE];
static int escopoAtual = 0;

static int hash(const char *key) {
    int h = 0;
    while (*key) {
        h = (h << 4) + *key++;
        int g = h & 0xF0000000;
        if (g) h ^= g >> 24;
        h &= ~g;
    }
    return h % HASH_SIZE;
}

void symtab_init(void) {
    for (int i = 0; i < HASH_SIZE; i++) {
        tabela[i] = NULL;
    }
    escopoAtual = 0;
}

void symtab_enter_scope(void) {
    escopoAtual++;
}

void symtab_leave_scope(void) {
    /* remove símbolos do escopoAtual */
    for (int i = 0; i < HASH_SIZE; i++) {
        Symbol *prev = NULL;
        Symbol *cur  = tabela[i];
        while (cur) {
            if (cur->escopo == escopoAtual) {
                Symbol *tmp = cur;
                if (prev) prev->next = cur->next;
                else      tabela[i]  = cur->next;
                cur = cur->next;
                free(tmp->name);
                free(tmp);
            } else {
                prev = cur;
                cur  = cur->next;
            }
        }
    }
    if (escopoAtual > 0) escopoAtual--;
}

int symtab_insert(const char *name, TypeKind type, int linha) {
    int h = hash(name);
    Symbol *cur = tabela[h];
    while (cur) {
        if (cur->escopo == escopoAtual && strcmp(cur->name, name) == 0) {
            /* declaração duplicada */
            printf("ERRO SEMANTICO: identificador \"%s\" redeclarado - LINHA: %d\n",
                   name, linha);
            Error = 1;
            return 0;
        }
        cur = cur->next;
    }

    Symbol *s = (Symbol *)malloc(sizeof(Symbol));
    s->name = strdup(name);
    s->type = type;
    s->escopo = escopoAtual;
    s->linhaDecl = linha;
    s->next = tabela[h];
    tabela[h] = s;
    return 1;
}

Symbol *symtab_lookup(const char *name) {
    int h = hash(name);
    Symbol *cur = tabela[h];
    Symbol *found = NULL;
    int melhorEscopo = -1;

    while (cur) {
        if (strcmp(cur->name, name) == 0 && cur->escopo <= escopoAtual) {
            if (cur->escopo > melhorEscopo) {
                melhorEscopo = cur->escopo;
                found = cur;
            }
        }
        cur = cur->next;
    }
    return found;
}
