#pragma once
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <queue>


#define  CO_OK 0
#define  CO_FAIL -1

#define ONCE_EXEC  0
#define CYCLE_EXEC 1




#define exit_if(r,...)                                                                           \
    if (r) {                                                                                     \
        printf(__VA_ARGS__);                                                                     \
        printf("%s:%d error no: %d error msg %s\n", __FILE__, __LINE__, errno, strerror(errno)); \
        exit(1);                                                                                 \
    }


#define return_if(r,...)                                                                           \
    if (r) {                                                                                     \
        printf(__VA_ARGS__);                                                                     \
        printf("%s:%d error no: %d error msg %s\n", __FILE__, __LINE__, errno, strerror(errno)); \
        return -1;                                                                                 \
    }


