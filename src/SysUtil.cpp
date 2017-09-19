#include "SysUtil.h"
#include <chrono>
#include <ctime>
#include <string>

using namespace std;
using namespace chrono;

void getCurrentTime(TimeInfo &timeInfo) {    
    time_t t = time(0);
    struct tm *ltNow = localtime(&t);

    system_clock::time_point now = system_clock::now();
    system_clock::duration tp = now.time_since_epoch();
    tp -= duration_cast<seconds>(tp);
    timeInfo.h = ltNow->tm_hour;
    timeInfo.m = ltNow->tm_min;
    timeInfo.s = ltNow->tm_sec;
    timeInfo.ms = (short)(tp/ milliseconds(1));
}