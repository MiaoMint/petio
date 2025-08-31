#include "timer_manager.h"

TimerManager::TimerManager() {
    timerCount = 0;
    timeManager = nullptr;
    lastStateSave = 0;
}

void TimerManager::begin(TimeManager* tm) {
    timeManager = tm;
    
    // 初始化EEPROM
    EEPROM.begin(EEPROM_SIZE);
    
    // 初始化所有可用引脚为输出模式
    for (int i = 0; i < AVAILABLE_PINS_COUNT; i++) {
        pinMode(AVAILABLE_PINS[i], OUTPUT);
        digitalWrite(AVAILABLE_PINS[i], LOW);
        // 为PWM引脚设置频率
        analogWriteFreq(PWM_FREQUENCY);
        analogWriteResolution(PWM_RESOLUTION);
    }
    
    loadTimers();
    Serial.println("Timer Manager 初始化完成，已加载 " + String(timerCount) + " 个定时器");
    
    // 输出恢复的状态信息
    int activeCount = 0;
    for (int i = 0; i < timerCount; i++) {
        if (timers[i].isActive) {
            activeCount++;
        }
    }
    if (activeCount > 0) {
        Serial.println("已恢复 " + String(activeCount) + " 个活跃定时器状态");
    }
}

void TimerManager::update() {
    if (!timeManager) return;
    
    unsigned long currentTime = millis();
    int currentHour = timeManager->getCurrentHour();
    int currentMinute = timeManager->getCurrentMinute();
    unsigned long currentDay = timeManager->getCurrentDay();
    
    bool stateChanged = false;
    
    for (int i = 0; i < timerCount; i++) {
        if (!timers[i].enabled) continue;
        
        // 检查是否到达触发时间
        bool shouldTrigger = false;
        
        if (!timers[i].isActive && 
            timers[i].hour == currentHour && 
            timers[i].minute == currentMinute) {
            
            if (timers[i].repeatDaily) {
                // 每天重复：检查今天是否已经触发过
                if (timers[i].lastTriggerDay != currentDay) {
                    shouldTrigger = true;
                    timers[i].lastTriggerDay = currentDay;
                }
            } else {
                // 单次触发
                shouldTrigger = true;
            }
        }
        
        if (shouldTrigger) {
            timers[i].isActive = true;
            timers[i].startTime = currentTime;
            // 保存真实时间戳（如果可用）
            if (timeManager->isTimeValid()) {
                timers[i].realStartTime = timeManager->getEpochTime();
            } else {
                timers[i].realStartTime = 0;
            }
            setPin(timers[i].pin, HIGH, timers[i].isPWM ? timers[i].pwmValue : 0);
            
            String modeStr = timers[i].isPWM ? " PWM(" + String(timers[i].pwmValue) + ")" : "";
            Serial.println("定时器 " + String(i) + " 激活，引脚 " + String(timers[i].pin) + " 开启" + modeStr +
                          (timers[i].repeatDaily ? " (每天重复)" : " (单次)") + 
                          ", 预期运行时间: " + String(timers[i].duration * 1000.0) + "ms");
            
            stateChanged = true;
            
            // 如果是单次定时器，触发后自动禁用
            if (!timers[i].repeatDaily) {
                timers[i].enabled = false;
                saveTimers(); // 立即保存配置更改
            }
        }
        
        // 检查是否需要关闭
        if (timers[i].isActive && 
            currentTime - timers[i].startTime >= (unsigned long)(timers[i].duration * 1000.0 + 0.5)) {
            
            timers[i].isActive = false;
            timers[i].realStartTime = 0; // 清理真实时间戳
            setPin(timers[i].pin, LOW, 0);
            
            Serial.println("定时器 " + String(i) + " 完成，引脚 " + String(timers[i].pin) + " 关闭，实际运行时间: " + String(currentTime - timers[i].startTime) + "ms");
            stateChanged = true;
        }
    }
    
    // 定期保存状态，或者有状态变化时立即保存
    if (stateChanged || (currentTime - lastStateSave >= STATE_SAVE_INTERVAL)) {
        saveTimerStates();
        lastStateSave = currentTime;
    }
}

bool TimerManager::addTimer(int pin, int hour, int minute, float duration, bool repeatDaily, bool isPWM, int pwmValue) {
    if (timerCount >= MAX_TIMERS) {
        return false;
    }
    
    // 验证引脚是否可用
    bool pinValid = false;
    for (int i = 0; i < AVAILABLE_PINS_COUNT; i++) {
        if (AVAILABLE_PINS[i] == pin) {
            pinValid = true;
            break;
        }
    }
    if (!pinValid) return false;
    
    // 验证时间
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || duration <= 0.0) {
        return false;
    }
    
    // 验证PWM值
    if (isPWM && (pwmValue < 0 || pwmValue > PWM_MAX_VALUE)) {
        return false;
    }
    
    timers[timerCount].enabled = true;
    timers[timerCount].pin = pin;
    timers[timerCount].hour = hour;
    timers[timerCount].minute = minute;
    timers[timerCount].duration = duration;
    timers[timerCount].repeatDaily = repeatDaily;
    timers[timerCount].isActive = false;
    timers[timerCount].startTime = 0;
    timers[timerCount].lastTriggerDay = 0; // 初始化为0
    timers[timerCount].isPWM = isPWM;
    timers[timerCount].pwmValue = isPWM ? pwmValue : 0;
    timers[timerCount].realStartTime = 0; // 初始化真实时间戳
    
    timerCount++;
    saveTimers();
    
    String modeStr = isPWM ? " PWM模式, 值=" + String(pwmValue) : " 数字模式";
    Serial.println("添加定时器：引脚 " + String(pin) + ", 时间 " + String(hour) + ":" + String(minute) + 
                  ", 持续 " + String(duration) + "秒" + modeStr + (repeatDaily ? " (每天重复)" : " (单次)"));
    
    return true;
}

bool TimerManager::removeTimer(int index) {
    if (index < 0 || index >= timerCount) {
        return false;
    }
    
    // 如果定时器正在运行，先关闭引脚
    if (timers[index].isActive) {
        setPin(timers[index].pin, LOW, 0);
    }
    
    // 移动数组元素
    for (int i = index; i < timerCount - 1; i++) {
        timers[i] = timers[i + 1];
    }
    
    timerCount--;
    saveTimers();
    
    Serial.println("删除定时器 " + String(index));
    
    return true;
}

bool TimerManager::updateTimer(int index, int pin, int hour, int minute, float duration, bool enabled, bool repeatDaily, bool isPWM, int pwmValue) {
    if (index < 0 || index >= timerCount) {
        return false;
    }
    
    // 验证引脚
    bool pinValid = false;
    for (int i = 0; i < AVAILABLE_PINS_COUNT; i++) {
        if (AVAILABLE_PINS[i] == pin) {
            pinValid = true;
            break;
        }
    }
    if (!pinValid) return false;
    
    // 验证时间
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || duration <= 0.0) {
        return false;
    }
    
    // 验证PWM值
    if (isPWM && (pwmValue < 0 || pwmValue > PWM_MAX_VALUE)) {
        return false;
    }
    
    // 如果定时器正在运行且引脚发生变化，先关闭旧引脚
    if (timers[index].isActive && timers[index].pin != pin) {
        setPin(timers[index].pin, LOW, 0);
        timers[index].isActive = false;
    }
    
    timers[index].enabled = enabled;
    timers[index].pin = pin;
    timers[index].hour = hour;
    timers[index].minute = minute;
    timers[index].duration = duration;
    timers[index].repeatDaily = repeatDaily;
    timers[index].isPWM = isPWM;
    timers[index].pwmValue = isPWM ? pwmValue : 0;
    
    // 如果修改了重复设置，重置触发状态
    timers[index].lastTriggerDay = 0;
    
    saveTimers();
    
    Serial.println("更新定时器 " + String(index));
    
    return true;
}

String TimerManager::getTimersJSON() {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    for (int i = 0; i < timerCount; i++) {
        JsonObject timer = array.add<JsonObject>();
        timer["index"] = i;
        timer["enabled"] = timers[i].enabled;
        timer["pin"] = timers[i].pin;
        timer["hour"] = timers[i].hour;
        timer["minute"] = timers[i].minute;
        timer["duration"] = timers[i].duration;
        timer["repeatDaily"] = timers[i].repeatDaily;
        timer["isActive"] = timers[i].isActive;
        timer["isPWM"] = timers[i].isPWM;
        timer["pwmValue"] = timers[i].pwmValue;
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

String TimerManager::getAvailablePinsJSON() {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    for (int i = 0; i < AVAILABLE_PINS_COUNT; i++) {
        JsonObject pin = array.add<JsonObject>();
        pin["pin"] = AVAILABLE_PINS[i];
        pin["state"] = digitalRead(AVAILABLE_PINS[i]);
        
        // 检查是否被定时器占用
        bool inUse = false;
        for (int j = 0; j < timerCount; j++) {
            if (timers[j].pin == AVAILABLE_PINS[i] && timers[j].isActive) {
                inUse = true;
                break;
            }
        }
        pin["inUse"] = inUse;
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

void TimerManager::executeManualControl(int pin, float duration, bool isPWM, int pwmValue) {
    // 验证引脚
    bool pinValid = false;
    for (int i = 0; i < AVAILABLE_PINS_COUNT; i++) {
        if (AVAILABLE_PINS[i] == pin) {
            pinValid = true;
            break;
        }
    }
    if (!pinValid) return;
    
    // 验证PWM值
    if (isPWM && (pwmValue < 0 || pwmValue > PWM_MAX_VALUE)) {
        return;
    }
    
    if (duration > 0.0) {
        // 开启引脚指定时间
        setPin(pin, HIGH, isPWM ? pwmValue : 0);
        String modeStr = isPWM ? " PWM模式, 值=" + String(pwmValue) : " 数字模式";
        Serial.println("手动控制：引脚 " + String(pin) + " 开启 " + String(duration) + " 秒" + modeStr);
        
        // 这里应该使用定时器来关闭，简化处理
        delay((unsigned long)(duration * 1000.0));
        setPin(pin, LOW, 0);
        Serial.println("手动控制：引脚 " + String(pin) + " 关闭");
    } else {
        // 切换状态
        bool currentState = digitalRead(pin);
        if (isPWM) {
            // PWM模式切换
            if (currentState) {
                setPin(pin, LOW, 0);
                Serial.println("手动控制：引脚 " + String(pin) + " PWM关闭");
            } else {
                setPin(pin, HIGH, pwmValue);
                Serial.println("手动控制：引脚 " + String(pin) + " PWM开启, 值=" + String(pwmValue));
            }
        } else {
            // 数字模式切换
            setPin(pin, !currentState, 0);
            Serial.println("手动控制：引脚 " + String(pin) + " 数字切换到 " + String(!currentState));
        }
    }
}

void TimerManager::saveTimers() {
    int addr = TIMER_CONFIG_ADDR;
    
    // 保存定时器数量
    EEPROM.write(addr++, timerCount);
    
    // 保存每个定时器
    for (int i = 0; i < timerCount; i++) {
        EEPROM.write(addr++, timers[i].enabled ? 1 : 0);
        EEPROM.write(addr++, timers[i].pin);
        EEPROM.write(addr++, timers[i].hour);
        EEPROM.write(addr++, timers[i].minute);
        
        // 保存float类型的duration（4字节）
        union {
            float f;
            uint8_t bytes[4];
        } durationConverter;
        durationConverter.f = timers[i].duration;
        EEPROM.write(addr++, durationConverter.bytes[0]);
        EEPROM.write(addr++, durationConverter.bytes[1]);
        EEPROM.write(addr++, durationConverter.bytes[2]);
        EEPROM.write(addr++, durationConverter.bytes[3]);
        
        EEPROM.write(addr++, timers[i].repeatDaily ? 1 : 0);
        EEPROM.write(addr++, timers[i].isPWM ? 1 : 0);
        EEPROM.write(addr++, (timers[i].pwmValue >> 8) & 0xFF);
        EEPROM.write(addr++, timers[i].pwmValue & 0xFF);
        
        // 保存运行时状态
        EEPROM.write(addr++, timers[i].isActive ? 1 : 0);
        
        // 保存开始时间（4字节）
        unsigned long startTime = timers[i].startTime;
        EEPROM.write(addr++, (startTime >> 24) & 0xFF);
        EEPROM.write(addr++, (startTime >> 16) & 0xFF);
        EEPROM.write(addr++, (startTime >> 8) & 0xFF);
        EEPROM.write(addr++, startTime & 0xFF);
        
        // 保存上次触发天数（4字节）
        unsigned long lastTriggerDay = timers[i].lastTriggerDay;
        EEPROM.write(addr++, (lastTriggerDay >> 24) & 0xFF);
        EEPROM.write(addr++, (lastTriggerDay >> 16) & 0xFF);
        EEPROM.write(addr++, (lastTriggerDay >> 8) & 0xFF);
        EEPROM.write(addr++, lastTriggerDay & 0xFF);
        
        // 保存真实开始时间（4字节）
        unsigned long realStartTime = timers[i].realStartTime;
        EEPROM.write(addr++, (realStartTime >> 24) & 0xFF);
        EEPROM.write(addr++, (realStartTime >> 16) & 0xFF);
        EEPROM.write(addr++, (realStartTime >> 8) & 0xFF);
        EEPROM.write(addr++, realStartTime & 0xFF);
    }
    
    EEPROM.commit();
}

void TimerManager::loadTimers() {
    int addr = TIMER_CONFIG_ADDR;
    
    // 读取定时器数量
    timerCount = EEPROM.read(addr++);
    if (timerCount > MAX_TIMERS) timerCount = 0;
    
    // 读取每个定时器
    for (int i = 0; i < timerCount; i++) {
        timers[i].enabled = EEPROM.read(addr++) == 1;
        timers[i].pin = EEPROM.read(addr++);
        timers[i].hour = EEPROM.read(addr++);
        timers[i].minute = EEPROM.read(addr++);
        
        // 读取float类型的duration（4字节）
        union {
            float f;
            uint8_t bytes[4];
        } durationConverter;
        durationConverter.bytes[0] = EEPROM.read(addr++);
        durationConverter.bytes[1] = EEPROM.read(addr++);
        durationConverter.bytes[2] = EEPROM.read(addr++);
        durationConverter.bytes[3] = EEPROM.read(addr++);
        timers[i].duration = durationConverter.f;
        
        timers[i].repeatDaily = EEPROM.read(addr++) == 1;
        
        // 检查是否有PWM数据（向后兼容）
        if (addr < EEPROM_SIZE - 20) { // 更新了字节数计算：duration现在是4字节而不是2字节
            timers[i].isPWM = EEPROM.read(addr++) == 1;
            int pwmHigh = EEPROM.read(addr++);
            int pwmLow = EEPROM.read(addr++);
            timers[i].pwmValue = (pwmHigh << 8) | pwmLow;
            
            // 读取运行时状态
            timers[i].isActive = EEPROM.read(addr++) == 1;
            
            // 读取开始时间（4字节）
            unsigned long startTime = 0;
            startTime |= ((unsigned long)EEPROM.read(addr++)) << 24;
            startTime |= ((unsigned long)EEPROM.read(addr++)) << 16;
            startTime |= ((unsigned long)EEPROM.read(addr++)) << 8;
            startTime |= (unsigned long)EEPROM.read(addr++);
            timers[i].startTime = startTime;
            
            // 读取上次触发天数（4字节）
            unsigned long lastTriggerDay = 0;
            lastTriggerDay |= ((unsigned long)EEPROM.read(addr++)) << 24;
            lastTriggerDay |= ((unsigned long)EEPROM.read(addr++)) << 16;
            lastTriggerDay |= ((unsigned long)EEPROM.read(addr++)) << 8;
            lastTriggerDay |= (unsigned long)EEPROM.read(addr++);
            timers[i].lastTriggerDay = lastTriggerDay;
            
            // 读取真实开始时间（4字节）
            unsigned long realStartTime = 0;
            realStartTime |= ((unsigned long)EEPROM.read(addr++)) << 24;
            realStartTime |= ((unsigned long)EEPROM.read(addr++)) << 16;
            realStartTime |= ((unsigned long)EEPROM.read(addr++)) << 8;
            realStartTime |= (unsigned long)EEPROM.read(addr++);
            timers[i].realStartTime = realStartTime;
            
        } else {
            // 旧数据没有PWM和运行时状态信息，设为默认值
            timers[i].isPWM = false;
            timers[i].pwmValue = 0;
            timers[i].isActive = false;
            timers[i].startTime = 0;
            timers[i].lastTriggerDay = 0;
            timers[i].realStartTime = 0;
        }
        
        // 恢复活跃定时器的引脚状态
        if (timers[i].isActive) {
            // 使用真实时间检查定时器是否应该仍然活跃
            if (timeManager && timeManager->isTimeValid() && timers[i].realStartTime > 0) {
                // 需要添加获取当前时间戳的方法到TimeManager
                // 暂时使用millis()逻辑，但添加更好的时间处理
                unsigned long currentTime = millis();
                
                // 重置开始时间为当前时间减去已经运行的时间
                // 这样可以在重启后继续正确计时
                if (timers[i].startTime > currentTime) {
                    // millis() 已重置，重新计算开始时间
                    timers[i].startTime = currentTime;
                    Serial.println("重启后调整定时器 " + String(i) + " 开始时间");
                }
                
                unsigned long elapsedTime = currentTime - timers[i].startTime;
                
                // 检查定时器是否已经超时
                if (elapsedTime >= (unsigned long)(timers[i].duration * 1000.0)) {
                    // 定时器已经超时，关闭它
                    timers[i].isActive = false;
                    timers[i].realStartTime = 0;
                    setPin(timers[i].pin, LOW, 0);
                    Serial.println("重启后发现定时器 " + String(i) + " 已超时，关闭引脚 " + String(timers[i].pin));
                } else {
                    // 定时器仍然有效，恢复引脚状态
                    setPin(timers[i].pin, HIGH, timers[i].isPWM ? timers[i].pwmValue : 0);
                    String modeStr = timers[i].isPWM ? " PWM(" + String(timers[i].pwmValue) + ")" : "";
                    unsigned long remainingTime = (unsigned long)(timers[i].duration * 1000.0) - elapsedTime;
                    Serial.println("恢复定时器 " + String(i) + " 状态，引脚 " + String(timers[i].pin) + " 开启" + modeStr + 
                                  "，剩余时间: " + String(remainingTime / 1000) + "秒");
                }
            } else {
                // 没有有效的时间或者是旧格式数据，保守处理
                unsigned long currentTime = millis();
                timers[i].startTime = currentTime;
                setPin(timers[i].pin, HIGH, timers[i].isPWM ? timers[i].pwmValue : 0);
                String modeStr = timers[i].isPWM ? " PWM(" + String(timers[i].pwmValue) + ")" : "";
                Serial.println("恢复定时器 " + String(i) + " 状态，引脚 " + String(timers[i].pin) + " 开启" + modeStr + 
                              "（重启后重新计时）");
            }
        }
    }
}

bool TimerManager::hasValidTime() {
    return timeManager && timeManager->isTimeValid();
}

void TimerManager::saveTimerStates() {
    // 只保存运行时状态，不保存完整配置
    // 这样可以减少EEPROM写入次数，延长寿命
    int addr = TIMER_CONFIG_ADDR + 1; // 跳过定时器数量
    
    for (int i = 0; i < timerCount; i++) {
        // 跳过基本配置部分 (12字节: enabled(1) + pin(1) + hour(1) + minute(1) + duration(4) + repeatDaily(1) + isPWM(1) + pwmValue(2))
        addr += 12;
        
        // 更新运行时状态部分 (13字节)
        EEPROM.write(addr++, timers[i].isActive ? 1 : 0);
        
        // 更新开始时间（4字节）
        unsigned long startTime = timers[i].startTime;
        EEPROM.write(addr++, (startTime >> 24) & 0xFF);
        EEPROM.write(addr++, (startTime >> 16) & 0xFF);
        EEPROM.write(addr++, (startTime >> 8) & 0xFF);
        EEPROM.write(addr++, startTime & 0xFF);
        
        // 更新上次触发天数（4字节）
        unsigned long lastTriggerDay = timers[i].lastTriggerDay;
        EEPROM.write(addr++, (lastTriggerDay >> 24) & 0xFF);
        EEPROM.write(addr++, (lastTriggerDay >> 16) & 0xFF);
        EEPROM.write(addr++, (lastTriggerDay >> 8) & 0xFF);
        EEPROM.write(addr++, lastTriggerDay & 0xFF);
        
        // 更新真实开始时间（4字节）
        unsigned long realStartTime = timers[i].realStartTime;
        EEPROM.write(addr++, (realStartTime >> 24) & 0xFF);
        EEPROM.write(addr++, (realStartTime >> 16) & 0xFF);
        EEPROM.write(addr++, (realStartTime >> 8) & 0xFF);
        EEPROM.write(addr++, realStartTime & 0xFF);
    }
    
    EEPROM.commit();
}

void TimerManager::clearAllTimers() {
    // 关闭所有激活的引脚
    for (int i = 0; i < timerCount; i++) {
        if (timers[i].isActive) {
            setPin(timers[i].pin, LOW, 0);
        }
    }
    
    timerCount = 0;
    saveTimers();
    Serial.println("所有定时器已清除");
}

int TimerManager::getTimerCount() {
    return timerCount;
}

TimerConfig TimerManager::getTimer(int index) {
    if (index >= 0 && index < timerCount) {
        return timers[index];
    }
    
    TimerConfig empty = {false, 0, 0, 0, 0.0, false, false, 0, 0UL, false, 0, 0UL};
    return empty;
}

void TimerManager::setPin(int pin, bool state, int pwmValue) {
    if (pwmValue > 0 && state) {
        // PWM 模式
        analogWrite(pin, pwmValue);
    } else {
        // 数字模式
        digitalWrite(pin, state ? HIGH : LOW);
    }
}
