#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "wifi_manager.h"
#include "timer_manager.h"
#include "time_manager.h"
#include "web_pages.h"

class WebServer {
private:
    ESP8266WebServer server;
    WiFiManager* wifiManager;
    TimerManager* timerManager;
    TimeManager* timeManager;
    
public:
    WebServer(WiFiManager* wm, TimerManager* tm, TimeManager* timeM);
    void begin();
    void handleClient();
    
private:
    void setupRoutes();
    
    // 页面路由
    void handleRoot();
    
    // API 路由
    void handleGetStatus();
    void handleGetSystemInfo();
    void handleGetTimers();
    void handleAddTimer();
    void handleUpdateTimer();
    void handleDeleteTimer();
    void handleClearTimers();
    void handleGetPins();
    void handleGetPWMConfig();
    void handleManualControl();
    void handleWiFiConfig();
    void handleWiFiReset();
    void handleRestartAP();
    void handleFirmwareUpdate();
    
    // 工具函数
    void sendJSON(int code, const String& message, bool success = true);
    void sendJSON(JsonDocument& doc);
    void enableCORS();
};

#endif
