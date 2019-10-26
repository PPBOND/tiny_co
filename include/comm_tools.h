#pragma once
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <queue>
#define _aDEBUG_
#ifdef  _DEBUG_

#define LOG_DEBUG(fmt,...) \
printf("[DEBUG co_id=%d:%s:%d:%s] " fmt"\n",get_current()->co_id,__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#endif

#ifndef _DEBUG_
#define LOG_DEBUG(fmt,...) ;
#endif


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


