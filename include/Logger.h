#pragma once
#include "coroutine.h"
#include <fstream>
#include <chrono>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>


#define WRITE_LOG(LEVEL, FMT, ...) \
{ \
    std::stringstream ss; \
        ss <<"[co_id = "<<get_current()->routine_id<<"]["<<__FILE__<< "][" <<__LINE__ << "][" <<__FUNCTION__<<"]"; \
        ss << FMT; \
    Logger::getInstance()->WriteLog(LEVEL, ss.str().c_str(), ##__VA_ARGS__); \
}

#define LOG_DEBUG(FMT , ...) WRITE_LOG(Logger::LOG_DEBUG, FMT, ##__VA_ARGS__)
#define LOG_ERROR(FMT , ...) WRITE_LOG(Logger::LOG_ERROR, FMT, ##__VA_ARGS__)

class Logger
{
public:
    enum level {LOG_DEBUG =0, LOG_ERROR };

    static Logger* getInstance() {
        static Logger* m_pInstance = new Logger();
        return m_pInstance;
    }


    void WriteLog(level log_level, const char * buf, ...)
    {
        va_list args;
		char logText[4096] = { '\0'};
		va_start(args, buf);
		vsnprintf(logText, 4095, buf, args);
		WriteLog(logText, log_level);
    }

    void WriteLog(const std::string& logText, level level = LOG_ERROR)
	{
		static const char *const LOG_STRING[] =
		{
			"LOG_DEBUG",
			"LOG_ERROR ",
		};

        std::stringstream szLogLine;
        szLogLine << "[" << getCurrentSystemTime(true)<< "] " <<LOG_STRING[level]<<logText<<std::endl;
		m_writer<<szLogLine.str();
		m_writer.flush();
		
	}
   


    Logger(){
        std::string file_name = "../log/zyw_co-";
        file_name +=getCurrentSystemTime(false)+".txt";
        m_writer.open(file_name,std::ofstream::out | std::ofstream::app);
    }

    ~Logger(){
        m_writer.close();
    }

    const std::string getCurrentSystemTime(bool isneed)    
    {
	    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	    struct tm* ptm = localtime(&tt);
	    char date[60] = {0};
        if(isneed)
	        sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
		        (int)ptm->tm_year + 1900,(int)ptm->tm_mon + 1,(int)ptm->tm_mday,
		        (int)ptm->tm_hour,(int)ptm->tm_min,(int)ptm->tm_sec);
	    else 
            sprintf(date, "%d-%02d-%02d",
		        (int)ptm->tm_year + 1900,(int)ptm->tm_mon + 1,(int)ptm->tm_mday);

        return std::string(date);
    }

private:
    std::ofstream  m_writer;
};



