#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "config.h"
#include "time_manager.h"

class TimerManager {
private:
    TimerConfig timers[MAX_TIMERS];
    int timerCount;
    TimeManager* timeManager;
    unsigned long lastStateSave;
    const unsigned long STATE_SAVE_INTERVAL = 30000; // 30秒保存一次状态
    
public:
    TimerManager();
    void begin(TimeManager* tm);
    void update();
    bool addTimer(int pin, int hour, int minute, float duration, bool repeatDaily = false, bool isPWM = false, int pwmValue = 512);
    bool removeTimer(int index);
    bool updateTimer(int index, int pin, int hour, int minute, float duration, bool enabled, bool repeatDaily = false, bool isPWM = false, int pwmValue = 512);
    String getTimersJSON();
    void saveTimers();
    void saveTimerStates(); // 新增：仅保存运行时状态
    void loadTimers();
    void clearAllTimers();
    int getTimerCount();
    TimerConfig getTimer(int index);
    void setPin(int pin, bool state, int pwmValue = 0);
    String getAvailablePinsJSON();
    void executeManualControl(int pin, float duration, bool isPWM = false, int pwmValue = 512);
    bool hasValidTime();
};

#endif
