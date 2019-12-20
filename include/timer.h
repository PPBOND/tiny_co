#pragma once
#include <sys/time.h>
#include <vector>
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
    auto time_now = chrono::system_clock::now();
	return chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch()).count();
}


class Time_Event {

public:

    using FuncPtrOnTimeout = void (*)(void *data);

    Time_event(FuncPtrOnTimeout func, void *data, int time_spec, int flag):expired_func(func),
    data(data),expired_ms(time_spec),cycle_flag(flag) {

    }

    ~Time_event(){
        if(data_ != nullptr) delete data;
    }

    void update_time(int time_spec){
        this->expired_ms = time_spec;
        this->total_ms = time_spec + time_now();
    }

    void update_time_cycle(){
		this->total_ms = expired_ms + time_now();
	}

    Time_event& operator >(const Time_event & rhs){
        return this->total_ms > rhs.total_ms;
    }

     Time_event& operator < (const Time_event & rhs){
        return this->total_ms < rhs.total_ms;
    }
    long ms_time(){ return this->total_ms; }
    void run() { return this->expired_func(data); }

	bool is_timeout;
    FuncPtrOnTimeout expired_func; // 超时后执行的函数
    void* data;                    //expired_func_的参数
    uint64_t expired_ms;           //相对的超时时间,单位ms
    long  total_ms;                //在插入时计算
    int   cycle_flag;              //是否循环执行
  
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
    int  addTimeEvent(Time_event * time_event);                  
    int  delTimeEvent(Time_event *time_event);       
    void checkExpire();
    long getMixTime();
    int  size();
    bool empty();    
private:
    Heap_Container <Time_Event*, cmp_time>  m_min_heap;
};





