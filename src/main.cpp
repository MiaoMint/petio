#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "timer_manager.h"
#include "time_manager.h"
#include "web_server.h"
// #include "display_manager.h"

// 全局对象
WiFiManager wifiManager;
TimerManager timerManager;
TimeManager timeManager;
WebServer webServer(&wifiManager, &timerManager, &timeManager);
// DisplayManager display(&wifiManager, &timerManager, &timeManager);

// 状态变量
unsigned long lastUpdate = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastTimeUpdate = 0;
const unsigned long UPDATE_INTERVAL = 10;         // 10毫秒 - 高精度定时器更新
const unsigned long WIFI_CHECK_INTERVAL = 30000;  // 30秒
const unsigned long TIME_UPDATE_INTERVAL = 10000; // 10秒

void setup()
{
  Serial.begin(115200);
  delay(1000); // 给串口时间稳定
  Serial.println();
  Serial.println("=================================");
  Serial.println("🐾 PetIO 控制系统启动中...");
  Serial.println("=================================");

  // // 初始化 OLED 显示
  // Serial.println("🖥️ 初始化 OLED 显示屏...");
  // display.begin();
  // Serial.println("✅ OLED 初始化完成!");

  // 初始化时间管理器
  Serial.println("🕐 初始化时间管理器...");
  timeManager.begin();
  Serial.println("✅ 时间管理器初始化完成!");

  // 初始化定时器管理器
  Serial.println("⏰ 初始化定时器管理器...");
  timerManager.begin(&timeManager);
  Serial.println("✅ 定时器管理器初始化完成!");

  // 初始化 WiFi 管理器
  Serial.println("📶 初始化 WiFi 管理器...");
  bool wifiConnected = wifiManager.begin();
  Serial.println("✅ WiFi 管理器初始化完成!");

  // 初始化 Web 服务器
  Serial.println("🌐 启动 Web 服务器...");
  webServer.begin();
  Serial.println("✅ Web 服务器启动完成!");

  // 启动信息
  Serial.println("=================================");
  Serial.println("✅ 系统启动完成！");
  Serial.println("=================================");

  if (wifiConnected)
  {
    Serial.println("🌍 WiFi 模式 - 可通过以下地址访问:");
    Serial.println("   http://" + wifiManager.getLocalIP());
    Serial.println("🕐 NTP 时间同步已启用");
  }
  else
  {
    Serial.println("📡 AP 模式 - 请连接以下热点:");
    Serial.println("   SSID: " + String(DEFAULT_AP_SSID));
    Serial.println("   密码: 无密码");
    Serial.println("   然后访问: http://" + wifiManager.getAPIP());
    Serial.println("⚠️  AP 模式下定时器功能受限，请连接 WiFi 以启用完整功能");
  }

  Serial.println("=================================");
  Serial.println();

  // 显示可用引脚信息
  Serial.print("🔌 可用引脚: ");
  for (int i = 0; i < AVAILABLE_PINS_COUNT; i++)
  {
    Serial.print(AVAILABLE_PINS[i]);
    if (i < AVAILABLE_PINS_COUNT - 1)
      Serial.print(", ");
  }
  Serial.println();

  // 显示定时器信息
  Serial.println("⏰ 已加载定时器数量: " + String(timerManager.getTimerCount()));

  Serial.println();
  Serial.println("📝 系统日志:");
  Serial.println("----------------------------------------");
}

void loop()
{
  unsigned long currentTime = millis();

  // 处理 Web 请求 (优先级最高)
  webServer.handleClient();

  // 高频更新定时器 (10ms间隔，确保定时器精度)
  if (currentTime - lastUpdate >= UPDATE_INTERVAL)
  {
    timerManager.update();
    lastUpdate = currentTime;
  }

  // 定期更新时间同步
  if (currentTime - lastTimeUpdate >= TIME_UPDATE_INTERVAL)
  {
    timeManager.update();
    lastTimeUpdate = currentTime;
  }

  // 定期检查 WiFi 连接
  if (currentTime - lastWiFiCheck >= WIFI_CHECK_INTERVAL)
  {
    wifiManager.handleWiFiConnection();
    lastWiFiCheck = currentTime;
  }

  // display.update();

  // 让系统有时间处理其他任务
  yield();
}
