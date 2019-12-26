#pragma once
#include "coroutine.h"
//保存被调方与调用方的链接关系,管理event事件与超时事件.


static int chain_index = 0;



class schedule_centor
{

public:
    static int  update_event(event_t* ev,int time_out){
        return ev_manager.update_event(ev, timeout);
    }

    static int  remove_event(Event* ev){
        return ev_manager.remove_event(ev);
    }
    
    static int  wait_event(){
        return ev_manager.wait_event();
    }
    static void wake_event(){
        return ev_manager.wait_event();
    }


    static event_t* alloc_event_by_fd(int fd){
        event_t* ev = new event_t();
        ev->fd = fd;
        event_map[fd] = ev;
        return ev;
    }
    
    static event_t* get_event_by_fd(int fd){
        
        if(event_map[fd].empty())
            return nullptr;
            
        return event_map[fd];
    }

    static int free_event_by_fd(int fd){

        if(!event_map[fd].empty()){
            delete event_map[fd];
            event_map.erase(fd);
            return 0;
        }
            return -1;
        
    }

    static void add_wait_list(coroutine_t * co)
    {
        wait_manager.push_back(co);
    }
    
    static void remove_wait_list(coroutine_t* co){
        wait_manager.remove(co);
    }

    static void add_ready_queue(coroutine_t* co){
        ready_manager.push_back(co);
    }

    static void remove_top_ready_queue(){
        ready_manager.pop_front();
    }

    static coroutine_t* get_top_ready_queue(){
        return ready_manager.front();        
    }


    static int remove_timer(timer_event_t*  timer_ev){
        return timer_manager.remove_timer(timer_ev);
    }

    static void add_timer(timer_event_t* timer_ev){
        timer_manager.add_timer(timer_ev);
    }

	static void check_expired() {
		timer_manager.check_expired();
	}
	static void create_epoll(int maxev, int time_out) {
		ev_manager.create(maxev, time_out);
	}


public:
    static int generator_uuid =0;
    static coroutine_t     main_co;
    
    static coroutine_t*   call_stack[128];
    static event_manager_t    ev_manager;
    static timer_manager_t  timer_manager;
    static wait_manager   wait_manager;
    static ready_manager  ready_manager;
    static std::map<int ,Event*> event_map; 
};


