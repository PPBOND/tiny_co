#include "thread.h"
#include <dlfcn.h>



typedef ssize_t (*read_pfn_t)(int fildes, void *buf, size_t nbyte);
typedef int (*accept_pfn_t)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
typedef ssize_t (*write_pfn_t)(int fildes, const void *buf, size_t nbyte);
typedef size_t (*send_pfn_t)(int socket, const void *buffer, size_t length, int flags);
typedef ssize_t (*recv_pfn_t)(int socket, void *buffer, size_t length, int flags);
typedef unsigned int (*sleep_pfn_t)(unsigned int seconds);

typedef ssize_t (*sendto_pfn_t)(int socket, const void *message, size_t length,
	                 int flags, const struct sockaddr *dest_addr,
					               socklen_t dest_len);

typedef ssize_t (*recvfrom_pfn_t)(int socket, void *buffer, size_t length,
	                 int flags, struct sockaddr *address,
					               socklen_t *address_len);





static accept_pfn_t g_sys_accept_func 	  = (accept_pfn_t)dlsym(RTLD_NEXT,"accept");
static read_pfn_t g_sys_read_func 		  = (read_pfn_t)dlsym(RTLD_NEXT,"read");
static write_pfn_t g_sys_write_func 	  = (write_pfn_t)dlsym(RTLD_NEXT,"write");
static recv_pfn_t g_sys_recv_func 		  = (recv_pfn_t)dlsym(RTLD_NEXT,"recv");
static send_pfn_t g_sys_send_func 		  = (send_pfn_t)dlsym(RTLD_NEXT,"send");
static sendto_pfn_t g_sys_sendto_func 	  = (sendto_pfn_t)dlsym(RTLD_NEXT,"sendto");
static recvfrom_pfn_t g_sys_recvfrom_func = (recvfrom_pfn_t)dlsym(RTLD_NEXT,"recvfrom");
static sleep_pfn_t g_sys_sleep_func       = (sleep_pfn_t)dlsym(RTLD_NEXT,"sleep");




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
    ev_register_to_manager(fildes, EPOLLOUT ,EPOLL_CTL_ADD);
	int ret = g_sys_write_func(fildes, buf, nbyte);
	return ret;

}

extern "C" unsigned int sleep(unsigned int seconds)
{
	LOG_DEBUG("sleep begin");
	co_struct* current_co = get_current();
    current_co->status    = Status::SLEEPING;

    gettimeofday(&current_co->tv,NULL);
    current_co->tv.tv_sec += seconds;
    time_queue.push(current_co);
    co_yield();
    LOG_DEBUG("sleep end");
	return 0;
}



ssize_t recvfrom(int socket, void *buffer, size_t length,
	                 int flags, struct sockaddr *address,
					               socklen_t *address_len)
{
	LOG_DEBUG("call hook recvfrom  %s!!!!!!! fd=%d", (const char *)buffer,socket );
    ev_register_to_manager(socket, EPOLLIN ,EPOLL_CTL_ADD);
	ssize_t ret = g_sys_recvfrom_func( socket,buffer,length,flags,address,address_len );
	return ret;
}



ssize_t sendto(int socket, const void *message, size_t length,
	                 int flags, const struct sockaddr *dest_addr,
					               socklen_t dest_len)
{
	/*
		1.no enable sys call ? sys
		2.( !lp || lp is non block ) ? sys
		3.try
		4.wait
		5.try
	*/
	LOG_DEBUG("call hook sendto  %s!!!!!!! fd=%d", (const char *)message,socket );

	ssize_t ret = g_sys_sendto_func( socket,message,length,flags,dest_addr,dest_len );
	if( ret < 0 && EAGAIN == errno )
	{
        ev_register_to_manager(socket, EPOLLOUT ,EPOLL_CTL_ADD);
		ret = g_sys_sendto_func( socket,message,length,flags,dest_addr,dest_len );

	}
	return ret;
}


ssize_t send(int socket, const void *buffer, size_t length, int flags)
{
    size_t wrotelen = 0;
	ssize_t writeret = g_sys_send_func( socket,buffer,length,flags );
	if (writeret == 0)
	{
		return writeret;
	}

	if( writeret > 0 )
	{
		wrotelen += writeret;	
	}
	while( wrotelen < length )
	{

	    ev_register_to_manager(socket, EPOLLOUT ,EPOLL_CTL_ADD);
		writeret = g_sys_send_func( socket,(const char*)buffer + wrotelen,length - wrotelen,flags );	
		if( writeret <= 0 )
		{
			break;
		}
		wrotelen += writeret ;
	}
	if (writeret <= 0 && wrotelen == 0)
	{
		return writeret;
	}

	return wrotelen;

}



ssize_t recv( int socket, void *buffer, size_t length, int flags )
{

    ev_register_to_manager(socket, EPOLLIN ,EPOLL_CTL_ADD);
	ssize_t readret = g_sys_recv_func( socket,buffer,length,flags );
	return readret;
	
}