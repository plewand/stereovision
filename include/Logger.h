#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <string>
#include "SysUtil.h"

using namespace std;

enum LogLevel {
    NONE = 0,
    ERROR = 1,
    INFO = 2,
    DEBUG = 3
};

class Logger {
public:
    Logger();
    Logger(const Logger& orig);
    virtual ~Logger();

    template<typename... Args>
    static void error(const char *fmt, Args... args) {
        if(logLevel >= LogLevel::ERROR)
            logWithLevel("ERROR", fmt, args...);
    }
    
    template<typename... Args>
    static void log(const char *fmt, Args... args) {
        if(logLevel >= LogLevel::INFO)
            logWithLevel("INFO", fmt, args...);
    }     
    
    template<typename... Args>
    static void debug(const char *fmt, Args... args) {
        if(logLevel >= LogLevel::DEBUG)
            logWithLevel("DEBUG", fmt, args...);
    }    
    
    template<typename... Args>
    static void error(const char *msg) {
        error("%s", msg);
    }  
   
    template<typename... Args>
    static void log(const char *msg) {
        log("%s", msg);
    }  
    
    static void log(const string &str) {
        if(logLevel >= LogLevel::INFO)
            log("%s", str.c_str());
    }
    
    static void debug(const string &str) {
        if(logLevel >= LogLevel::INFO)
            debug("%s", str.c_str());
    }
    
    static void error(const string &str) {
        if(logLevel >= LogLevel::ERROR)
            error("%s", str.c_str());
    }   
    
    
    static LogLevel logLevel;

private:
    template<typename... Args>
    static void logWithLevel(const char *levelPrefix, const char *fmt, Args... args) {
        logTimestamp();        
        printf("[%s] ", levelPrefix);
        printf(fmt, args...);
        printf("\n");
        fflush(stdout);
    }
    
    static void logTimestamp() {        
        TimeInfo timeInfo;
        getCurrentTime(timeInfo);
              
        printf("%02d:%02d:%02d.%.3d: ", timeInfo.h, timeInfo.m, timeInfo.s, timeInfo.ms);
    }
};

#endif /* LOGGER_H */

