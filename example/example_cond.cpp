#include "socket.h"
#include "thread.h"
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
        LOG_DEBUG("Producer  product::%d\n",product);
        cond.cond_wake_once();
        sleep(1);
    }

}


void  Consumer(void *arg)
{
    while(1)
    {

        if(product == 0)
            cond.cond_wait();
        product--;
        LOG_DEBUG("Consumer  product::%d\n",product);
    }
}



int main()
{
    co_init();
    co_struct *co_consume1;
    co_struct *co_consume2;
    co_struct *co_product;
    co_create(co_consume1, Consumer, NULL);
    co_create(co_consume2, Consumer, NULL);
    co_create(co_product, Producer, NULL);
    schedule();


}
