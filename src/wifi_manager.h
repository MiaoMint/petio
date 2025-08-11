#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "config.h"

class WiFiManager {
private:
    String savedSSID;
    String savedPassword;
    bool isAPMode;
    
public:
    WiFiManager();
    bool begin();
    void setupAP();
    bool connectToWiFi(const String& ssid, const String& password);
    void saveWiFiCredentials(const String& ssid, const String& password);
    String loadWiFiSSID();
    String loadWiFiPassword();
    bool isConnected();
    bool isInAPMode();
    String getLocalIP();
    String getAPIP();
    void handleWiFiConnection();
};

#endif
