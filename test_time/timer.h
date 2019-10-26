#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <queue>
#include <unistd.h>


template<typename T,typename Q>
class heap : public std::priority_queue<T, std::vector<T>,Q>
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


typedef void (*FuncPtrOnTimeout)(void *data);

struct TimerElem {
    
    FuncPtrOnTimeout expired_func_; // 超时后执行的函数
    void *data_;                    //expired_func_的参数
    uint64_t expired_ms_;           //相对的超时时间,单位ms
    struct timeval tv;       
    int cycle_flag;
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


class CTimerManager {
public:
  // 添加定时器
    TimerElem *AddTimer(FuncPtrOnTimeout expired_func, void *data,
                      uint64_t expired_ms, int flag);                  
    int DelTimer(TimerElem *timer_elem);       
    bool CheckExpire();
    struct timeval get_mix_time();
    int get_elem_size();
};





