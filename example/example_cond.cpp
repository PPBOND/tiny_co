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

void*  Producer(void *arg)
{
    while(1)
    {
        product ++;
        product ++;
        product ++;
        LOG_DEBUG("Producer  product::%d\n",product);
        cond.cond_wake_once();
        sleep(5);
    }

    return NULL;

}


void*  Consumer(void *arg)
{
    while(1)
    {

        if(product == 0)
            cond.cond_wait();
        product--;
        LOG_DEBUG("%s  product::%d\n",(char*)arg, product);
    }

    return NULL;
}



int main()
{
    char buf1[1024]= "consume1";
    char buf2[1024]= "consume2";
    co_init();
    CoRoutine_t *co_consume1;
    CoRoutine_t *co_consume2;
    CoRoutine_t *co_product;
    co_create(co_consume1, Consumer, buf1, 0);
    co_create(co_consume2, Consumer, buf2, 0);
    co_create(co_product, Producer, NULL, 0);
    event_loop_run();

}
