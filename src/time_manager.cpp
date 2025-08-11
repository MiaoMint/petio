#include "time_manager.h"
#include <ESP8266WiFi.h>

TimeManager::TimeManager() : timeClient(ntpUDP, NTP_SERVER, TIME_ZONE * 3600) {
    timeInitialized = false;
    lastNTPUpdate = 0;
}

void TimeManager::begin() {
    timeClient.begin();
    timeClient.setUpdateInterval(NTP_UPDATE_INTERVAL);
    
    Serial.println("时间管理器初始化完成");
    
    // 如果 WiFi 已连接，立即尝试同步时间
    if (WiFi.status() == WL_CONNECTED) {
        forceSync();
    }
}

void TimeManager::update() {
    // 只有在 WiFi 连接时才更新 NTP 时间
    if (WiFi.status() == WL_CONNECTED) {
        timeClient.update();
        
        // 检查是否成功获取到时间
        if (!timeInitialized && timeClient.getEpochTime() > 0) {
            timeInitialized = true;
            lastNTPUpdate = millis();
            Serial.println("NTP 时间同步成功: " + getCurrentTimeString());
        }
        
        // 定期强制同步
        if (timeInitialized && millis() - lastNTPUpdate > NTP_UPDATE_INTERVAL) {
            forceSync();
        }
    }
}

bool TimeManager::isTimeValid() {
    // WiFi 连接且时间已初始化
    return (WiFi.status() == WL_CONNECTED) && timeInitialized && (timeClient.getEpochTime() > 0);
}

int TimeManager::getCurrentHour() {
    if (isTimeValid()) {
        return timeClient.getHours();
    }
    
    // 如果没有有效时间，返回基于运行时间的模拟时间（仅用于测试）
    unsigned long seconds = millis() / 1000;
    return (seconds / 3600) % 24;
}

int TimeManager::getCurrentMinute() {
    if (isTimeValid()) {
        return timeClient.getMinutes();
    }
    
    // 如果没有有效时间，返回基于运行时间的模拟时间（仅用于测试）
    unsigned long seconds = millis() / 1000;
    return (seconds / 60) % 60;
}

int TimeManager::getCurrentDay() {
    if (isTimeValid()) {
        // 返回自 Unix epoch 以来的天数
        return timeClient.getEpochTime() / 86400;
    }
    
    // 如果没有有效时间，返回基于运行时间的模拟天数
    return millis() / (24 * 60 * 60 * 1000UL);
}

String TimeManager::getCurrentTimeString() {
    char timeStr[10];
    
    if (isTimeValid()) {
        sprintf(timeStr, "%02d:%02d:%02d", 
                timeClient.getHours(), 
                timeClient.getMinutes(), 
                timeClient.getSeconds());
    } else {
        sprintf(timeStr, "%02d:%02d:%02d*", 
                getCurrentHour(), 
                getCurrentMinute(),
                (int)((millis() / 1000) % 60));
    }
    
    return String(timeStr);
}

String TimeManager::getCurrentDateString() {
    if (isTimeValid()) {
        unsigned long epochTime = timeClient.getEpochTime();
        
        // 简单的日期计算
        int days = epochTime / 86400;
        int year = 1970 + (days / 365);
        int month = ((days % 365) / 30) + 1;
        int day = (days % 30) + 1;
        
        char dateStr[20];
        sprintf(dateStr, "%04d-%02d-%02d", year, month, day);
        return String(dateStr);
    }
    
    return "未同步";
}

void TimeManager::forceSync() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("强制同步 NTP 时间...");
        timeClient.forceUpdate();
        
        if (timeClient.getEpochTime() > 0) {
            timeInitialized = true;
            lastNTPUpdate = millis();
            Serial.println("NTP 同步成功: " + getCurrentTimeString());
        } else {
            Serial.println("NTP 同步失败");
        }
    }
}

bool TimeManager::isWiFiTimeAvailable() {
    return WiFi.status() == WL_CONNECTED;
}

unsigned long TimeManager::getEpochTime() {
    if (isTimeValid()) {
        return timeClient.getEpochTime();
    }
    return 0; // 无效时间返回0
}
