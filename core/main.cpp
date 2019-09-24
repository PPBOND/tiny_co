#include "thread.h"
#include<unistd.h>
#include<string>
using namespace std;

co_struct * co1;
co_struct * co2;
co_struct * co3;
extern co_struct* current;


void fun1(void *arg)
{
   // while(1)
    {
        string f1_str = *(string*)arg;
        printf("%s\n\n",f1_str.c_str());
        co_sleep(4);
        printf("%s\n\n",f1_str.c_str());
        co_yield();
       
    }
}

void fun2(void *arg)
{
 
        string f2_str = *(string*)arg;
        printf("%s\n\n",f2_str.c_str());
        co_sleep(2);
        printf("%s\n\n",f2_str.c_str());
        
        
  
}


int main()
{
   
    co_init();
    string  str1 = "call func1";
    string  str2 = "call func2";
 
    co_create(co1, fun1,&str1);
    co_create(co2, fun2,&str2);

    schedule();

    return 0;
}