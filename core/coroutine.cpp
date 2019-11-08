#include "coroutine.h"
using namespace std;

//协程调度中心
Schedule_Centor  sche_centor;

int get_uuid()
{
    static int generator_uuid;
    return ++generator_uuid;
}

CoRoutine_t* get_current()
{
    return sche_centor.call_stack[sche_centor.chain_index -1];
}


void func_entry(CoRoutine_t* co_param)
{
    if(co_param->routine){
        co_param->exit_ret = co_param->routine(co_param->co_arg);
    }

    co_param->is_end = true;
    co_yield();
}

void co_init()
{
    if(sche_centor.call_stack[0] == NULL)
    {
        getcontext(&sche_centor.main_co.u_context);
        sche_centor.main_co.routine_id  = get_uuid();
        sche_centor.main_co.status = Status::running;
        sche_centor.call_stack[sche_centor.chain_index++] = &sche_centor.main_co;
        sche_centor.ev_manger.create(20,1000);
    }
}



int  co_create(CoRoutine_t*& create_co, Func func, void *arg, bool isjoin)
{
    create_co = new CoRoutine_t(func, arg,isjoin);
    
    makecontext(&create_co->u_context, (void (*)())func_entry, 1, create_co); 
    sche_centor.ready_manager.push_back(create_co);
    return 0;
}


void Schedule_Centor::shedule_run()
{
    while(1)
    {
        /*sleep函数的实现:如果有睡眠协程,执行特定的事件:将修改睡眠协程状态为可执行状态,然后放入就绪队列
          如果是普通事件,将执行定时事件
        */
        time_manager.checkexpire();
        if(!ready_manager.empty())
            goto  ready_co_run;

        if(ready_manager.empty() && time_manager.empty() && wait_manager.empty())
        {
            LOG_DEBUG("work_deues.empty, schedule finish");
            return;
        }

        /*
        等待IO事件发生,如果发生了则根据fd找到对应的协程,接着再将协程状态改变为可执行状态,然后放入就绪队列.
        等待时间则根据最小堆的top节点的时间进行等待,如果最小堆的节点为空,则默认为5秒.
        */
        this->ev_manger.wait_event();
        this->ev_manger.wake_event();
         
        if(ready_manager.empty())
            continue;  


ready_co_run:
        /*
            执行就绪事件,首先从就绪队列弹出一个协程执行,协程执行完或者被yield()并没执行完,再恢复到
            主协程时做判断,如果没执行完主动yield则继续放入队列等待执行,如果执行完毕并不是joinable则释放资源(这里默认都是detach协程).
            如果是joinable则需要自己调用co_join的处理.这里不做处理.
        */
        while(!ready_manager.empty())
        {
            CoRoutine_t* ready_co = ready_manager.front();
            ready_manager.pop_front(); 
            LOG_DEBUG("ready_co= %d\n", ready_co->routine_id);

            co_resume(ready_co);

	        LOG_DEBUG("co_status=%d\n",ready_co->status); 

            if(ready_co->status == Status::ready)
            {
                ready_manager.push_back(ready_co);   
            }
            else if(ready_co->status == Status::exit && !(ready_co->is_joinable))
                co_releae(ready_co);
        }
    }
}




void  co_yield()
{
    CoRoutine_t * current  = sche_centor.call_stack[sche_centor.chain_index -1];
    CoRoutine_t * prev     = sche_centor.call_stack[sche_centor.chain_index -2];
    LOG_DEBUG("will runing=%d", prev->co_id);
    sche_centor.chain_index--;

    if(current->is_end == true)    current->status = Status::exit;
    else if(current->status == Status::running)  current->status = Status::ready;
       
    prev->status = Status::running;
    swapcontext(&current->u_context, &prev->u_context);
    current->status = Status::running;

}


int co_join(CoRoutine_t* &co, void** retval)
{
    CoRoutine_t * current = get_current();
    if(co == current )  return 0;
    if(co->is_joinable == false)
        return -1;

    while(co->status != Status::exit)
        co_yield();
    
    if(retval != nullptr) 
        *retval = co->exit_ret;
    
    co_releae(co);
    return 0;
}



void co_resume(CoRoutine_t* ready_co)
{
    if(ready_co->status != Status::ready)
    {
        LOG_DEBUG("co->status != READY,=%d",ready_co->status);
        return;
    }

    sche_centor.call_stack[sche_centor.chain_index++] = ready_co;
    CoRoutine_t * prev_co  = sche_centor.call_stack[sche_centor.chain_index -2];

    ready_co->status     = Status::running;    
    prev_co->status      = Status::ready;
    swapcontext(&prev_co->u_context, &ready_co->u_context);
    prev_co->status      = Status::running;
}



void co_releae(CoRoutine_t* release_co)
{
    if(release_co->status != Status::exit)
        return;

    printf("co_id=%d release\n", release_co->routine_id);
    delete release_co;     
}



void ev_register_to_manager(int fd, int event,int ops)
{
    LOG_DEBUG("ev_register_to_manager fd =%d, event=%d, ops=%d",fd, event, ops );
    CoRoutine_t* current_co = get_current();
    current_co->status    = Status::waiting;
    current_co->ev.alter_status(fd, event, ops);
    sche_centor.ev_manger.updateEvent(&current_co->ev);
    sche_centor.wait_manager.push_back(current_co);
    co_yield();
}



void wake_sleep_co(void *co)
{
    CoRoutine_t* wake_co = (CoRoutine_t*) co;
    wake_co->status = Status::ready;
    sche_centor.ready_manager.push_back(wake_co);
}


TimerElem * addtimer(FuncPtrOnTimeout expired_func, void *data,
                                   uint64_t expired_ms, int flag)
{
    return sche_centor.time_manager.addtimer(expired_func,data,expired_ms, flag);

}

int deltimer(TimerElem *timer_elem)
{
    return sche_centor.time_manager.deltimer(timer_elem);
}

void event_loop_run()
{
    sche_centor.shedule_run();
}