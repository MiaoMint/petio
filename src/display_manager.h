#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "wifi_manager.h"
#include "timer_manager.h"
#include "time_manager.h"

// 默认 I2C 地址通常为 0x3C；若你的模块丝印为 0x3D，请在 config.h 中覆盖
#ifndef OLED_I2C_ADDRESS
#define OLED_I2C_ADDRESS 0x3C
#endif

// 默认 I2C 引脚（ESP8266: ··）
#ifndef OLED_SDA_PIN
#define OLED_SDA_PIN 4
#endif
#ifndef OLED_SCL_PIN
#define OLED_SCL_PIN 5
#endif

class DisplayManager {   
public:
  DisplayManager(WiFiManager* wm, TimerManager* tm, TimeManager* tim);
  bool begin();
  void update();

private:
  WiFiManager* wifi;
  TimerManager* timers;
  TimeManager* timeM;
  U8G2_SSD1315_128X64_NONAME_F_HW_I2C u8g2; // 全缓冲，硬件 I2C
  unsigned long lastDraw = 0;
  const unsigned long drawInterval = 1000; // 1s 刷新
  uint8_t pageIndex = 0;               // 0:网络 1:任务 2:运行
  unsigned long lastPageSwitch = 0;
  const unsigned long pageInterval = 3000; // 3s 轮播

  void drawScreen();
  void drawPageNetwork();
  void drawPageTimers();
  void drawPageUptime();
  String uptimeStr();
};

#endif // DISPLAY_MANAGER_H
