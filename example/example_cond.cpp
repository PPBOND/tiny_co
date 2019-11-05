#include "socket.h"
#include "coroutine.h"
#include "event.h"
#include "co_cond.h"
#include<string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

co_cond_t cond;

static int product =0;

void  Producer(void *arg)
{
    while(1)
    {
        product ++;
        product ++;
        product ++;
        printf("Producer  product::%d\n",product);
        cond.cond_wake_once();
        sleep(5);
    }

}


void  Consumer(void *arg)
{
    while(1)
    {

        if(product == 0)
            cond.cond_wait();
        product--;
        printf("%s  product::%d\n",(char*)arg, product);
    }
}



int main()
{
    char *test1 ="consume1";
    char *test2 = "consume2";
    co_init();
    co_struct *co_consume1;
    co_struct *co_consume2;
    co_struct *co_product;
    co_create(co_consume1, Consumer, test1);
  //  co_create(co_consume2, Consumer, test2);
   // co_create(co_product, Producer, NULL);
    schedule();

}
