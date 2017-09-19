#ifndef SYSUTIL_H
#define SYSUTIL_H

struct TimeInfo {
    char h;
    char m;
    char s;
    short ms;
};

void getCurrentTime(TimeInfo &timeInfo);

#endif /* SYSUTIL_H */

