#pragma once
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <queue>
#include <unistd.h>


template<typename T,typename Q>
/*
 此协程库只是用来学习,并不需要过多考虑性能,因此避免造轮子使用priority_queue来构建最小堆.
 priority_queue只是容器适配器不提供erase与remove操作,而其底层容器一般默认为vector
 其成员为c,因此可以遍历底层容器删除指定元素,再make_heap重新建立,复杂度为O(n)
*/
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

using FuncPtrOnTimeout = void (*)(void *data);
//typedef void (*FuncPtrOnTimeout)(void *data);


struct TimerElem {
    bool isexec = false;  //判断是否执行过
    FuncPtrOnTimeout expired_func_; // 超时后执行的函数
    void *data_;                    //expired_func_的参数
    uint64_t expired_ms_;           //相对的超时时间,单位ms
    struct timeval tv;       
    int cycle_flag;                 //是否循环执行
    int get_time_with_usec() const{
        return tv.tv_sec*1000000 + tv.tv_usec;
    }
  
  };


struct cmp_time
{
    bool operator()(TimerElem* &lhs, TimerElem* &rhs){
        return lhs->get_time_with_usec() > rhs->get_time_with_usec();
    }
};


class CTimerManager 
{
public:
    TimerElem *AddTimer(FuncPtrOnTimeout expired_func, void *data,
                      uint64_t expired_ms, int flag);                  
    int  DelTimer(TimerElem *timer_elem);       
    void CheckExpire();
    struct timeval get_mix_time();
    int  size();
    bool empty();    
    
    
private:
    Heap_Container <TimerElem*, cmp_time>  m_min_heap;
};





