#pragma once
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "comm_tools.h"

class Socket
{
public:
    Socket(){}
  
    void setnonblock();

    int fd;
   
};

class ListenSocket: public Socket
{
public:
    ListenSocket()
    {
        memset(&addr, 0, sizeof(addr));
    }
    int  get_fd() { return fd;}
    void create(int port, const char *ip);
    struct sockaddr_in addr;


     
};