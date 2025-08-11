#include "display_manager.h"
#include <Wire.h>

DisplayManager::DisplayManager(WiFiManager *wm, TimerManager *tm, TimeManager *tim)
    : wifi(wm), timers(tm), timeM(tim),
      u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE) {}

bool DisplayManager::begin()
{
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    u8g2.setI2CAddress(OLED_I2C_ADDRESS << 1); // U8g2 使用 8-bit 地址
    u8g2.begin();
    u8g2.setContrast(200);
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_6x12_tf); // 默认 ASCII 字体，兼容性更好

    // 开机画面
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy12_t_chinese1);
    u8g2.drawStr(0, 12, "PetIO Booting...");
    u8g2.drawStr(0, 28, "OLED init...");
    u8g2.sendBuffer();
    return true;
}

void DisplayManager::update()
{
    unsigned long now = millis();
    if (now - lastDraw >= drawInterval)
    {
        drawScreen();
        lastDraw = now;
    }

    if (now - lastPageSwitch >= pageInterval)
    {
        pageIndex = (pageIndex + 1) % 3;
        lastPageSwitch = now;
    }
}

String DisplayManager::uptimeStr()
{
    unsigned long ms = millis();
    unsigned long s = ms / 1000UL;
    unsigned int d = s / 86400UL;
    unsigned int h = (s / 3600UL) % 24;
    unsigned int m = (s / 60UL) % 60;
    unsigned int sec = s % 60;
    char buf[32];
    if (d > 0)
        sprintf(buf, "%ud %02u:%02u:%02u", d, h, m, sec);
    else
        sprintf(buf, "%02u:%02u:%02u", h, m, sec);
    return String(buf);
}

void DisplayManager::drawScreen()
{
    u8g2.clearBuffer();
    switch (pageIndex)
    {
    case 0:
        drawPageNetwork();
        break;
    case 1:
        drawPageTimers();
        break;
    case 2:
        drawPageUptime();
        break;
    }
    u8g2.sendBuffer();
}

void DisplayManager::drawPageNetwork()
{
    // 标题
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "Network");
    // SSID 或 AP 名
    String line1 = wifi->isConnected() ? String("SSID ") + WiFi.SSID() : String("AP ") + DEFAULT_AP_SSID;
    u8g2.drawUTF8(0, 24, line1.c_str());
    // IP
    String ip = wifi->isConnected() ? wifi->getLocalIP() : wifi->getAPIP();
    String ipLine = String("IP  ") + ip;
    u8g2.drawUTF8(0, 38, ipLine.c_str());
    // RSSI/Mode
    String line3;
    if (wifi->isConnected())
        line3 = String("RSSI ") + WiFi.RSSI() + " dBm";
    else
        line3 = "Mode AP";
    u8g2.drawUTF8(0, 52, line3.c_str());
}

void DisplayManager::drawPageTimers()
{
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "Timers");
    int total = timers->getTimerCount();
    int y = 24;
    for (int i = 0; i < total && i < 3; i++)
    { // 最多展示3条
        TimerConfig t = timers->getTimer(i);
        char buf[32];
        snprintf(buf, sizeof(buf), "P%d %02d:%02d %ds %s", t.pin, t.hour, t.minute, t.duration, t.repeatDaily ? "R" : "1");
        u8g2.drawStr(0, y, buf);
        y += 12;
    }
    char stat[24];
    int active = 0;
    for (int i = 0; i < total; i++)
    {
        if (timers->getTimer(i).isActive)
            active++;
    }
    snprintf(stat, sizeof(stat), "%d active / %d", active, total);
    int w = u8g2.getStrWidth(stat);
    u8g2.drawStr(128 - w, 64, stat);
}

void DisplayManager::drawPageUptime()
{
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "System");

    String timeStr = timeM->getCurrentTimeString();
    String dateStr = timeM->getCurrentDateString();
    String timeLine = String("Time ") + timeStr;
    String dateLine = String("Date ") + dateStr;
    String upLine = String("Up   ") + uptimeStr();

    u8g2.drawUTF8(0, 24, timeLine.c_str());
    u8g2.drawUTF8(0, 38, dateLine.c_str());
    u8g2.drawUTF8(0, 52, upLine.c_str());
}