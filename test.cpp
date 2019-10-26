#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <queue>
#include <unistd.h>
using namespace std;
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


static inline uint64_t GetTimeMs() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000UL + tv.tv_usec / 1000;
}

typedef void (*FuncPtrOnTimeout)(void *data);

struct TimerElem {
  friend class CTimerManager;

public:
  TimerElem();
  FuncPtrOnTimeout expired_func_; // 超时后执行的函数
  void *data_;                    //expired_func_的参数
  uint64_t expired_ms_;           // 绝对的超时时间,单位ms
  int heap_idx_;                  //用来删除定时器
};

struct cmp_time
{
    bool operator()(TimerElem* &lhs, TimerElem* &rhs)
    {
        return lhs->expired_ms_ > rhs->expired_ms_;
    }

};


static heap<TimerElem*,cmp_time>  min_heap;




class CTimerManager {
public:
  // 添加定时器
  TimerElem *AddTimer(FuncPtrOnTimeout expired_func, void *data,
                      uint64_t expired_ms) {
    TimerElem *timer_elem = (TimerElem *)malloc(sizeof(TimerElem));
    if (!timer_elem) {
      return NULL;
    }
    timer_elem->expired_func_ = expired_func;
    timer_elem->data_ = data;
    timer_elem->expired_ms_ = expired_ms;
    min_heap.push(timer_elem);
    return timer_elem;
  }
  //删除定时器
  int DelTimer(TimerElem *timer_elem) {
    if (timer_elem == NULL) {
      return -1;
    }
    min_heap.remove(timer_elem);
    return 0;
  }
  //检查是否有定时器超时
  bool CheckExpire() {
   
   
    while(min_heap.size()> 0)
    {
        bool has_expired = false;
    uint64_t now_ms = GetTimeMs();

       printf("min_heap.size=%d\n", min_heap.size());
       sleep(1);
      TimerElem * top_elem = min_heap.top();
      if(top_elem->expired_ms_ < now_ms )
      {
        top_elem->expired_func_(top_elem->data_);
        min_heap.pop();
      }

    }
   
  }
};




// test code below
void fooTimeout(void *data) {
  uint64_t *expired_ms = (uint64_t *)data;
  printf("%s,%lu \n", __func__, *expired_ms);
}

int main() {

  CTimerManager timer_manager;


    uint64_t *expired_ms = (uint64_t *)malloc(sizeof(uint64_t));
    *expired_ms = GetTimeMs() + 900;
    timer_manager.AddTimer(fooTimeout, expired_ms, *expired_ms);
  

    uint64_t *expired_ms1 = (uint64_t *)malloc(sizeof(uint64_t));
    *expired_ms1 = GetTimeMs() + 100;
     timer_manager.AddTimer(fooTimeout, expired_ms1, *expired_ms1);
  

 
     printf("min_heap.size=%d\n", min_heap.size());
    timer_manager.CheckExpire();
  
  

  return 0;
}
