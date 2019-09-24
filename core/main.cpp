#include "socket.h"
#include "thread.h"
#include "event.h"
#include <string>
static Event listen_ev;
using namespace std;


void handleRead(void *rhs)
{
    int fd = (int ) rhs;
    LOG_DEBUG("handle read \n");
    char buf[1024];
    co_recv(fd, buf,1024);
    close(fd);
    LOG_DEBUG("recv_buf=%s\n",buf);
    
}

void handleWrite(void *rhs)
{

}


void handleAccept(void * rhs) 
{
    
    co_struct *event_co;
    int fd = (int) rhs;
    
    LOG_DEBUG("listen_fd=%d", fd);

    while(1)
    {
        struct sockaddr_in raddr;
        socklen_t rsz = sizeof(raddr);
        int cfd = co_accept(fd, (struct sockaddr *) &raddr, &rsz);
        exit_if(cfd < 0, "accept failed");
        sockaddr_in peer, local;
        socklen_t alen = sizeof(peer);
        int r = getpeername(cfd, (sockaddr *) &peer, &alen);
        exit_if(r < 0, "getpeername failed");
        LOG_DEBUG("accept a connection from %s\n", inet_ntoa(raddr.sin_addr));
        co_create(event_co, handleRead, (void*)cfd);
    }


}



int main()
{   
    co_struct *event_co;
    co_init();
    ListenSocket sockfd;
    sockfd.create(9898,"0.0.0.0");
    LOG_DEBUG("listen_fd=%d", sockfd.get_fd());
    co_create(event_co, handleAccept, (void*)sockfd.fd);
   // env.ev_manger.updateEvent(&listen_ev);
    schedule();
    

    return 0;
}