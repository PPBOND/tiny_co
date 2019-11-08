#include "socket.h"
#include "coroutine.h"
#include "event.h"
#include "co_cond.h"
#include<string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


void*  Producer(void *arg)
{
    while(1)
    {
        sleep(2);
        printf("Producer  product\n");
        
    }
    return NULL;

}


void*  Consumer(void *arg)
{
    while(1)
    {
        sleep(4);
        printf("Consumer  product\n");
    }

    return NULL;
}



int main()
{
    co_init();
    co_struct *co_consume1;
    co_struct *co_consume2;
    co_struct *co_product;
    co_create(co_consume1, Consumer, NULL, 0);
    co_create(co_product, Producer, NULL, 0);
    schedule();

}
