#include "socket.h"
#include "thread.h"
#include "event.h"
#include<string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


static Event listen_ev;
using namespace std;

void handleTask(void *rhs)
{
    while(1)
    {
        char * buf= (char*) rhs;
        printf("%s\n",buf);
        sleep(2);

    }
    
}
int main()
{
    co_init();
    co_struct *test_time;
    co_timer(test_time, handleTask, (void*)("helloworld"),2);
    schedule();
}



