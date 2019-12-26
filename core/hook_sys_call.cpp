#include "coroutine.h"
#include <dlfcn.h>


typedef int (*socket_pfn_t)(int domain, int type, int protocol);
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
typedef int (*close_pfn_t)(int fd);




static accept_pfn_t g_sys_accept_func 	  = (accept_pfn_t)dlsym(RTLD_NEXT,"accept");
static read_pfn_t g_sys_read_func 		  = (read_pfn_t)dlsym(RTLD_NEXT,"read");
static write_pfn_t g_sys_write_func 	  = (write_pfn_t)dlsym(RTLD_NEXT,"write");
static recv_pfn_t g_sys_recv_func 		  = (recv_pfn_t)dlsym(RTLD_NEXT,"recv");
static send_pfn_t g_sys_send_func 		  = (send_pfn_t)dlsym(RTLD_NEXT,"send");
static sendto_pfn_t g_sys_sendto_func 	  = (sendto_pfn_t)dlsym(RTLD_NEXT,"sendto");
static recvfrom_pfn_t g_sys_recvfrom_func = (recvfrom_pfn_t)dlsym(RTLD_NEXT,"recvfrom");
static sleep_pfn_t g_sys_sleep_func       = (sleep_pfn_t)dlsym(RTLD_NEXT,"sleep");
static socket_pfn_t g_sys_socket_func 	= (socket_pfn_t)dlsym(RTLD_NEXT,"socket");
static close_pfn_t g_sys_close_func 	= (close_pfn_t)dlsym(RTLD_NEXT,"close");


extern "C" ssize_t read(int fd, void *buf, size_t count)
{	
	event_t* ev = get_event_by_fd(fd);
	ev.set_event(EPOLLIN);
	ev_register_to_manager(ev, 5);
    int ret = g_sys_read_func(fd, buf, count);
	return ret;
}



extern "C" int socket(int domain, int type, int protocol)
{

	int fd = g_sys_socket_func(domain,type,protocol);
	if( fd < 0 )
	{
		return fd;
	}

	alloc_event_by_fd( fd );	
	return fd;
}


extern "C" int close(int fd)
{
	free_event_by_fd(fd);
	int ret = g_sys_close_func(fd);
	return ret;
}

extern "C" int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	
    LOG_DEBUG("call hook accept !!!!!!!");
	ev_register_to_manager(sockfd, EPOLLIN, EPOLL_CTL_ADD);
    int fd = g_sys_accept_func(sockfd, addr, addrlen);
    exit_if(fd < 0, "accept failed");

	alloc_event_by_fd(fd);
    return fd;
}

extern "C" ssize_t write(int fildes, const void *buf, size_t nbyte)
{
	size_t wrotelen =0;
	int retlen = g_sys_write_func(fildes, (const char*)buf+ wrotelen, nbyte -wrotelen );
	if(retlen == 0)
		return  retlen;
	
	if(retlen >0)
	{
		wrotelen += retlen;
	}

	while(wrotelen < nbyte)
	{

		ev_register_to_manager(fildes, EPOLLOUT, EPOLL_CTL_ADD);
		retlen = g_sys_write_func(fildes, (const char*)buf + wrotelen, nbyte -wrotelen );
		
		if(retlen <=0)
			break;
		
		wrotelen += retlen;	
	}

	if(retlen<=0 &&wrotelen ==0 )
		return retlen;
	return wrotelen;

}

extern "C" unsigned int sleep(unsigned int seconds)
{
	LOG_DEBUG("sleep begin");
	CoRoutine_t* current_co = get_current();
    current_co->status      = Status::sleeping;
	current_co->time_event  = addtimer(wake_sleep_co, current_co, seconds, ONCE_EXEC);
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
		no enable sys call ? sys
		( !lp || lp is non block ) ? sys
		3.try
		4.wait
		5.try
	*/
	LOG_DEBUG("call hook sendto  %s!!!!!!! fd=%d", (const char *)message,socket );

	ssize_t ret = g_sys_sendto_func( socket,message,length,flags,dest_addr,dest_len );
	if( ret < 0 && EAGAIN == errno )
	{
        ev_register_to_manager( socket, EPOLLOUT ,EPOLL_CTL_ADD);
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
