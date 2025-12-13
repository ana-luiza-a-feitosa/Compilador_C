#include <stdio.h>
#include "runtime.h"

int input(void) {
    int x;
    scanf("%d", &x);
    return x;
}

void output(int x) {
    printf("%d\n", x);
}
