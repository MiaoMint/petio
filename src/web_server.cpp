#include "web_server.h"

WebServer::WebServer(WiFiManager* wm, TimerManager* tm, TimeManager* timeM) : server(WEB_SERVER_PORT) {
    wifiManager = wm;
    timerManager = tm;
    timeManager = timeM;
}

void WebServer::begin() {
    setupRoutes();
    server.begin();
    Serial.println("Web 服务器启动，端口: " + String(WEB_SERVER_PORT));
}

void WebServer::handleClient() {
    server.handleClient();
}

void WebServer::setupRoutes() {
    // 主页
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    
    // API 路由
    server.on("/api/status", HTTP_GET, [this]() { handleGetStatus(); });
    server.on("/api/system", HTTP_GET, [this]() { handleGetSystemInfo(); });
    server.on("/api/timers", HTTP_GET, [this]() { handleGetTimers(); });
    server.on("/api/timers", HTTP_POST, [this]() { handleAddTimer(); });
    server.on("/api/timers/clear", HTTP_POST, [this]() { handleClearTimers(); });
    server.on("/api/pins", HTTP_GET, [this]() { handleGetPins(); });
    server.on("/api/pwm/config", HTTP_GET, [this]() { handleGetPWMConfig(); });
    server.on("/api/manual", HTTP_POST, [this]() { handleManualControl(); });
    server.on("/api/wifi", HTTP_POST, [this]() { handleWiFiConfig(); });
    server.on("/api/wifi/reset", HTTP_POST, [this]() { handleWiFiReset(); });
    server.on("/api/restart-ap", HTTP_POST, [this]() { handleRestartAP(); });
    
    // 404 处理 - 这里会处理动态路由
    server.onNotFound([this]() {
        String uri = server.uri();
        
        // 处理定时器的 PUT 和 DELETE 请求
        if (uri.startsWith("/api/timers/") && uri != "/api/timers/clear") {
            if (server.method() == HTTP_PUT) {
                handleUpdateTimer();
                return;
            } else if (server.method() == HTTP_DELETE) {
                handleDeleteTimer();
                return;
            }
        }
        
        // 真正的 404
        server.send(404, "text/plain", "Not Found");
    });
}

void WebServer::handleRoot() {
    enableCORS();
    
    // 如果设备处于AP模式且未连接WiFi，显示简化的WiFi设置页面
    if (wifiManager->isInAPMode() && !wifiManager->isConnected()) {
        server.send_P(200, "text/html", AP_MODE_HTML);
    } else {
        // 否则显示完整的控制面板
        server.send_P(200, "text/html", INDEX_HTML);
    }
}

void WebServer::handleGetStatus() {
    enableCORS();
    
    DynamicJsonDocument doc(1024);
    doc["wifiConnected"] = wifiManager->isConnected();
    doc["localIP"] = wifiManager->getLocalIP();
    doc["apIP"] = wifiManager->getAPIP();
    doc["isAPMode"] = wifiManager->isInAPMode();
    doc["hasValidTime"] = timerManager->hasValidTime();
    doc["timeSource"] = timerManager->hasValidTime() ? "NTP" : "系统运行时间";
    
    // 获取当前时间
    doc["currentTime"] = timeManager->getCurrentTimeString();
    doc["currentDate"] = timeManager->getCurrentDateString();
    
    // 统计活跃定时器
    int activeCount = 0;
    for (int i = 0; i < timerManager->getTimerCount(); i++) {
        TimerConfig timer = timerManager->getTimer(i);
        if (timer.isActive) activeCount++;
    }
    doc["activeTimers"] = activeCount;
    doc["totalTimers"] = timerManager->getTimerCount();
    
    sendJSON(doc);
}

void WebServer::handleGetSystemInfo() {
    enableCORS();
    
    DynamicJsonDocument doc(2048);
    
    // 基本系统信息
    doc["chipId"] = String(ESP.getChipId(), HEX);
    doc["flashChipId"] = String(ESP.getFlashChipId(), HEX);
    doc["flashChipSize"] = ESP.getFlashChipSize();
    doc["flashChipRealSize"] = ESP.getFlashChipRealSize();
    doc["flashChipSpeed"] = ESP.getFlashChipSpeed();
    doc["cpuFreqMHz"] = ESP.getCpuFreqMHz();
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["heapFragmentation"] = ESP.getHeapFragmentation();
    doc["maxFreeBlockSize"] = ESP.getMaxFreeBlockSize();
    doc["freeSketchSpace"] = ESP.getFreeSketchSpace();
    doc["sketchSize"] = ESP.getSketchSize();
    doc["sketchMD5"] = ESP.getSketchMD5();
    doc["resetReason"] = ESP.getResetReason();
    doc["resetInfo"] = ESP.getResetInfo();
    
    // 运行时间信息
    unsigned long uptime = millis();
    doc["uptimeMs"] = uptime;
    doc["uptimeDays"] = uptime / (24 * 60 * 60 * 1000UL);
    doc["uptimeHours"] = (uptime / (60 * 60 * 1000UL)) % 24;
    doc["uptimeMinutes"] = (uptime / (60 * 1000UL)) % 60;
    doc["uptimeSeconds"] = (uptime / 1000UL) % 60;
    
    // WiFi 详细信息
    JsonObject wifi = doc.createNestedObject("wifi");
    wifi["status"] = WiFi.status();
    wifi["statusText"] = wifiManager->isConnected() ? "已连接" : (wifiManager->isInAPMode() ? "AP模式" : "断开连接");
    wifi["mode"] = WiFi.getMode();
    wifi["modeText"] = WiFi.getMode() == WIFI_STA ? "STA" : (WiFi.getMode() == WIFI_AP ? "AP" : "AP+STA");
    
    if (wifiManager->isConnected()) {
        wifi["ssid"] = WiFi.SSID();
        wifi["bssid"] = WiFi.BSSIDstr();
        wifi["rssi"] = WiFi.RSSI();
        wifi["localIP"] = WiFi.localIP().toString();
        wifi["subnetMask"] = WiFi.subnetMask().toString();
        wifi["gatewayIP"] = WiFi.gatewayIP().toString();
        wifi["dnsIP"] = WiFi.dnsIP().toString();
        wifi["macAddress"] = WiFi.macAddress();
        wifi["channel"] = WiFi.channel();
        wifi["autoConnect"] = WiFi.getAutoConnect();
    }
    
    if (wifiManager->isInAPMode()) {
        wifi["apSSID"] = WiFi.softAPSSID();
        wifi["apIP"] = WiFi.softAPIP().toString();
        wifi["apMacAddress"] = WiFi.softAPmacAddress();
        wifi["apStationCount"] = WiFi.softAPgetStationNum();
    }
    
    // 时间信息
    JsonObject timeInfo = doc.createNestedObject("time");
    timeInfo["hasValidTime"] = timeManager->isTimeValid();
    timeInfo["timeSource"] = timeManager->isTimeValid() ? "NTP" : "系统运行时间";
    timeInfo["currentTime"] = timeManager->getCurrentTimeString();
    timeInfo["currentDate"] = timeManager->getCurrentDateString();
    timeInfo["isWiFiTimeAvailable"] = timeManager->isWiFiTimeAvailable();
    
    // 引脚状态
    JsonArray pins = doc.createNestedArray("pins");
    for (int i = 0; i < AVAILABLE_PINS_COUNT; i++) {
        JsonObject pin = pins.createNestedObject();
        pin["pin"] = AVAILABLE_PINS[i];
        pin["state"] = digitalRead(AVAILABLE_PINS[i]);
        
        // 检查是否被定时器占用
        bool inUse = false;
        int timerIndex = -1;
        for (int j = 0; j < timerManager->getTimerCount(); j++) {
            TimerConfig timer = timerManager->getTimer(j);
            if (timer.pin == AVAILABLE_PINS[i] && timer.isActive) {
                inUse = true;
                timerIndex = j;
                break;
            }
        }
        pin["inUse"] = inUse;
        pin["timerIndex"] = timerIndex;
    }
    
    // 定时器统计
    JsonObject timerStats = doc.createNestedObject("timerStats");
    timerStats["total"] = timerManager->getTimerCount();
    
    int enabled = 0, active = 0, repeatDaily = 0, oneTime = 0;
    for (int i = 0; i < timerManager->getTimerCount(); i++) {
        TimerConfig timer = timerManager->getTimer(i);
        if (timer.enabled) enabled++;
        if (timer.isActive) active++;
        if (timer.repeatDaily) repeatDaily++;
        else oneTime++;
    }
    
    timerStats["enabled"] = enabled;
    timerStats["active"] = active;
    timerStats["repeatDaily"] = repeatDaily;
    timerStats["oneTime"] = oneTime;
    timerStats["maxTimers"] = MAX_TIMERS;
    
    // EEPROM 使用情况
    JsonObject eeprom = doc.createNestedObject("eeprom");
    eeprom["size"] = EEPROM_SIZE;
    eeprom["wifiConfigStart"] = WIFI_SSID_ADDR;
    eeprom["wifiConfigSize"] = TIMER_CONFIG_ADDR - WIFI_SSID_ADDR;
    eeprom["timerConfigStart"] = TIMER_CONFIG_ADDR;
    eeprom["timerConfigUsed"] = 1 + (timerManager->getTimerCount() * 7); // 1 byte count + 7 bytes per timer
    
    sendJSON(doc);
}

void WebServer::handleGetTimers() {
    enableCORS();
    server.send(200, "application/json", timerManager->getTimersJSON());
}

void WebServer::handleAddTimer() {
    enableCORS();
    
    if (!server.hasArg("plain")) {
        sendJSON(400, "缺少请求数据", false);
        return;
    }
    
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    
    int pin = doc["pin"];
    int hour = doc["hour"];
    int minute = doc["minute"];
    int duration = doc["duration"];
    bool repeatDaily = doc["repeatDaily"].as<bool>();
    bool isPWM = doc["isPWM"].as<bool>();
    int pwmValue = doc["pwmValue"].is<int>() ? doc["pwmValue"].as<int>() : 512; // 默认值50%
    
    if (timerManager->addTimer(pin, hour, minute, duration, repeatDaily, isPWM, pwmValue)) {
        sendJSON(200, "定时器添加成功");
    } else {
        sendJSON(400, "定时器添加失败，请检查参数", false);
    }
}

void WebServer::handleUpdateTimer() {
    enableCORS();
    
    String uri = server.uri();
    int lastSlash = uri.lastIndexOf('/');
    if (lastSlash == -1) {
        sendJSON(400, "无效的请求路径", false);
        return;
    }
    
    int index = uri.substring(lastSlash + 1).toInt();
    
    if (!server.hasArg("plain")) {
        sendJSON(400, "缺少请求数据", false);
        return;
    }
    
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    
    int pin = doc["pin"];
    int hour = doc["hour"];
    int minute = doc["minute"];
    int duration = doc["duration"];
    bool enabled = doc["enabled"];
    bool repeatDaily = doc["repeatDaily"].as<bool>();
    bool isPWM = doc["isPWM"].as<bool>();
    int pwmValue = doc["pwmValue"].is<int>() ? doc["pwmValue"].as<int>() : 512; // 默认值50%
    
    if (timerManager->updateTimer(index, pin, hour, minute, duration, enabled, repeatDaily, isPWM, pwmValue)) {
        sendJSON(200, "定时器更新成功");
    } else {
        sendJSON(400, "定时器更新失败", false);
    }
}

void WebServer::handleDeleteTimer() {
    enableCORS();
    
    String uri = server.uri();
    int lastSlash = uri.lastIndexOf('/');
    if (lastSlash == -1) {
        sendJSON(400, "无效的请求路径", false);
        return;
    }
    
    int index = uri.substring(lastSlash + 1).toInt();
    
    if (timerManager->removeTimer(index)) {
        sendJSON(200, "定时器删除成功");
    } else {
        sendJSON(400, "定时器删除失败", false);
    }
}

void WebServer::handleClearTimers() {
    enableCORS();
    
    timerManager->clearAllTimers();
    sendJSON(200, "所有定时器已清除");
}

void WebServer::handleGetPins() {
    enableCORS();
    server.send(200, "application/json", timerManager->getAvailablePinsJSON());
}

void WebServer::handleManualControl() {
    enableCORS();
    
    if (!server.hasArg("plain")) {
        sendJSON(400, "缺少请求数据", false);
        return;
    }
    
    DynamicJsonDocument doc(512);
    deserializeJson(doc, server.arg("plain"));
    
    int pin = doc["pin"];
    int duration = doc["duration"];
    bool isPWM = doc["isPWM"].as<bool>();
    int pwmValue = doc["pwmValue"].is<int>() ? doc["pwmValue"].as<int>() : 512; // 默认值50%
    
    timerManager->executeManualControl(pin, duration, isPWM, pwmValue);
    sendJSON(200, "手动控制执行成功");
}

void WebServer::handleWiFiConfig() {
    enableCORS();
    
    if (!server.hasArg("plain")) {
        sendJSON(400, "缺少请求数据", false);
        return;
    }
    
    DynamicJsonDocument doc(512);
    deserializeJson(doc, server.arg("plain"));
    
    String ssid = doc["ssid"];
    String password = doc["password"];
    
    if (ssid.length() == 0) {
        sendJSON(400, "SSID 不能为空", false);
        return;
    }
    
    // 保存 WiFi 凭据
    wifiManager->saveWiFiCredentials(ssid, password);
    
    sendJSON(200, "WiFi 配置已保存，设备将重启");
    
    // 延迟重启以确保响应发送完成
    delay(1000);
    ESP.restart();
}

void WebServer::handleWiFiReset() {
    enableCORS();
    
    // 清除 WiFi 凭据
    wifiManager->saveWiFiCredentials("", "");
    
    sendJSON(200, "WiFi 设置已重置，设备将重启为 AP 模式");
    
    // 延迟重启
    delay(1000);
    ESP.restart();
}

void WebServer::handleRestartAP() {
    enableCORS();
    
    // 重启为AP模式（清除WiFi凭据并重启）
    wifiManager->saveWiFiCredentials("", "");
    
    sendJSON(200, "设备将重启为热点模式");
    
    // 延迟重启
    delay(1000);
    ESP.restart();
}

void WebServer::sendJSON(int code, const String& message, bool success) {
    DynamicJsonDocument doc(512);
    doc["success"] = success;
    doc["message"] = message;
    
    String response;
    serializeJson(doc, response);
    
    server.send(code, "application/json", response);
}

void WebServer::sendJSON(DynamicJsonDocument& doc) {
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebServer::enableCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    if (server.method() == HTTP_OPTIONS) {
        server.send(204);
    }
}

void WebServer::handleGetPWMConfig() {
    enableCORS();
    
    DynamicJsonDocument doc(512);
    doc["frequency"] = PWM_FREQUENCY;
    doc["resolution"] = PWM_RESOLUTION;
    doc["maxValue"] = PWM_MAX_VALUE;
    doc["minValue"] = 0;
    doc["defaultValue"] = PWM_MAX_VALUE / 2; // 50%
    
    sendJSON(doc);
}
