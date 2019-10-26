#include "socket.h"
#include "thread.h"
#include "event.h"
#include "co_cond.h"
#include<string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


void  Producer(void *arg)
{
    while(1)
    {
        sleep(5);
        printf("Producer  product\n");
        
    }

}


void  Consumer(void *arg)
{
    while(1)
    {
        sleep(10);
        printf("Consumer  product\n");
    }
}



int main()
{
    co_init();
    co_struct *co_consume1;
    co_struct *co_consume2;
    co_struct *co_product;
    co_create(co_consume1, Consumer, NULL);
    co_create(co_product, Producer, NULL);
    schedule();

}
