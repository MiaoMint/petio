#ifndef CONFIG_H
#define CONFIG_H

// 固件版本信息
#define FIRMWARE_VERSION "v2.1.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__
#define FIRMWARE_NAME "PetIO"

// WiFi 配置
#define DEFAULT_AP_SSID "PetIO_Setup"
#define WIFI_TIMEOUT 30000 // 30秒

// NTP 时间同步配置
#define NTP_SERVER "pool.ntp.org"
#define TIME_ZONE 8                 // UTC+8 中国时区
#define NTP_UPDATE_INTERVAL 3600000 // 1小时同步一次

// Web 服务器端口
#define WEB_SERVER_PORT 80

// EEPROM 地址配置
#define EEPROM_SIZE 512
#define WIFI_SSID_ADDR 0
#define WIFI_PASSWORD_ADDR 64
#define TIMER_CONFIG_ADDR 128

// 定时器配置
#define MAX_TIMERS 10
#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64

// ESP8266 可用引脚列表
const int AVAILABLE_PINS[] = {0, 1, 2, 3, 12, 13, 14, 15, 16};
const int AVAILABLE_PINS_COUNT = sizeof(AVAILABLE_PINS) / sizeof(AVAILABLE_PINS[0]);

// PWM 配置
#define PWM_FREQUENCY 1000      // PWM 频率 1KHz
#define PWM_RESOLUTION 10       // PWM 分辨率 10位 (0-1023)
#define PWM_MAX_VALUE 1023      // PWM 最大值

// 定时器结构体
struct TimerConfig
{
    bool enabled;
    int pin;
    int hour;
    int minute;
    float duration;               // 持续时间（秒，支持小数）
    bool repeatDaily;             // 每天重复
    bool isActive;                // 当前是否激活
    unsigned long startTime;      // 开始时间（millis时间戳）
    unsigned long lastTriggerDay; // 上次触发的天数（用于每天重复检查）
    bool isPWM;                   // 是否为PWM模式
    int pwmValue;                 // PWM值 (0-1023)
    unsigned long realStartTime;  // 真实开始时间（时间戳）
};

#endif
