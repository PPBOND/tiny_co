#include "socket.h"
#include <sys/socket.h>


void Socket::setnonblock()
{
    int flag = fcntl(fd,F_GETFL,0);
    exit_if(flag < 0,"getfl error");
    int ret = fcntl(fd,F_SETFL, flag | O_NONBLOCK);
    exit_if( ret < 0, "setnonblock error");


}

void ListenSocket::create(int port, const char *ip)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    exit_if(fd < 0 , "create error");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    
    int val =1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val))<0) {
        perror("setsockopt()");         
    }    

    bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr));
    listen(fd, 20);
    printf("listenfd=%d", fd);
    setnonblock();

}