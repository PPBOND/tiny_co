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
    while(1)
    {
        string f1_str = *(string*)arg;
        printf("%s\n\n",f1_str.c_str());
        co_sleep(1);
        printf("%s\n\n",f1_str.c_str());
       
    }
}

void fun2(void *arg)
{
    while(1)
    {
        string f2_str = *(string*)arg;
        printf("%s\n\n",f2_str.c_str());
        co_sleep(1);
        printf("%s\n\n",f2_str.c_str());
        
        
    }
}

void fun3(void *arg)
{
    while(1)
    {
        string f3_str = *(string*)arg;
        printf("%s\n\n",f3_str.c_str());
        co_sleep(1);
        printf("%s\n\n",f3_str.c_str());
      
    }

}

int main()
{
   
    
    string  str1 = "call func1";
    string  str2 = "call func2";
    string  str3 = "call func3";

    for(int i=0;i<1;++i)
    {
        
        co_create(co1, fun1,&str1);
        co_create(co2, fun2,&str2);
        co_create(co3, fun3,&str3);

    }
  
    schedule();

    return 0;
}
