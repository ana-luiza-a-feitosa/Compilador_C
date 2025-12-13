#ifndef SCAN_H
#define SCAN_H

#include "globals.h"  // ← DEVE TER ESTA LINHA!

#define MAXTOKENLEN 40

extern char tokenString[MAXTOKENLEN + 1];

TokenType getToken(void);

#endif