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

void*  test_fuck(void *arg)
{
    printf("test_fuck has been call\n");
    sleep(5);
    return (char*)("success exit");
}

void*  Producer(void *arg)
{   
    printf("wait test_fuck begin\n");
    co_struct *co_product;
    co_create(co_product, test_fuck, NULL);
    co_join(co_product, NULL);
    printf("wait_test_fuck end \n");
    return NULL;
}


int main()
{
    co_init();  
    co_struct *co_product;
    co_create(co_product, Producer, NULL);
    schedule();

}
