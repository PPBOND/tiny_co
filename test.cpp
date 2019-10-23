#include<iostream>
#include<vector>
#include<queue>
#include <algorithm>
using namespace std;

struct cmp_int
{
    bool operator()(int &lhs, int &rhs)
    {
        return lhs> rhs;
    }

};

template<typename T,typename Q>
class custom_priority_queue : public std::priority_queue<T, std::vector<T>,Q >
{
  public:

      bool remove(const T& value) {
        auto it = std::find(this->c.begin(), this->c.end(), value);
        if (it != this->c.end()) {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
       }
       else {
        return false;
       }
 }
};



int main()
{
   custom_priority_queue<int,cmp_int> queue;

   queue.push(10);
   queue.push(2);
   queue.push(4);
   queue.push(6);
   queue.push(3);

   queue.remove(8);

   while (!queue.empty())
   {
      std::cout << queue.top();
      queue.pop();

      if (!queue.empty())
      {
        std::cout << ", ";
      }
   }
   return 0;

 }
