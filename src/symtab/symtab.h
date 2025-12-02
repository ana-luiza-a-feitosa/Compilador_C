#ifndef SYMTAB_H
#define SYMTAB_H

#include "globals.h"

typedef enum {
    TYPE_INT,
    TYPE_VOID
} TypeKind;

typedef struct Symbol {
    char *name;
    TypeKind type;
    int escopo;      /* 0 = global, 1, 2... */
    int linhaDecl;
    struct Symbol *next;
} Symbol;

void symtab_init(void);
void symtab_enter_scope(void);
void symtab_leave_scope(void);

int      symtab_insert(const char *name, TypeKind type, int linha);
Symbol * symtab_lookup(const char *name);

#endif /* SYMTAB_H */
