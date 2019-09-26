#include "socket.h"
#include "thread.h"
#include "event.h"
#include <string>
static Event listen_ev;
using namespace std;


void handleRead(void *rhs)
{
    int fd = reinterpret_cast<long>(rhs);
    LOG_DEBUG("handle read \n");
    char buf[1024];
    co_read(fd, buf,1024);
    close(fd);
    LOG_DEBUG("recv_buf=%s\n",buf);
    
}


void handleAccept(void * rhs) 
{
    
    int fd = reinterpret_cast<long>(rhs);
    LOG_DEBUG("listen_fd=%d", fd);
    while(1)
    {   
        co_struct* read_co;
        struct sockaddr_in raddr;
        socklen_t rsz = sizeof(raddr);
        int cfd = co_accept(fd, (struct sockaddr *) &raddr, &rsz);
        exit_if(cfd < 0, "accept failed");
        co_create(read_co, handleRead, (void*)(size_t)cfd);
        LOG_DEBUG("read_co=%d", read_co->co_id);
    }


}


int main()
{   
    co_init();
    co_struct *listen_co;
    ListenSocket sockfd;
    sockfd.create(9898,"0.0.0.0");
    LOG_DEBUG("listen_fd=%d", sockfd.get_fd());
    co_create(listen_co, handleAccept, (void*)(size_t)sockfd.get_fd());
    schedule();
    
    return 0;
}