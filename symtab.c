/*
 * Tabela de Símbolos para C-
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "symtab.h"

#define SIZE 211
#define SHIFT 4

static int hash(char *key) {
    int temp = 0;
    int i = 0;
    while (key[i] != '\0') {
        temp = ((temp << SHIFT) + key[i]) % SIZE;
        ++i;
    }
    return temp;
}

typedef struct LineListRec {
    int lineno;
    struct LineListRec *next;
} *LineList;

typedef struct BucketListRec {
    char *name;
    LineList lines;
    int memloc;
    ExpType type;
    char *scope;
    struct BucketListRec *next;
} *BucketList;

static BucketList hashTable[SIZE];

void st_insert(char *name, int lineno, int loc, ExpType type, char *scope) {
    int h = hash(name);
    BucketList l = hashTable[h];
    
    while ((l != NULL) && (strcmp(name, l->name) != 0)) l = l->next;
    
    if (l == NULL) {
        l = (BucketList)malloc(sizeof(struct BucketListRec));
        l->name = name;
        l->lines = (LineList)malloc(sizeof(struct LineListRec));
        l->lines->lineno = lineno;
        l->memloc = loc;
        l->type = type;
        l->scope = scope;
        l->lines->next = NULL;
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

int st_lookup(char *name) {
    int h = hash(name);
    BucketList l = hashTable[h];
    while ((l != NULL) && (strcmp(name, l->name) != 0)) l = l->next;
    if (l == NULL) return -1;
    else return l->memloc;
}

void printSymTab(FILE *listing) {
    int i;
    fprintf(listing, "Nome        Escopo      Tipo        Linha\n");
    fprintf(listing, "----------- ----------- ----------- -----\n");
    
    for (i = 0; i < SIZE; ++i) {
        if (hashTable[i] != NULL) {
            BucketList l = hashTable[i];
            while (l != NULL) {
                LineList t = l->lines;
                fprintf(listing, "%-11s %-11s ", l->name, l->scope);
                
                switch (l->type) {
                case Integer: fprintf(listing, "%-11s ", "int"); break;
                case Void: fprintf(listing, "%-11s ", "void"); break;
                case IntegerArray: fprintf(listing, "%-11s ", "int[]"); break;
                default: fprintf(listing, "%-11s ", "?"); break;
                }
                
                while (t != NULL) {
                    fprintf(listing, "%4d ", t->lineno);
                    t = t->next;
                }
                fprintf(listing, "\n");
                l = l->next;
            }
        }
    }
}
