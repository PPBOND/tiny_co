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
class heap_container_t : public std::priority_queue<T, std::vector<T>,Q>
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


class timer_event_t {

public:

    using timer_callback = void (*)(void *data);

    timer_event_t(timer_callback func, void *data_, int time_spec, int flag):
    expired_func(func),data(data_),expired_ms(time_spec),
    total_ms(time_now() + time_spec),cycle_flag(flag),
    in_heap(false){

    }

    void set_data(void * data_){
        this->data = data_;
    }

    ~timer_event_t(){

    }

    void update_time(int time_spec){
        this->expired_ms = time_spec;
        this->total_ms = time_spec + time_now();
    }

    void update_time_cycle(){
		this->total_ms = expired_ms + time_now();
	}

    bool operator >(const timer_event_t& rhs){
        return this->total_ms > rhs.total_ms;
    }

    bool operator < (const timer_event_t& rhs){
        return this->total_ms < rhs.total_ms;
    }

    long ms_time(){ 
        return this->total_ms; 
    }

    void run() { 
        return this->expired_func(data); 
    }

    timer_callback expired_func; // 超时后执行的函数
    void* data;                    //expired_func_的参数
    uint64_t expired_ms;           //相对的超时时间,单位ms
    long  total_ms;                //在插入时计算
    int   cycle_flag;              //是否循环执行
    bool  in_heap;                 //是否已经存在定时器最小堆中
  
  };


struct cmp_time
{
    bool operator()(timer_event_t*& lhs, timer_event_t*& rhs){
        return (*lhs) > (*rhs);
    }
};

class timer_manager_t 
{
public:
    timer_manager_t() = default;
    ~timer_manager_t() = default;

    int  add_timer(timer_event_t* time_ev);
    int  remove_timer(timer_event_t* time_ev);                  
    int  del_timer(timer_event_t* time_ev);       
    void check_expired();
    long get_mix_time();
    int  size();
    bool empty();    
private:
    heap_container_t <timer_event_t*, cmp_time>  m_min_heap;
};





