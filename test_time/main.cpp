#include "timer.h"


CTimerManager manager;

void fooTimeout(char *data) {
  printf("%s\n",data);
}
int main()
{

    char * hello ="helloworld";
    TimerElem * elem = manager.AddTimer((FuncPtrOnTimeout)fooTimeout,hello,2, 1);
    while(1)
    manager.CheckExpire();

    return 0;
}