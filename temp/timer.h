#pragma once
#include <sys/time.h>
#include <vector>
#include <chrono>
#include <algorithm>
#include <queue>
#include <unistd.h>


/*
 此协程库只是用来学习,并不需要过多考虑性能,因此避免造轮子使用priority_queue来构建最小堆.
 priority_queue只是容器适配器不提供erase与remove操作,而其底层容器一般默认为vector
 其成员为c,因此可以遍历底层容器删除指定元素,再make_heap重新建立,复杂度为O(n)
*/
template<typename T,typename Q>
class Heap_Container : public std::priority_queue<T, std::vector<T>,Q>
{

public:
    bool remove(const T& value)
    {
        auto it = std::find(this->c.begin(), this->c.end(), value);
        if (it != this->c.end()) 
        {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
        }
        else 
            return false;
    }
};



static inline  long time_now(){
    auto time_now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch()).count();
}


class Time_Event {

public:

    using FuncPtrOnTimeout = void (*)(void *data);

    Time_Event(FuncPtrOnTimeout func, void *data_, int time_spec, int flag):expired_func(func),
    data(data_),expired_ms(time_spec),total_ms(time_now() + time_spec),
    cycle_flag(flag),in_heap(false){

    }

    void set_args(void * data_){
        this->data = data_;
    }

    ~Time_Event(){

    }

    void update_time(int time_spec){
        this->expired_ms = time_spec;
        this->total_ms = time_spec + time_now();
    }

    void update_time_cycle(){
		this->total_ms = expired_ms + time_now();
	}

    bool operator >(const Time_Event & rhs){
        return this->total_ms > rhs.total_ms;
    }

    bool operator < (const Time_Event & rhs){
        return this->total_ms < rhs.total_ms;
    }
    
    long ms_time(){ 
        return this->total_ms; 
    }
    
    void run() { 
        return this->expired_func(data);
    }

    FuncPtrOnTimeout expired_func; // 超时后执行的函数
    void*  data;                    //expired_func_的参数
    long  expired_ms;           //相对的超时时间,单位ms
    long  total_ms;                //在插入时计算
    int   cycle_flag;              //是否循环执行
    bool  in_heap;                 //是否已经存在定时器最小堆中
  
  };


struct cmp_time
{
    bool operator()(Time_Event* &lhs, Time_Event* &rhs){
        return (*lhs) > (*rhs);
    }
};

class Timer_Manager 
{
public:
    int  add_timer(Time_Event * time_event);
    int  remove_timer(Time_Event * time_event);                  
    int  del_timer(Time_Event *time_event);       
    void check_expired();
    long get_mix_time();
    int  size();
    bool empty();    
private:
    Heap_Container <Time_Event*, cmp_time>  m_min_heap;
};





