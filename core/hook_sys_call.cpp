#include "thread.h"
#include <dlfcn.h>





typedef ssize_t (*read_pfn_t)(int fildes, void *buf, size_t nbyte);
static read_pfn_t g_sys_read_func 		= (read_pfn_t)dlsym(RTLD_NEXT,"read");

typedef int (*accept_pfn_t)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
static accept_pfn_t g_sys_accept_func 		= (accept_pfn_t)dlsym(RTLD_NEXT,"accept");


typedef ssize_t (*write_pfn_t)(int fildes, const void *buf, size_t nbyte);
static write_pfn_t g_sys_write_func 	= (write_pfn_t)dlsym(RTLD_NEXT,"write");



extern "C" ssize_t read(int fd, void *buf, size_t count) {
	LOG_DEBUG("call hook read !!!!!!!");
	ev_register_to_manager(fd, EPOLLIN ,EPOLL_CTL_ADD);
    int ret = g_sys_read_func(fd, buf, count);
	return ret;
}

extern "C" int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    LOG_DEBUG("call hook accept !!!!!!!");
    ev_register_to_manager(sockfd, EPOLLIN, EPOLL_CTL_ADD);
    int fd = g_sys_accept_func(sockfd, addr, addrlen);
    exit_if(fd < 0, "accept failed");
    return fd;
}

extern "C" ssize_t write(int fildes, const void *buf, size_t nbyte)
{
    LOG_DEBUG("call hook write  %s!!!!!!! fd=%d", (const char *)buf,fildes );
    ev_register_to_manager(fildes, EPOLLOUT  ,EPOLL_CTL_ADD);
  
	int ret = g_sys_write_func(fildes, buf, nbyte);
    LOG_DEBUG("ret=%d",ret);
    perror("error");
	return ret;

}
