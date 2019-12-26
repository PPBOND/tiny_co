#include "coroutine.h"
#include "schedule.h"
using namespace std;

//协程调度中

int get_uuid()
{
    static int generator_uuid;
    return ++generator_uuid;
}

coroutine_t* get_current()
{
    return schedule_centor::call_stack[chain_index - 1]; 
}


void func_entry(coroutine_t* co_param)
{
    if(co_param->routine){
        co_param->exit_ret = co_param->routine(co_param->co_arg);
    }

    co_param->is_end = true;
    co_yield();
}

void co_init()
{
    if(schedule_centor::call_stack[0] == NULL)
    {
        getcontext(&main_co.u_context);
        main_co.routine_id  = get_uuid();
        main_co.status = Status::running;
        schedule_centor::call_stack[chain_index++] = &main_co;
        schedule_centor::create_epoll(20,5000);
    }
}



int  co_create(coroutine_t*& create_co, Func func, void *arg)
{
    create_co = new coroutine_t(func, arg);
    
    makecontext(&create_co->u_context, (void (*)())func_entry, 1, create_co); 
    schedule_centor::add_ready_queue(create_co);
    return 0;
}


void shedule_run()
{
    while(1)
    {
        /*sleep函数的实现:如果有睡眠协程,执行特定的事件:将修改睡眠协程状态为可执行状态,然后放入就绪队列
          如果是普通事件,将执行定时事件
        */
        schedule_centor::check_expired();
        
        if(!schedule_centor::ready_manager.empty())
            goto  ready_co_run;

        if(schedule_centor::ready_manager.empty() && 
        schedule_centor::timer_manager.empty() && 
        schedule_centor::wait_manager.empty())
        {
            LOG_DEBUG("work_deues.empty, schedule finish");
            return;
        }

        /*
        等待IO事件发生,如果发生了则根据fd找到对应的协程,接着再将协程状态改变为可执行状态,然后放入就绪队列.
        等待时间则根据最小堆的top节点的时间进行等待,如果最小堆的节点为空,则默认为5秒.
        */
        schedule_centor::wait_event();
        schedule_centor::wake_event();
         
        if(ready_manager.empty())
            continue;  


ready_co_run:
        /*
            执行就绪事件,首先从就绪队列弹出一个协程执行,协程执行完或者被yield()并没执行完,再恢复到
            主协程时做判断,如果没执行完主动yield则继续放入队列等待执行,如果执行完毕并不是joinable则释放资源(这里默认都是detach协程).
            如果是joinable则需要自己调用co_join的处理.这里不做处理.
        */
        while(!schedule_centor::ready_manager.empty())
        {
            coroutine_t* ready_co = schedule_centor::get_top_ready_queue();
            schedule_centor::remove_top_ready_queue();
            LOG_DEBUG("ready_co= %d", ready_co->routine_id);

            co_resume(ready_co);

	        LOG_DEBUG("co_status=%d",ready_co->co_status); 

            if(ready_co->co_status == status::ready)
            {
                ready_manager.push_back(ready_co);   
            }
            else if(ready_co->co_status == status::exit && !(ready_co->is_joinable))
                co_releae(ready_co);
            /*这里临时修复,如果被等待的协程sleep同时等待协程joinable那将永远无法执行到定时器
              下一步准备实现:将join跟被join的协程根据标志位关联,当被join执行退出时,修改与之关联的join协程的状态为ready同时加入到就绪队列中
             */
                schedule_centor::check_expired();
        }
    }
}




void  co_yield()
{
    coroutine_t * current  = schedule_centor::call_stack[chain_index -1];
    coroutine_t * prev     = schedule_centor::call_stack[chain_index -2];
    chain_index--;

    if(current->is_end == true)    
        current->co_status = status::exit;
    else if(current->co_status == status::running)  
        current->co_status = status::ready;
       
    prev->co_status = status::running;
    swapcontext(&current->u_context, &prev->u_context);
    current->co_status = status::running;

}


int co_join(coroutine_t* &co, void** retval)
{
    coroutine_t * current = get_current();
    if(co == current )  return 0;
    if(co->is_joinable == false)
        return -1;

    while(co->co_status != status::exit)
        co_yield();
    
    if(retval != nullptr) 
        *retval = co->exit_ret;
    
    co_releae(co);
    return 0;
}



void co_resume(coroutine_t* ready_co)
{
    if(ready_co->co_status != status::ready)
    {
        LOG_DEBUG("co->co_status != READY,=%d",ready_co->co_status);
        return;
    }

    schedule_centor::call_stack[chain_index++] = ready_co;
    coroutine_t * prev_co  = schedule_centor::call_stack[chain_index -2];

    ready_co->co_status     = status::running;    
    prev_co->co_status      = status::ready;
    swapcontext(&prev_co->u_context, &ready_co->u_context);
    prev_co->co_status      = status::running;
}



void co_releae(coroutine_t* release_co)
{
    if(release_co->co_status != status::exit)
        return;

    LOG_DEBUG("co_id=%d release\n", release_co->routine_id);
    delete release_co;     
}



void ev_register_to_manager(event_t* ev,int timeout)
{
    
    coroutine_t* current_co = get_current();
    current_co->co_status      = status::waiting;
    ev->co = current_co;
    schedule_centor::update_event(ev, timeout);
    schedule_centor::add_wait_list(current_co);
    co_yield();
}



void wake_sleep_co(void *co)
{
    coroutine_t* wake_co = (coroutine_t*) co;
    wake_co->co_status = status::ready;
    schedule_centor::add_ready_queue(wake_co);
}


void event_loop_run()
{
    schedule_centor::shedule_run();
}