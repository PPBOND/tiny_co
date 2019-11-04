#include "socket.h"
#include "thread.h"
#include "event.h"
#include "co_cond.h"
#include<string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>



void fuck_test(void *data)
{
    printf("%s\n",(char*)data);

}

void  Consumer(void *arg)
{
    char * test ="fuck_test";
    auto k =addtimer(fuck_test,test,2,1);
    sleep(11);
  //  DelTimer(k);

}

int main()
{


    co_init();
    co_struct *co_product;
    co_create(co_product, Consumer, NULL);
    
  
    
    schedule();


}
