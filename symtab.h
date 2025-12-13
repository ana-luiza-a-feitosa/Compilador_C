#ifndef SYMTAB_H
#define SYMTAB_H

#include "globals.h"  // ← DEVE TER ESTA LINHA!

void st_insert(char *name, int lineno, int loc, ExpType type, char *scope);
int st_lookup(char *name);
void printSymTab(FILE *listing);

#endif