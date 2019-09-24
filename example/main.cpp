#include "socket.h"
#include "thread.h"
#include "event.h"
#include <string>
static Epoll_event  ep_ev;
static Event listen_ev;
using namespace std;


void bad_request(int client)
{
    char buf[1024];
 
    /*回应客户端错误的 HTTP 请求 */
    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Your browser sent a bad request, ");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(client, buf, sizeof(buf), 0);
}


void handleRead(void *rhs)
{
    printf("handle read \n");
    struct epoll_event  * ep = (struct epoll_event  *)rhs;
    int n;
    string msg;
    char buf[1024];
    while((n = read(ep->data.fd,buf, sizeof(buf))) > 0)
    {
        msg.append(buf, n);
    }
    LOG_DEBUG("read msg=%s", msg.c_str());
    bad_request(ep->data.fd);
    close(ep->data.fd);
    if(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        LOG_DEBUG("read %d error: %d %s\n", ep->data.fd, errno, strerror(errno));
         return;
    }
       

    if (n < 0)  
        LOG_DEBUG("read %d error: %d %s\n", ep->data.fd, errno, strerror(errno));

    epoll_ctl(ep_ev.epoll_fd, EPOLL_CTL_DEL , ep->data.fd, ep);
    close(ep->data.fd);
}

void handleWrite(void *rhs)
{

}


void handleAccept(void * rhs) 
{
    struct epoll_event  * ep = (struct epoll_event  *)rhs;
    int fd = ep->data.fd;
    struct sockaddr_in raddr;
    socklen_t rsz = sizeof(raddr);
    int cfd = accept(fd, (struct sockaddr *) &raddr, &rsz);
    exit_if(cfd < 0, "accept failed");
    Socket  sock;
    sock.fd = cfd;
    sock.setnonblock();
    Event   ev;
    ev.init_event(&sock, EPOLLIN, EPOLL_CTL_ADD);
    ep_ev.updateEvent(&ev);

    sockaddr_in peer, local;
    socklen_t alen = sizeof(peer);
    int r = getpeername(cfd, (sockaddr *) &peer, &alen);
    exit_if(r < 0, "getpeername failed");
    printf("accept a connection from %s\n", inet_ntoa(raddr.sin_addr));
}



void handle_event(Epoll_event * ev)
{
    for(int i = 0; i< ev->active_num; ++i)
    {
        co_struct * event_co;
        int fd = ev->active_ev[i].data.fd;
        int events = ev->active_ev[i].events;

        if (events & (EPOLLIN | EPOLLERR))
        {
            if(fd == listen_ev.epoll_ev.data.fd)
            {   LOG_DEBUG("now accept a request");
                co_create(event_co, handleAccept, &ev->active_ev[i]);
                
            }   
            else
                co_create(event_co, handleRead, &ev->active_ev[i]);
        }
        else if (events & EPOLLOUT)
        {
            co_create(event_co, handleWrite, &ev->active_ev[i]);
        } 
        else 
        {
            exit_if(1, "unknown event");
        }


        
        
    }
}

void loop_run()
{
    
    
 for(;;)
    {
        ep_ev.wait_event();
        LOG_DEBUG("wait callback");
        handle_event(&ep_ev);
        schedule();
        
    }
}




int main()
{   co_init();
    ListenSocket sockfd;
    sockfd.create(9898,"0.0.0.0");
    ep_ev.create(20, 10000);
    listen_ev.init_event(&sockfd, EPOLLIN, EPOLL_CTL_ADD);
    ep_ev.updateEvent(&listen_ev);
    loop_run();
    

    return 0;
}