#include "wifi_manager.h"

WiFiManager::WiFiManager() {
    isAPMode = false;
}

bool WiFiManager::begin() {
    EEPROM.begin(EEPROM_SIZE);
    
    savedSSID = loadWiFiSSID();
    savedPassword = loadWiFiPassword();
    
    Serial.println("WiFi Manager 初始化");
    Serial.println("保存的 SSID: " + savedSSID);
    
    // 如果有保存的 WiFi 信息，尝试连接
    if (savedSSID.length() > 0) {
        Serial.println("尝试连接到保存的 WiFi...");
        if (connectToWiFi(savedSSID, savedPassword)) {
            Serial.println("连接成功！IP: " + WiFi.localIP().toString());
            return true;
        }
    }
    
    // 连接失败或没有保存的信息，启动 AP 模式
    Serial.println("启动 AP 模式");
    setupAP();
    return false;
}

void WiFiManager::setupAP() {
    WiFi.mode(WIFI_AP);
    // AP 模式无密码，更容易连接
    WiFi.softAP(DEFAULT_AP_SSID);
    isAPMode = true;
    
    Serial.println("AP 模式启动");
    Serial.println("SSID: " + String(DEFAULT_AP_SSID));
    Serial.println("密码: 无密码");
    Serial.println("AP IP: " + WiFi.softAPIP().toString());
}

bool WiFiManager::connectToWiFi(const String& ssid, const String& password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        isAPMode = false;
        return true;
    }
    
    return false;
}

void WiFiManager::saveWiFiCredentials(const String& ssid, const String& password) {
    // 清空 EEPROM 区域
    for (int i = 0; i < MAX_SSID_LENGTH + MAX_PASSWORD_LENGTH; i++) {
        EEPROM.write(WIFI_SSID_ADDR + i, 0);
    }
    
    // 保存 SSID
    for (int i = 0; i < ssid.length() && i < MAX_SSID_LENGTH - 1; i++) {
        EEPROM.write(WIFI_SSID_ADDR + i, ssid[i]);
    }
    
    // 保存 Password
    for (int i = 0; i < password.length() && i < MAX_PASSWORD_LENGTH - 1; i++) {
        EEPROM.write(WIFI_PASSWORD_ADDR + i, password[i]);
    }
    
    EEPROM.commit();
    
    savedSSID = ssid;
    savedPassword = password;
    
    Serial.println("WiFi 凭据已保存");
}

String WiFiManager::loadWiFiSSID() {
    String ssid = "";
    for (int i = 0; i < MAX_SSID_LENGTH; i++) {
        char c = EEPROM.read(WIFI_SSID_ADDR + i);
        if (c == 0) break;
        ssid += c;
    }
    return ssid;
}

String WiFiManager::loadWiFiPassword() {
    String password = "";
    for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) {
        char c = EEPROM.read(WIFI_PASSWORD_ADDR + i);
        if (c == 0) break;
        password += c;
    }
    return password;
}

bool WiFiManager::isConnected() {
    return !isAPMode && WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::isInAPMode() {
    return isAPMode;
}

String WiFiManager::getLocalIP() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "";
}

String WiFiManager::getAPIP() {
    if (isAPMode) {
        return WiFi.softAPIP().toString();
    }
    return "";
}

void WiFiManager::handleWiFiConnection() {
    if (!isAPMode && WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi 连接丢失，重新连接...");
        if (!connectToWiFi(savedSSID, savedPassword)) {
            Serial.println("重连失败，启动 AP 模式");
            setupAP();
        }
    }
}
