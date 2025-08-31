#include "web_server.h"
#include "config.h"
#include <ESP8266HTTPUpdateServer.h>
#include <Updater.h>

ESP8266HTTPUpdateServer httpUpdater;

WebServer::WebServer(WiFiManager* wm, TimerManager* tm, TimeManager* timeM) : server(WEB_SERVER_PORT) {
    wifiManager = wm;
    timerManager = tm;
    timeManager = timeM;
}

void WebServer::begin() {
    // 设置最大上传文件大小为 2MB
    server.setContentLength(2 * 1024 * 1024);
    
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
    
    // 固件更新测试端点
    server.on("/api/firmware/info", HTTP_GET, [this]() {
        enableCORS();
        JsonDocument doc;
        doc["version"] = "1.0.0";
        doc["buildTime"] = __DATE__ " " __TIME__;
        doc["chipId"] = String(ESP.getChipId(), HEX);
        doc["flashSize"] = ESP.getFlashChipSize();
        doc["freeSpace"] = ESP.getFreeSketchSpace();
        sendJSON(doc);
    });
    
    server.on("/api/firmware/update", HTTP_POST, 
        [this]() { 
            // 处理上传完成后的响应
            enableCORS();
            if (Update.hasError()) {
                sendJSON(500, "固件更新失败", false);
            } else {
                sendJSON(200, "固件更新成功，设备即将重启");
                delay(1000);
                ESP.restart();
            }
        },
        [this]() { 
            // 处理文件上传过程
            handleFirmwareUpdate(); 
        }
    );
    
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
    
    JsonDocument doc;
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
    
    JsonDocument doc;
    
    // 固件信息
    doc["firmwareVersion"] = FIRMWARE_VERSION;
    doc["firmwareName"] = FIRMWARE_NAME;
    doc["buildDate"] = BUILD_DATE;
    doc["buildTime"] = BUILD_TIME;
    doc["buildDateTime"] = String(BUILD_DATE) + " " + String(BUILD_TIME);
    
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
    
        // WiFi 信息
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["connected"] = wifiManager->isConnected();
    wifi["localIP"] = wifiManager->getLocalIP();
    wifi["macAddress"] = WiFi.macAddress();
    wifi["apIP"] = wifiManager->getAPIP();
    wifi["isAPMode"] = wifiManager->isInAPMode();
    
    // WiFi状态信息
    wifi["status"] = WiFi.status();
    if (wifiManager->isConnected()) {
        wifi["statusText"] = "已连接";
        wifi["modeText"] = "Station模式";
        wifi["rssi"] = WiFi.RSSI();
        wifi["ssid"] = WiFi.SSID();
    } else if (wifiManager->isInAPMode()) {
        wifi["statusText"] = "AP模式";
        wifi["modeText"] = "热点模式";
        wifi["rssi"] = 0;
        wifi["ssid"] = "未连接";
        wifi["apSSID"] = DEFAULT_AP_SSID;
        wifi["apStationCount"] = WiFi.softAPgetStationNum();
    } else {
        wifi["statusText"] = "未连接";
        wifi["modeText"] = "未知";
        wifi["rssi"] = 0;
        wifi["ssid"] = "未连接";
    }
    
    // 时间信息
    JsonObject timeInfo = doc["time"].to<JsonObject>();
    timeInfo["currentTime"] = timeManager->getCurrentTimeString();
    timeInfo["currentDate"] = timeManager->getCurrentDateString();
    timeInfo["hasValidTime"] = timeManager->isTimeValid();
    timeInfo["isValid"] = timeManager->isTimeValid();
    timeInfo["timeSource"] = timeManager->isTimeValid() ? "NTP网络时间" : "系统运行时间";
    timeInfo["ntpEnabled"] = wifiManager->isConnected();
    timeInfo["uptime"] = millis() / 1000; // 运行时间（秒）
    
    // 引脚状态
    JsonArray pins = doc["pins"].to<JsonArray>();
    for (int i = 0; i < AVAILABLE_PINS_COUNT; i++) {
        JsonObject pin = pins.add<JsonObject>();
        pin["pin"] = AVAILABLE_PINS[i];      // 前端期望的字段名
        pin["number"] = AVAILABLE_PINS[i];   // 保持兼容
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
    JsonObject timerStats = doc["timerStats"].to<JsonObject>();
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
    JsonObject eeprom = doc["eeprom"].to<JsonObject>();
    eeprom["size"] = EEPROM_SIZE;
    eeprom["wifiConfigStart"] = WIFI_SSID_ADDR;
    eeprom["wifiConfigSize"] = TIMER_CONFIG_ADDR - WIFI_SSID_ADDR;
    eeprom["timerConfigStart"] = TIMER_CONFIG_ADDR;
    eeprom["timerConfigUsed"] = 1 + (timerManager->getTimerCount() * 25); // 1 byte count + 25 bytes per timer (12 config + 13 runtime)
    
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
    
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    
    int pin = doc["pin"];
    int hour = doc["hour"];
    int minute = doc["minute"];
    float duration = doc["duration"];
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
    
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    
    int pin = doc["pin"];
    int hour = doc["hour"];
    int minute = doc["minute"];
    float duration = doc["duration"];
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
    
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    
    int pin = doc["pin"];
    float duration = doc["duration"];
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
    
    JsonDocument doc;
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
    JsonDocument doc;
    doc["success"] = success;
    doc["message"] = message;
    
    String response;
    serializeJson(doc, response);
    
    server.send(code, "application/json", response);
}

void WebServer::sendJSON(JsonDocument& doc) {
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
    
    JsonDocument doc;
    doc["frequency"] = PWM_FREQUENCY;
    doc["resolution"] = PWM_RESOLUTION;
    doc["maxValue"] = PWM_MAX_VALUE;
    doc["minValue"] = 0;
    doc["defaultValue"] = PWM_MAX_VALUE / 2; // 50%
    
    sendJSON(doc);
}

void WebServer::handleFirmwareUpdate() {
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("固件更新开始: %s\n", upload.filename.c_str());
        
        // 检查文件扩展名
        if (!upload.filename.endsWith(".bin")) {
            Serial.println("错误：不支持的文件格式");
            return;
        }
        
        // 开始OTA更新
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) {
            Serial.println("错误：无法开始固件更新");
            Update.printError(Serial);
            return;
        }
        Serial.println("固件更新已开始...");
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
        // 写入固件数据
        Serial.printf("写入 %u 字节...\n", upload.currentSize);
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Serial.println("错误：固件写入失败");
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END) {
        // 完成固件更新
        if (Update.end(true)) {
            Serial.printf("固件更新成功: %u 字节\n", upload.totalSize);
        } else {
            Serial.println("错误：固件更新完成失败");
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.end();
        Serial.println("固件更新被中止");
    }
}
