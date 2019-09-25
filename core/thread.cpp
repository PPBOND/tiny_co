#include "thread.h"
#include <unistd.h>

using namespace std;

co_env  env;

//用于保存所有协程
std::deque<co_struct*> co_deques;

//准备就绪的协程跟正在运行的协程
std::deque<co_struct*> work_deques;

//主进程上下文,主要用来保存切换的上下文
co_struct co_main;

//协程休眠存放的链表
std::priority_queue<co_struct*, std::vector<co_struct*>, cmp_time> time_queue;

//协程等待时需要用到,唤醒则在epoll_wait后.
std::list<co_struct *> wait_list;

void co_sleep(int sleep_time)
{
      
    co_struct* current_co = get_current();
    gettimeofday(&current_co->tv,NULL);
    current_co->tv.tv_sec += sleep_time;
    current_co->status = Status::SLEEPING;
    time_queue.push(&current_co);
    co_yield();

    LOG_DEBUG("co_sleep end");
}


void wake_sleeping_co()
{
    struct timeval  tv;
    gettimeofday(&tv,NULL);   

    while(!time_queue.empty())
    {
        co_struct * top_co = time_queue.top();
        int diff_time = top_co->get_time_with_usec() - (tv.tv_sec*1000000 + tv.tv_usec);
        LOG_DEBUG("diff_time =%d",diff_time);
        if(diff_time < 0)
        {
            time_queue.pop();
            top_co->co->status = Status::READY;
            work_deques.push_back(top_co->co);
            
        }
        else
            return ;
    }

}



co_struct* get_current()
{
    return env.call_stack[env.index-1];
}


void co_func(co_struct* co)
{
    if(co->fun)
    {
        Fun fn = co->fun;
        fn(co->arg);
    }

    co_struct * now  = env.call_stack[env.index-1];
    now->is_end = true;
    LOG_DEBUG("over");
    co_yield();
}

void co_init()
{
    if(env.call_stack[0] == NULL)
    {
        getcontext(&co_main.context);
        co_main.co_id  = (size_t)&co_main;
        co_main.status = Status::RUNNING;
        env.call_stack[env.index++] = &co_main;
        env.ev_manger.create(20,1000);
    }
}



int co_create(co_struct* &co, Fun func, void *arg)
{
    
    co = new co_struct;
    getcontext(&co->context);
    co->arg    = arg;
    co->fun    = func;
    co->co_id  = (size_t )co;
    co->status = Status::READY;
    co->context.uc_stack.ss_sp    = co->stack;
    co->context.uc_stack.ss_size  = Default_size;
    co->context.uc_stack.ss_flags = 0;
    co->context.uc_link = NULL;

    makecontext(&co->context, (void (*)())co_func, 1, co);  
    co_deques.push_back(co);

    return 0;
}

void ready_co_to_queue()
{
    if(!work_deques.empty())
        return;
    
    for(auto& co: co_deques)
    {
        if(co->status == Status::READY)
        {
            LOG_DEBUG("co->id=%d",co->co_id);
            work_deques.push_back(co);
            LOG_DEBUG("work.size()=%d", work_deques.size());
        }
    }
}

void schedule()
{
    
    while(1)
    {
        wake_sleeping_co();
        ready_co_to_queue();
        if(!work_deques.empty())
            goto ready_co_run;

        if(work_deques.empty() && time_queue.empty() && wait_list.empty())
        {
            LOG_DEBUG("work_deues.empty, schedule finish");
            return;
        }

        env.ev_manger.wait_event();
        env.ev_manger.wake_event();        
        ready_co_to_queue();
        if(work_deques.empty())
            continue;

ready_co_run:
        co_struct* next_co = work_deques.front();
        work_deques.pop_front(); 

        LOG_DEBUG("next_co= %d", next_co->co_id);
        co_resume(next_co);

        if(next_co->status == Status::EXIT)
            co_releae(next_co);

        LOG_DEBUG("schedule end ");

    }

}



void  co_yield()
{
    co_struct * now  = env.call_stack[env.index -1];
    co_struct * prev = env.call_stack[env.index -2];
    LOG_DEBUG("will runing=%d",prev->co_id);
    env.index--;

    if(now->is_end == true)
        now->status = Status::EXIT;
    else if(now->status == Status::RUNNING) 
        now->status = Status::READY;

    prev->status = Status::RUNNING;
    swapcontext(&now->context, &prev->context);
    now->status = Status::RUNNING;

}



void co_resume(co_struct* co)
{
    if(co->status != Status::READY)
    {
        LOG_DEBUG("co->status != READY,=%d",co->status);
        return;
    }

    env.call_stack[env.index++] = co;

    co_struct * prev  = env.call_stack[env.index -2];
    co->status   = Status::RUNNING;    
    prev->status = Status::READY;
    swapcontext(&prev->context, &co->context);
    prev->status = Status::RUNNING;
   
}



void co_releae(co_struct* co)
{
    if(co->status != Status::EXIT)
        return;

    for(auto iter = co_deques.begin(); iter != co_deques.end();)
    {
        if(*iter == co)
        {
            iter = co_deques.erase(iter);
            delete co;
        }
        else
            iter++;
    }

    LOG_DEBUG("co_deques.size=%d", co_deques.size());
}



void ev_register_to_manager(int fd, int event,int ops)
{
    LOG_DEBUG("ev_register_to_manager fd =%d, event=%d, ops=%d",fd, event, ops );
    co_struct* current_co = get_current();
    current_co->ev.init_event(fd, event, ops);
    env.ev_manger.updateEvent(&current_co->ev);
    current_co->status = Status::WAITING;
    wait_list.push_back(current_co);
    co_yield();
}

int co_accept(int fd ,struct sockaddr* addr, socklen_t *len)
{
   
    int sockfd = -1;
    ev_register_to_manager(fd, EPOLLIN, EPOLL_CTL_ADD);
    sockfd = accept(fd, addr, len);
	exit_if(sockfd < 0, "accept failed");
    return sockfd;
}



ssize_t co_recv(int fd, void *buf, size_t len) {
	

	ev_register_to_manager(fd, EPOLLIN | EPOLLHUP,EPOLL_CTL_ADD);

	int ret = read(fd, buf, len);
	if (ret < 0) {
		//if (errno == EAGAIN) return ret;
		if (errno == ECONNRESET) return -1;
		//printf("recv error : %d, ret : %d\n", errno, ret);
		
	}
	return ret;
}