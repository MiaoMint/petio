# PetIO 控制系统

一个基于 ESP8266 的智能引脚定时控制系统，支持 Web 界面管理和 WiFi 配置。

## 功能特性

### 🎯 定时器管理
- ✅ 设置任意引脚在指定时间点开启指定时长
- ✅ 支持多个定时器同时运行
- ✅ 可启用/禁用单个定时器
- ✅ **每天重复功能** - 可设置定时器每天自动重复
- ✅ **单次执行功能** - 执行一次后自动禁用
- ✅ 定时器状态实时显示
- ✅ 持久化配置储存
- ✅ **NTP 时间同步** - WiFi 连接时自动同步网络时间

### 🔌 引脚控制
- ✅ 支持所有 ESP8266 可用引脚 (0,1,2,3,4,5,12,13,14,15,16)
- ✅ 实时显示引脚状态
- ✅ 手动开关控制
- ✅ 定时开启功能
- ✅ 引脚占用状态提示

### 📶 WiFi 管理
- ✅ 自动连接已保存的 WiFi
- ✅ **无密码 AP 模式** - 无配置时自动启动无密码热点
- ✅ Web 界面配置 WiFi
- ✅ 支持重置为 AP 模式
- ✅ 连接状态监控和自动重连
- ✅ **智能功能限制** - AP 模式下自动隐藏定时器功能

### 🌐 Web 界面
- ✅ 响应式设计，支持手机和电脑
- ✅ 实时数据更新
- ✅ 现代化 UI 设计
- ✅ 多标签页管理
- ✅ 操作反馈和状态提示
- ✅ **改进的时间选择器** - 使用 HTML5 time 输入控件
- ✅ **智能功能显隐** - 根据连接状态自动调整界面

## 硬件要求

- ESP8266 开发板 (ESP-12E/NodeMCU/Wemos D1 等)
- 连接到引脚的负载设备（继电器、LED、水泵等）

## 快速开始

### 1. 编译和上传
```bash
# 使用 PlatformIO 编译并上传
pio run --target upload

# 或使用 Arduino IDE
# 选择板子: ESP8266 -> Generic ESP8266 Module
# 上传速度: 921600
# Flash Size: 4MB
```

### 2. 首次配置
1. 上传程序后，ESP8266 会自动启动 AP 模式
2. 用手机或电脑连接热点：
   - **SSID**: `PetIO_Setup`
   - **密码**: 无密码（开放网络）
3. 打开浏览器访问：`http://192.168.4.1`
4. 在 "WiFi 设置" 标签页配置你的 WiFi
5. 配置成功后设备会自动重启并连接到你的 WiFi

### 3. 正常使用
- WiFi 连接成功后，可通过设备的 IP 地址访问控制界面
- 串口监视器会显示设备的访问地址
- **注意**: 只有在 WiFi 连接状态下，定时器功能才能正常工作（需要 NTP 时间同步）

## API 接口文档

### 系统状态
```
GET /api/status
返回: {
  "wifiConnected": boolean,
  "localIP": "192.168.1.100",
  "apIP": "192.168.4.1",
  "isAPMode": boolean,
  "currentTime": "12:30",
  "activeTimers": 2,
  "totalTimers": 5
}
```

### 定时器管理
```
# 获取所有定时器
GET /api/timers

# 添加定时器
POST /api/timers
Body: {
  "pin": 2,
  "hour": 12,
  "minute": 30,
  "duration": 60,
  "repeatDaily": true
}

# 更新定时器
PUT /api/timers/{index}
Body: {
  "pin": 2,
  "hour": 12,
  "minute": 30,
  "duration": 60,
  "enabled": true,
  "repeatDaily": true
}

# 删除定时器
DELETE /api/timers/{index}

# 清除所有定时器
POST /api/timers/clear
```

### 引脚控制
```
# 获取引脚状态
GET /api/pins

# 手动控制
POST /api/manual
Body: {
  "pin": 2,
  "duration": 0  // 0=切换状态，>0=开启指定秒数
}
```

### WiFi 配置
```
# 保存WiFi配置
POST /api/wifi
Body: {
  "ssid": "My_WiFi",
  "password": "password123"
}

# 重置为AP模式
POST /api/wifi/reset
```

## 引脚说明

### ESP8266 可用引脚
| 引脚 | 说明 | 注意事项 |
|------|------|----------|
| 0    | GPIO0 | 启动时需要为 HIGH |
| 1    | TX    | 串口发送，调试时避免使用 |
| 2    | GPIO2 | 启动时需要为 HIGH，有板载 LED |
| 3    | RX    | 串口接收，调试时避免使用 |
| 4    | GPIO4 | 安全的通用 IO |
| 5    | GPIO5 | 安全的通用 IO |
| 12   | GPIO12| 安全的通用 IO |
| 13   | GPIO13| 安全的通用 IO |
| 14   | GPIO14| 安全的通用 IO |
| 15   | GPIO15| 启动时需要为 LOW |
| 16   | GPIO16| 无中断功能，适合简单输出 |

### 推荐用法
- **GPIO 4, 5, 12, 13, 14**: 最安全的选择，适合连接继电器等设备
- **GPIO 2**: 适合连接 LED 指示灯
- **GPIO 16**: 适合简单的开关控制

## 项目结构

```
src/
├── main.cpp            # 主程序入口
├── config.h            # 配置文件和常量定义
├── wifi_manager.h/cpp  # WiFi 连接管理
├── timer_manager.h/cpp # 定时器功能管理
├── time_manager.h/cpp  # NTP 时间同步管理
├── web_server.h/cpp    # Web 服务器和 API
└── web_pages.h         # HTML 页面模板
```

## 自定义配置

### 修改默认 AP 信息
在 `config.h` 中修改：
```cpp
#define DEFAULT_AP_SSID "Your_AP_Name"
#define DEFAULT_AP_PASSWORD ""  // 保持为空表示无密码
```

### 调整时间同步设置
```cpp
#define NTP_SERVER "your.ntp.server"
#define TIME_ZONE 8  // 修改为你的时区
#define NTP_UPDATE_INTERVAL 3600000  // 同步间隔（毫秒）
```

### 调整定时器数量
```cpp
#define MAX_TIMERS 20  // 增加到 20 个定时器
```

### 添加新引脚
```cpp
const int AVAILABLE_PINS[] = {0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17};
```

## 故障排除

### 1. 无法连接 WiFi
- 检查 SSID 和密码是否正确
- 确认路由器是 2.4GHz 频段
- 尝试重置为 AP 模式重新配置

### 2. 定时器不工作
- 检查系统时间是否正确（当前为简化版本，基于运行时间）
- 确认定时器已启用
- 检查引脚连接

### 3. 网页无法访问
- 确认设备已连接到正确的网络
- 检查 IP 地址是否正确
- 尝试重启设备

### 4. 引脚无响应
- 检查硬件连接
- 确认引脚编号正确
- 检查负载是否正常

## 开发和扩展

### 添加新功能
项目采用模块化设计，可以轻松添加新功能：

1. **定时功能**: 修改 `timer_manager.cpp`
2. **网页界面**: 修改 `web_pages.h`
3. **API 接口**: 修改 `web_server.cpp`
4. **WiFi 功能**: 修改 `wifi_manager.cpp`

### 调试模式
启用详细日志输出：
```cpp
// 在 Serial.begin() 后添加
Serial.setDebugOutput(true);
```

## 许可证

MIT License - 可自由使用、修改和分发。

## 贡献

欢迎提交 Issue 和 Pull Request！

---

**享受你的智能 PetIO 控制系统！** 🐾
