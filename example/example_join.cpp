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

void*  child(void *arg)
{
    LOG_DEBUG("child has been call");
    sleep(5);
    return (char*)("success exit");
}

void*  father(void *arg)
{   
    LOG_DEBUG("join child begin\n");
    CoRoutine_t *co_child;
    co_create(co_child, child, NULL, true);
    if(co_join(co_child, NULL) == 0){
         LOG_DEBUG("join child end \n");
    }
    else{
        LOG_DEBUG("co_id=%d cannot joinable\n", co_child->routine_id);
    }
    
      
    
    return NULL;
}


int main()
{
    co_init();  
    CoRoutine_t *co_father;
    co_create(co_father, father, NULL, false);
    event_loop_run();

}