#pragma once
#include "coroutine.h"
//保存被调方与调用方的链接关系,管理event事件与超时事件.

class Schedule_Centor
{

public:
    static int  update_event(Event* ev,int time_out){
        return ev_manger.update_event(ev, timeout);
    }

    static int  remove_event(Event* ev){
        return ev_manger.remove_event(ev);
    }
    
    static int  wait_event(){
        return ev_manger.wait_event();
    }
    static void wake_event(){
        return ev_manger.wait_event();
    }


    Event* alloc_event_by_fd(int fd){
        Event* ev = new Event();
        event_map[fd] = ev;
        return ev;
    }
    
    Event* get_event_by_fd(int fd){
        if(event_map[fd].empty())
            return nullptr;
        return event_map[fd];
    }
    int free_event_by_fd(int fd){

        if(!event_map[fd].empty()){
            delete event_map[fd];
            event_map.erase(fd);
            return 0;
        }
            return -1;
        
    }

public:
    int chain_index;
    int generator_uuid =0;
    CoRoutine_t*   call_stack[128];
    static Epoll_event    ev_manger;
    static Timer_Manager  time_manager;
    static Wait_Manager   wait_manager;
    static Ready_Manager  ready_manager;
    CoRoutine_t    main_co;
    static std::map<int ,Event*> event_map; 
};