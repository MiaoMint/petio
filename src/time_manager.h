#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include "config.h"

class TimeManager {
private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    bool timeInitialized;
    unsigned long lastNTPUpdate;
    
public:
    TimeManager();
    void begin();
    void update();
    bool isTimeValid();
    int getCurrentHour();
    int getCurrentMinute();
    int getCurrentDay(); // 获取当前日期（用于每天重复检查）
    unsigned long getEpochTime(); // 获取当前时间戳
    String getCurrentTimeString();
    String getCurrentDateString();
    void forceSync();
    bool isWiFiTimeAvailable();
};

#endif
