#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include "myhook.h"


static int count = 0;

void *malloc(size_t size) {
    printf("call mymalloc\n");
    count++;
	return NULL;
}

int enable_hook() {
    return count;
}
