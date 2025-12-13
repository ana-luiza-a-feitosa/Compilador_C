#include "globals.h"

#define SIZE 211
#define SHIFT 4
#define MAX_SCOPE 100

static struct BucketListRec *hashTable[SIZE];
static char *currentScope = "global";
static char scopeStack[MAX_SCOPE][50];
static int scopeStackTop = 0;

static int hash(char *key) {
    int temp = 0;
    for (int i = 0; key[i] != '\0'; i++)
        temp = ((temp << SHIFT) + key[i]) % SIZE;
    return temp;
}

void st_insert(char *name, int lineno, int loc, ExpType type, int arraySize) {
    int h = hash(name);
    BucketList l = hashTable[h];
    
    while (l != NULL && (strcmp(name, l->name) != 0 || strcmp(currentScope, l->scope) != 0))
        l = l->next;
    
    if (l == NULL) {
        l = (BucketList)malloc(sizeof(struct BucketListRec));
        l->name = strdup(name);
        l->lines = (LineList)malloc(sizeof(struct LineListRec));
        l->lines->lineno = lineno;
        l->lines->next = NULL;
        l->memloc = loc;
        l->type = type;
        l->arraySize = arraySize;
        l->scope = strdup(currentScope);
        l->next = hashTable[h];
        hashTable[h] = l;
    } else {
        LineList t = l->lines;
        while (t->next != NULL) t = t->next;
        t->next = (LineList)malloc(sizeof(struct LineListRec));
        t->next->lineno = lineno;
        t->next->next = NULL;
    }
}

BucketList st_lookup(char *name) {
    int h = hash(name);
    BucketList l = hashTable[h];
    
    for (int i = scopeStackTop; i >= 0; i--) {
        l = hashTable[h];
        while (l != NULL) {
            if (strcmp(name, l->name) == 0 && strcmp(scopeStack[i], l->scope) == 0)
                return l;
            l = l->next;
        }
    }
    return NULL;
}

BucketList st_lookup_current(char *name) {
    int h = hash(name);
    BucketList l = hashTable[h];
    
    while (l != NULL) {
        if (strcmp(name, l->name) == 0 && strcmp(currentScope, l->scope) == 0)
            return l;
        l = l->next;
    }
    return NULL;
}

void st_push_scope(char *scope) {
    if (scopeStackTop < MAX_SCOPE - 1) {
        strcpy(scopeStack[++scopeStackTop], scope);
        currentScope = scopeStack[scopeStackTop];
    }
}

void st_pop_scope(void) {
    if (scopeStackTop > 0) {
        currentScope = scopeStack[--scopeStackTop];
    }
}

void st_set_scope(char *scope) {
    currentScope = scope;
}

char *st_get_scope(void) {
    return currentScope;
}

void printSymTab(FILE *listing) {
    fprintf(listing, "\n=== TABELA DE SIMBOLOS ===\n");
    fprintf(listing, "%-15s %-10s %-15s %-10s %-10s %s\n",
            "Nome", "Tipo", "Escopo", "Localizacao", "Tam Array", "Linhas");
    fprintf(listing, "----------------------------------------------------------------------\n");
    
    for (int i = 0; i < SIZE; i++) {
        BucketList l = hashTable[i];
        while (l != NULL) {
            LineList t = l->lines;
            fprintf(listing, "%-15s ", l->name);
            
            switch (l->type) {
            case Void: fprintf(listing, "%-10s ", "void"); break;
            case Integer: fprintf(listing, "%-10s ", "int"); break;
            case Array: fprintf(listing, "%-10s ", "array"); break;
            case Function: fprintf(listing, "%-10s ", "function"); break;
            }
            
            fprintf(listing, "%-15s %-10d %-10d ",
                    l->scope, l->memloc, l->arraySize);
            
            while (t != NULL) {
                fprintf(listing, "%d ", t->lineno);
                t = t->next;
            }
            fprintf(listing, "\n");
            l = l->next;
        }
    }
}