#include "socket.h"
#include "coroutine.h"
#include "event.h"
#include<string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


static Event listen_ev;
using namespace std;


void* handleTask(void *rhs)
{
    int fd = reinterpret_cast<long>(rhs);
    LOG_DEBUG("handle read \n");
    char buf[1024];
    read(fd, buf,1024);
    std::string httpRes;
    httpRes = "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 6\r\n\r\n123456";
    write(fd,httpRes.c_str(),httpRes.size());
    
    close(fd);
    LOG_DEBUG("recv_buf=%s\n",buf);
    return NULL;
    
}


void* handleAccept(void * rhs) 
{
    int fd = reinterpret_cast<long>(rhs);
    LOG_DEBUG("listen_fd=%d", fd);
    while(1)
    {   
        CoRoutine_t* read_co;
        struct sockaddr_in raddr;
        socklen_t rsz = sizeof(raddr);
        int cfd = accept(fd, (struct sockaddr *) &raddr, &rsz);
        exit_if(cfd < 0, "accept failed");
        co_create(read_co, handleTask, (void*)(size_t)cfd,0);
        LOG_DEBUG("read_co=%d", read_co->routine_id);
    } 
    
    return NULL;


}

void* child_process()
{
    co_init();
    CoRoutine_t *listen_co;
    ListenSocket sockfd;
    sockfd.create(9898,"0.0.0.0");
    LOG_DEBUG("listen_fd=%d", sockfd.get_fd());
    co_create(listen_co, handleAccept, (void*)(size_t)sockfd.get_fd(),0);
    event_loop_run();
    return NULL;
}



int main()
{   
   child_process();
    return 0;
}