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
#define SOCK_TIMEOUT 5000
const int EVENT_NONE = 0x0;
const int EVENT_READ = 0x1;
const int EVENT_WRITE = 0x4;
const int EVENT_ERROR = 0x8; //not need to add
const int EVENT_RDHUP = 0x2000; //equal to EPOLLRDHUP
const int EVENT_ET = 1u << 31; 
const int EVENT_ONESHOT = 1u << 30; 
const int EVENT_TIMEOUT = 1u << 16; 



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


