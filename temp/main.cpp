#include "timer.h"
#include<stdio.h>


void test(void *data)
{
	printf("%s\n",(char*)data);	

}



int main()
{
	timer_manager_t manager;
	char buf[1024]="fuckfuck";
	timer_event_t t_ev(test,buf,5000,1);
	manager.add_timer(&t_ev);
	do
	{
		manager.check_expired();

	}while(manager.size()!=0);
	return 0;


}
