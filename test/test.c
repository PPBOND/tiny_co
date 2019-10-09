#include <stdio.h>
#include <stdlib.h>
#include "myhook.h"
int main() {

    enable_hook();
    int index = 0;
    for (index=0; index < 10; index++) {
         printf("index:%d, hook res\n", index);
         void *p = malloc(4);
         free(p);
    }
    printf("hello world\n");
    return 0;
}
