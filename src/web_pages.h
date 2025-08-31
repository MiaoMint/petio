#ifndef WEB_PAGES_H
#define WEB_PAGES_H

// AP模式专用页面 - 无需外部CSS，仅WiFi设置
const char AP_MODE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PetIO WiFi设置</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background-color: #f3f4f6;
            color: #374151;
            line-height: 1.6;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .container {
            max-width: 400px;
            width: 100%;
            background: white;
            border-radius: 12px;
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 24px;
            text-align: center;
        }
        .header h1 {
            font-size: 24px;
            font-weight: bold;
            margin-bottom: 8px;
        }
        .header p {
            opacity: 0.9;
            font-size: 14px;
        }
        .content {
            padding: 32px 24px;
        }
        .form-group {
            margin-bottom: 24px;
        }
        .form-group label {
            display: block;
            font-weight: 600;
            margin-bottom: 8px;
            color: #374151;
        }
        .form-group input {
            width: 100%;
            padding: 12px 16px;
            border: 2px solid #e5e7eb;
            border-radius: 8px;
            font-size: 16px;
            transition: border-color 0.2s;
        }
        .form-group input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        .btn {
            width: 100%;
            padding: 14px 20px;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
            text-align: center;
            display: inline-block;
            text-decoration: none;
        }
        .btn-primary {
            background: #667eea;
            color: white;
            margin-bottom: 12px;
        }
        .btn-primary:hover {
            background: #5a6fd8;
            transform: translateY(-1px);
        }
        .btn-secondary {
            background: #f59e0b;
            color: white;
        }
        .btn-secondary:hover {
            background: #d97706;
            transform: translateY(-1px);
        }
        .alert {
            padding: 16px;
            border-radius: 8px;
            margin-bottom: 24px;
            font-size: 14px;
        }
        .alert-info {
            background: #dbeafe;
            color: #1e40af;
            border: 1px solid #93c5fd;
        }
        .alert-success {
            background: #d1fae5;
            color: #065f46;
            border: 1px solid #6ee7b7;
        }
        .alert-error {
            background: #fee2e2;
            color: #991b1b;
            border: 1px solid #fca5a5;
        }
        .hidden {
            display: none !important;
        }
        .status-badge {
            display: inline-flex;
            align-items: center;
            padding: 6px 12px;
            background: #fbbf24;
            color: #92400e;
            border-radius: 20px;
            font-size: 12px;
            font-weight: 600;
            margin-bottom: 16px;
        }
        .footer {
            text-align: center;
            padding: 16px 24px;
            background: #f9fafb;
            border-top: 1px solid #e5e7eb;
            color: #6b7280;
            font-size: 12px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🐾 PetIO</h1>
            <p>WiFi网络配置</p>
        </div>
        
        <div class="content">
            <div class="status-badge">
                📶 AP模式 - 需要连接WiFi
            </div>
            
            <div id="wifi-message" class="hidden"></div>
            
            <div class="alert alert-info">
                💡 设备当前处于热点模式。请连接到您的WiFi网络以启用完整功能，包括定时器和时间同步。
            </div>
            
            <form id="wifi-form" onsubmit="saveWiFi(event)">
                <div class="form-group">
                    <label for="wifi-ssid">网络名称 (SSID)</label>
                    <input type="text" id="wifi-ssid" name="ssid" placeholder="输入WiFi网络名称" maxlength="31" required>
                </div>
                
                <div class="form-group">
                    <label for="wifi-password">网络密码</label>
                    <input type="password" id="wifi-password" name="password" placeholder="输入WiFi密码 (可选)" maxlength="63">
                </div>
                
                <button type="submit" class="btn btn-primary">
                    💾 保存并连接WiFi
                </button>
                
                <button type="button" class="btn btn-secondary" onclick="restartAP()">
                    🔄 重启热点模式
                </button>
            </form>
        </div>
        
        <div class="footer">
            连接WiFi后将自动跳转到完整控制面板
        </div>
    </div>

    <script>
        function showMessage(message, type = 'info') {
            const messageDiv = document.getElementById('wifi-message');
            messageDiv.className = `alert alert-${type}`;
            messageDiv.textContent = message;
            messageDiv.classList.remove('hidden');
            
            if (type === 'success') {
                setTimeout(() => {
                    messageDiv.classList.add('hidden');
                }, 3000);
            }
        }

        async function saveWiFi(event) {
            event.preventDefault();
            
            const ssid = document.getElementById('wifi-ssid').value.trim();
            const password = document.getElementById('wifi-password').value;
            
            if (!ssid) {
                showMessage('请输入WiFi网络名称', 'error');
                return;
            }
            
            try {
                showMessage('正在连接WiFi，请稍候...', 'info');
                
                const response = await fetch('/api/wifi', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({ ssid, password })
                });
                
                const result = await response.json();
                
                if (result.success) {
                    showMessage('WiFi配置已保存！设备正在重启并连接网络...', 'success');
                    // 15秒后尝试重定向到设备的新IP
                    setTimeout(() => {
                        if (result.ip) {
                            window.location.href = `http://${result.ip}`;
                        } else {
                            window.location.reload();
                        }
                    }, 15000);
                } else {
                    showMessage(result.message || 'WiFi配置失败，请检查网络名称和密码', 'error');
                }
            } catch (error) {
                console.error('WiFi配置错误:', error);
                showMessage('网络错误，请重试', 'error');
            }
        }

        async function restartAP() {
            try {
                showMessage('正在重启为热点模式...', 'info');
                
                const response = await fetch('/api/restart-ap', {
                    method: 'POST'
                });
                
                if (response.ok) {
                    showMessage('设备正在重启为热点模式...', 'success');
                    setTimeout(() => {
                        window.location.reload();
                    }, 5000);
                } else {
                    showMessage('重启失败，请重试', 'error');
                }
            } catch (error) {
                console.error('重启错误:', error);
                showMessage('网络错误，请重试', 'error');
            }
        }

        // 页面加载时检查连接状态
        document.addEventListener('DOMContentLoaded', function() {
            // 定期检查是否已连接WiFi
            const checkConnection = async () => {
                try {
                    const response = await fetch('/api/status');
                    const status = await response.json();
                    
                    if (status.wifiConnected) {
                        showMessage('WiFi连接成功！正在跳转到完整界面...', 'success');
                        setTimeout(() => {
                            window.location.href = '/';
                        }, 2000);
                    }
                } catch (error) {
                    // 忽略错误，继续检查
                }
            };
            
            // 每5秒检查一次连接状态
            setInterval(checkConnection, 5000);
        });
    </script>
</body>
</html>
)rawliteral";

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PetIO 控制面板</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        /* 保持字体平滑 */
        body {
            -webkit-font-smoothing: antialiased;
            -moz-osx-font-smoothing: grayscale;
        }
        /* 自定义活动标签下划线动画 */
        .tab.active {
            position: relative;
        }
        .tab.active::after {
            content: '';
            position: absolute;
            bottom: -1px;
            left: 0;
            right: 0;
            height: 2px;
            background-color: #4f46e5; /* indigo-600 */
            animation: underline 0.3s ease-out;
        }
        @keyframes underline {
            from { width: 0; }
            to { width: 100%; }
        }
        /* 简单的淡入动画 */
        .fade-in {
            animation: fadeIn 0.5s ease-in-out;
        }
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .hidden {
            display: none;
        }
    </style>
</head>
<body class="bg-gray-100 text-gray-800 font-sans">

    <div class="container mx-auto max-w-7xl p-4">
        
        <header class="bg-white rounded-xl shadow-sm p-6 mb-6">
            <div class="flex justify-between items-center">
                <div>
                    <h1 class="text-2xl font-bold text-gray-900">🐾 PetIO 控制面板</h1>
                    <p class="text-sm text-gray-500 mt-1">一个智能宠物喂养定时器</p>
                </div>
                <div id="connection-status" class="text-sm font-medium text-right">
                    <span class="inline-flex items-center px-2.5 py-0.5 rounded-full bg-gray-100 text-gray-800">
                        检测中...
                    </span>
                </div>
            </div>
            <div id="device-info" class="text-xs text-gray-500 mt-4 border-t border-gray-200 pt-2"></div>
        </header>

        <main>
            <div class="bg-white rounded-xl shadow-sm">
                <div class="border-b border-gray-200">
                    <nav class="flex space-x-2 px-4" aria-label="Tabs">
                        <button class="tab active" onclick="switchTab('timers', this)" id="timers-tab">定时器</button>
                        <button class="tab" onclick="switchTab('manual', this)">手动控制</button>
                        <button class="tab" onclick="switchTab('system', this)">系统状态</button>
                        <button class="tab" onclick="switchTab('wifi', this)">WiFi设置</button>
                        <button class="tab" onclick="switchTab('firmware', this)">固件更新</button>
                    </nav>
                </div>

                <div class="p-6">
                    <!-- 定时器管理 -->
                    <div id="timers-content" class="tab-content fade-in">
                        <div id="ap-mode-notice" class="hidden p-4 mb-4 text-sm text-yellow-800 rounded-lg bg-yellow-50 border border-yellow-200" role="alert">
                            <span class="font-medium">⚠️ AP 模式提醒：</span> 当前设备处于 AP 模式，无法同步网络时间。请先连接到 WiFi 网络以启用定时器功能。
                        </div>
                        
                        <div class="grid grid-cols-1 lg:grid-cols-3 gap-6">
                            <div class="lg:col-span-1 space-y-6">
                                <div id="timer-form" class="p-5 border rounded-lg bg-gray-50">
                                    <h3 class="text-lg font-semibold mb-4 text-gray-800">添加新定时器</h3>
                                    <div id="timer-message" class="hidden mb-4"></div>
                                    <div class="space-y-4">
                                        <div>
                                            <label for="timer-pin" class="block text-sm font-medium text-gray-700 mb-1">选择引脚</label>
                                            <select id="timer-pin" class="w-full p-2 border border-gray-300 rounded-md shadow-sm focus:ring-indigo-500 focus:border-indigo-500"></select>
                                        </div>
                                        <div>
                                            <label for="timer-time" class="block text-sm font-medium text-gray-700 mb-1">触发时间</label>
                                            <input type="time" id="timer-time" value="12:00" class="w-full p-2 border border-gray-300 rounded-md shadow-sm focus:ring-indigo-500 focus:border-indigo-500">
                                        </div>
                                        <div>
                                            <label for="timer-duration" class="block text-sm font-medium text-gray-700 mb-1">持续时间 (秒)</label>
                                            <input type="number" id="timer-duration" max="86400" value="0.3" class="w-full p-2 border border-gray-300 rounded-md shadow-sm focus:ring-indigo-500 focus:border-indigo-500">
                                        </div>
                                        <div class="flex items-center">
                                            <input type="checkbox" id="timer-repeat" checked class="h-4 w-4 text-indigo-600 border-gray-300 rounded focus:ring-indigo-500">
                                            <label for="timer-repeat" class="ml-2 block text-sm text-gray-900">每天重复执行</label>
                                        </div>
                                        <div>
                                            <label class="flex items-center space-x-2">
                                                <input type="checkbox" id="timer-pwm-mode" onchange="toggleTimerPWMMode()" class="h-4 w-4 text-indigo-600 focus:ring-indigo-500 border-gray-300 rounded">
                                                <span class="text-sm font-medium text-gray-700">PWM 模式</span>
                                            </label>
                                        </div>
                                        <div id="timer-pwm-controls" class="hidden">
                                            <label for="timer-pwm-value" class="block text-sm font-medium text-gray-700 mb-1">PWM 强度: <span id="timer-pwm-percentage">50%</span></label>
                                            <input type="range" id="timer-pwm-value" min="0" max="1023" value="512" 
                                                   oninput="updateTimerPWMPercentage(this.value)" 
                                                   class="w-full h-2 bg-gray-200 rounded-lg appearance-none cursor-pointer">
                                            <div class="flex justify-between text-xs text-gray-500 mt-1">
                                                <span>0%</span>
                                                <span>25%</span>
                                                <span>50%</span>
                                                <span>75%</span>
                                                <span>100%</span>
                                            </div>
                                        </div>
                                        <button class="w-full bg-indigo-600 text-white font-semibold py-2 px-4 rounded-md hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-indigo-500 transition-colors" onclick="addTimer()">
                                            🕐 添加定时器
                                        </button>
                                    </div>
                                </div>
                                <button class="w-full bg-red-50 text-red-700 font-semibold py-2 px-4 rounded-md hover:bg-red-100 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-red-500 transition-colors" onclick="clearAllTimers()">
                                    清除所有定时器
                                </button>
                            </div>
                            <div id="timer-list-container" class="lg:col-span-2">
                                <h3 class="text-lg font-semibold mb-4 text-gray-800">现有定时器</h3>
                                <div class="timer-list space-y-3 max-h-[500px] overflow-y-auto pr-2" id="timer-list">
                                    <div class="loading-placeholder text-center p-10 text-gray-500">加载中...</div>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- 手动控制 -->
                    <div id="manual-content" class="tab-content hidden fade-in">
                        <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
                            <div>
                                <h3 class="text-lg font-semibold mb-4 text-gray-800">🔌 引脚快速控制</h3>
                                <div id="pins-grid" class="grid grid-cols-3 sm:grid-cols-4 md:grid-cols-5 gap-3">
                                    <div class="loading-placeholder text-center p-10 text-gray-500">加载中...</div>
                                </div>
                                <p class="text-xs text-gray-500 text-center mt-3">点击引脚卡片可快速切换状态</p>
                            </div>
                            <div class="p-5 border rounded-lg bg-gray-50 h-fit">
                                <h3 class="text-lg font-semibold mb-4 text-gray-800">⚙️ 精确控制</h3>
                                <div id="manual-message" class="hidden mb-4"></div>
                                <div class="space-y-4">
                                    <div>
                                        <label for="manual-pin" class="block text-sm font-medium text-gray-700 mb-1">选择引脚</label>
                                        <select id="manual-pin" class="w-full p-2 border border-gray-300 rounded-md shadow-sm focus:ring-indigo-500 focus:border-indigo-500"></select>
                                    </div>
                                    <div>
                                        <label for="manual-duration" class="block text-sm font-medium text-gray-700 mb-1">持续时间 (0=切换)</label>
                                        <input type="number" id="manual-duration"  max="86400" value="0" class="w-full p-2 border border-gray-300 rounded-md shadow-sm focus:ring-indigo-500 focus:border-indigo-500">
                                    </div>
                                    <div>
                                        <label class="flex items-center space-x-2">
                                            <input type="checkbox" id="manual-pwm-mode" onchange="togglePWMMode()" class="h-4 w-4 text-indigo-600 focus:ring-indigo-500 border-gray-300 rounded">
                                            <span class="text-sm font-medium text-gray-700">PWM 模式</span>
                                        </label>
                                    </div>
                                    <div id="pwm-controls" class="hidden">
                                        <label for="manual-pwm-value" class="block text-sm font-medium text-gray-700 mb-1">PWM 强度: <span id="pwm-percentage">50%</span></label>
                                        <input type="range" id="manual-pwm-value" min="0" max="1023" value="512" 
                                               oninput="updatePWMPercentage(this.value)" 
                                               class="w-full h-2 bg-gray-200 rounded-lg appearance-none cursor-pointer">
                                        <div class="flex justify-between text-xs text-gray-500 mt-1">
                                            <span>0%</span>
                                            <span>25%</span>
                                            <span>50%</span>
                                            <span>75%</span>
                                            <span>100%</span>
                                        </div>
                                    </div>
                                    <button class="w-full bg-indigo-600 text-white font-semibold py-2 px-4 rounded-md hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-indigo-500 transition-colors" onclick="manualControl()">
                                        🎛️ 执行控制
                                    </button>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- 系统状态 -->
                    <div id="system-content" class="tab-content hidden fade-in space-y-6">
                        <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
                            <div class="bg-gray-50 p-4 rounded-lg border border-gray-200">
                                <label class="block text-xs text-gray-500 font-medium mb-1">芯片 ID</label>
                                <span id="chip-id" class="text-base font-semibold text-gray-800">加载中...</span>
                            </div>
                            <div class="bg-gray-50 p-4 rounded-lg border border-gray-200">
                                <label class="block text-xs text-gray-500 font-medium mb-1">CPU 频率</label>
                                <span id="cpu-freq" class="text-base font-semibold text-gray-800">加载中...</span>
                            </div>
                            <div class="bg-gray-50 p-4 rounded-lg border border-gray-200">
                                <label class="block text-xs text-gray-500 font-medium mb-1">Flash 大小</label>
                                <span id="flash-size" class="text-base font-semibold text-gray-800">加载中...</span>
                            </div>
                            <div class="bg-gray-50 p-4 rounded-lg border border-gray-200">
                                <label class="block text-xs text-gray-500 font-medium mb-1">可用内存</label>
                                <span id="free-heap" class="text-base font-semibold text-gray-800">加载中...</span>
                            </div>
                            <div class="bg-gray-50 p-4 rounded-lg border border-gray-200">
                                <label class="block text-xs text-gray-500 font-medium mb-1">内存碎片化</label>
                                <span id="heap-frag" class="text-base font-semibold text-gray-800">加载中...</span>
                            </div>
                            <div class="bg-gray-50 p-4 rounded-lg border border-gray-200">
                                <label class="block text-xs text-gray-500 font-medium mb-1">运行时间</label>
                                <span id="uptime" class="text-base font-semibold text-gray-800">加载中...</span>
                            </div>
                        </div>
                        
                        <div>
                            <h3 class="text-lg font-semibold mb-3 text-gray-800">📶 WiFi 详情</h3>
                            <div id="wifi-details" class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">加载中...</div>
                        </div>
                        <div>
                            <h3 class="text-lg font-semibold mb-3 text-gray-800">🕐 时间信息</h3>
                            <div id="time-details" class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">加载中...</div>
                        </div>
                        <div>
                            <h3 class="text-lg font-semibold mb-3 text-gray-800">⏰ 定时器统计</h3>
                            <div id="timer-stats" class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">加载中...</div>
                        </div>
                        <div>
                            <h3 class="text-lg font-semibold mb-3 text-gray-800">💾 存储信息</h3>
                            <div id="storage-details" class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">加载中...</div>
                        </div>
                        <div>
                            <h3 class="text-lg font-semibold mb-3 text-gray-800">🔌 引脚状态详情</h3>
                            <div id="pin-details" class="grid grid-cols-3 sm:grid-cols-5 md:grid-cols-8 gap-3">加载中...</div>
                        </div>
                    </div>

                    <!-- WiFi 设置 -->
                    <div id="wifi-content" class="tab-content hidden fade-in">
                        <div class="max-w-md mx-auto p-5 border rounded-lg bg-gray-50">
                            <h3 class="text-lg font-semibold mb-4 text-gray-800">📶 WiFi 网络设置</h3>
                            <div id="wifi-message" class="hidden mb-4"></div>
                            <div class="space-y-4">
                                <div>
                                    <label for="wifi-ssid" class="block text-sm font-medium text-gray-700 mb-1">网络名称 (SSID)</label>
                                    <input type="text" id="wifi-ssid" placeholder="输入 WiFi 网络名称" maxlength="31" class="w-full p-2 border border-gray-300 rounded-md shadow-sm focus:ring-indigo-500 focus:border-indigo-500">
                                </div>
                                <div>
                                    <label for="wifi-password" class="block text-sm font-medium text-gray-700 mb-1">网络密码</label>
                                    <input type="password" id="wifi-password" placeholder="输入 WiFi 密码 (可选)" maxlength="63" class="w-full p-2 border border-gray-300 rounded-md shadow-sm focus:ring-indigo-500 focus:border-indigo-500">
                                </div>
                                <div class="p-3 text-xs text-blue-800 rounded-lg bg-blue-50">
                                    💡 提示：连接 WiFi 后才能使用定时器功能，因为需要网络时间同步。
                                </div>
                                <div class="grid grid-cols-1 sm:grid-cols-2 gap-3">
                                    <button class="w-full bg-indigo-600 text-white font-semibold py-2 px-4 rounded-md hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-indigo-500 transition-colors" onclick="saveWiFi()">
                                        💾 保存并连接
                                    </button>
                                    <button class="w-full bg-amber-500 text-white font-semibold py-2 px-4 rounded-md hover:bg-amber-600 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-amber-500 transition-colors" onclick="restartAP()">
                                        🔄 重启为热点
                                    </button>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- 固件更新 -->
                    <div id="firmware-content" class="tab-content hidden fade-in">
                        <div class="max-w-md mx-auto p-5 border rounded-lg bg-gray-50">
                            <h3 class="text-lg font-semibold mb-4 text-gray-800">🔧 固件更新</h3>
                            <div id="firmware-message" class="hidden mb-4"></div>
                            
                            <div class="space-y-4">
                                <div class="p-4 bg-blue-50 border border-blue-200 rounded-lg">
                                    <h4 class="font-medium text-blue-800 mb-2">当前固件信息</h4>
                                    <div class="text-sm text-blue-700">
                                        <div>版本: <span id="current-version">加载中...</span></div>
                                        <div>编译时间: <span id="build-time">加载中...</span></div>
                                        <div>芯片ID: <span id="firmware-chip-id">加载中...</span></div>
                                    </div>
                                </div>
                                
                                <div>
                                    <label for="firmware-file" class="block text-sm font-medium text-gray-700 mb-2">
                                        📁 选择固件文件 (.bin)
                                    </label>
                                    <input type="file" id="firmware-file" accept=".bin" class="w-full p-2 border border-gray-300 rounded-md shadow-sm focus:ring-indigo-500 focus:border-indigo-500">
                                    <div class="mt-2 text-xs text-gray-500">
                                        支持的文件格式: .bin (最大 2MB)
                                    </div>
                                </div>
                                
                                <div class="p-3 text-xs text-yellow-800 rounded-lg bg-yellow-50 border border-yellow-200">
                                    ⚠️ 警告：固件更新可能需要 2-5 分钟，期间请勿断电或关闭浏览器。更新完成后设备将自动重启。
                                </div>
                                
                                <div class="space-y-2">
                                    <button id="upload-btn" class="w-full bg-red-600 text-white font-semibold py-3 px-4 rounded-md hover:bg-red-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-red-500 transition-colors disabled:opacity-50 disabled:cursor-not-allowed" onclick="startFirmwareUpdate()" disabled>
                                        🚀 开始固件更新
                                    </button>
                                    
                                    <div id="upload-progress" class="hidden">
                                        <div class="w-full bg-gray-200 rounded-full h-3">
                                            <div id="progress-bar" class="bg-blue-600 h-3 rounded-full transition-all duration-300" style="width: 0%"></div>
                                        </div>
                                        <div class="text-sm text-gray-600 text-center mt-2">
                                            <span id="progress-text">准备中...</span>
                                        </div>
                                    </div>
                                </div>
                                
                                <div class="border-t pt-4">
                                    <h4 class="font-medium text-gray-800 mb-2">更新说明</h4>
                                    <ul class="text-sm text-gray-600 space-y-1">
                                        <li>• 确保设备连接稳定的电源</li>
                                        <li>• 选择正确的固件文件(.bin格式)</li>
                                        <li>• 更新过程中请勿断电</li>
                                        <li>• 更新完成后设备会自动重启</li>
                                        <li>• 如遇问题可重启设备恢复</li>
                                    </ul>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </main>
    </div>

    <script>
        let currentData = {
            timers: [],
            pins: [],
            systemInfo: null
        };

        let isLoadingData = false;

        // --- Class definitions for styling ---
        const activeTabClasses = ['active', 'text-indigo-600', 'font-semibold'];
        const inactiveTabClasses = ['text-gray-500', 'hover:text-gray-700'];
        const baseTabClasses = ['py-3', 'px-4', 'text-sm', 'font-medium', 'text-center', 'border-b-2', 'border-transparent', 'transition-colors', 'whitespace-nowrap'];

        document.addEventListener('DOMContentLoaded', function() {
            // Initialize tabs with base classes
            document.querySelectorAll('.tab').forEach(tab => {
                tab.classList.add(...baseTabClasses);
                if (!tab.classList.contains('active')) {
                    tab.classList.add(...inactiveTabClasses);
                }
            });
            
            // Set current time as default for timer
            setCurrentTimeAsDefault();
            
            loadData();
            // 使用异步轮询替代 setInterval
            startPolling();
        });

        function setCurrentTimeAsDefault() {
            const now = new Date();
            const hours = String(now.getHours()).padStart(2, '0');
            const minutes = String(now.getMinutes()).padStart(2, '0');
            const currentTime = `${hours}:${minutes}`;
            document.getElementById('timer-time').value = currentTime;
        }

        function startPolling() {
            async function poll() {
                if (!isLoadingData) {
                    await loadData();
                }
                // 等待5秒后再次轮询
                setTimeout(poll, 5000);
            }
            poll();
        }

        function switchTab(tabName, clickedTab) {
            document.querySelectorAll('.tab-content').forEach(content => content.classList.add('hidden'));
            document.querySelectorAll('.tab').forEach(tab => {
                tab.classList.remove(...activeTabClasses, 'active');
                tab.classList.add(...inactiveTabClasses);
            });
            
            document.getElementById(tabName + '-content').classList.remove('hidden');
            clickedTab.classList.add(...activeTabClasses);
            clickedTab.classList.remove(...inactiveTabClasses);
            
            if (tabName === 'system') {
                loadSystemInfo();
            }
        }

        async function loadData() {
            if (isLoadingData) {
                return; // 如果正在加载，直接返回
            }
            
            isLoadingData = true;
            try {
                const [statusRes, timersRes, pinsRes] = await Promise.all([
                    fetch('/api/status'),
                    fetch('/api/timers'),
                    fetch('/api/pins')
                ]);
                const status = await statusRes.json();
                const timers = await timersRes.json();
                const pins = await pinsRes.json();

                currentData.timers = timers;
                currentData.pins = pins;

                updateStatus(status);
                updateTimersList();
                updatePinsList();
                updatePinSelects();

                if (document.getElementById('system-content').offsetParent !== null) {
                    await loadSystemInfo();
                }
            } catch (error) {
                console.error('加载数据失败:', error);
                document.getElementById('connection-status').innerHTML = `<span class="inline-flex items-center px-2.5 py-0.5 rounded-full bg-red-100 text-red-800">加载失败</span>`;
            } finally {
                isLoadingData = false;
            }
        }

        async function loadSystemInfo() {
            try {
                const response = await fetch('/api/system');
                const systemInfo = await response.json();
                currentData.systemInfo = systemInfo;
                updateSystemInfo(systemInfo);
            } catch (error) {
                console.error('加载系统信息失败:', error);
            }
        }

        function updateStatus(status) {
            const statusElement = document.getElementById('connection-status');
            const apNotice = document.getElementById('ap-mode-notice');
            const timerRelated = ['timer-form', 'timer-list-container'];

            if (status.wifiConnected) {
                statusElement.innerHTML = `<span class="inline-flex items-center px-2.5 py-0.5 rounded-full bg-green-100 text-green-800">✅ WiFi: ${status.localIP}</span>`;
                apNotice.classList.add('hidden');
                timerRelated.forEach(id => document.getElementById(id).style.opacity = 1);
            } else {
                statusElement.innerHTML = `<span class="inline-flex items-center px-2.5 py-0.5 rounded-full bg-yellow-100 text-yellow-800">📶 AP: ${status.apIP}</span>`;
                apNotice.classList.remove('hidden');
                timerRelated.forEach(id => document.getElementById(id).style.opacity = 0.5);
            }
            
            const timeInfo = status.hasValidTime ? `🕐 ${status.currentTime}` : `🕐 时间未同步`;
            document.getElementById('device-info').innerHTML = `${timeInfo} | 活跃定时器: ${status.activeTimers || 0}`;
        }

        function updateTimersList() {
            const list = document.getElementById('timer-list');
            if (currentData.timers.length === 0) {
                list.innerHTML = '<div class="text-center p-10 text-gray-500">暂无定时器</div>';
                return;
            }
            list.innerHTML = currentData.timers.map((timer, index) => {
                const statusText = timer.isActive ? '🔴 运行中' : (timer.enabled ? '⏰ 等待中' : '⏸️ 已禁用');
                const statusColor = timer.isActive ? 'text-red-600' : (timer.enabled ? 'text-blue-600' : 'text-gray-500');
                const cardBorder = timer.isActive ? 'border-red-300' : (timer.enabled ? 'border-blue-300' : 'border-gray-200');
                
                return `
                <div class="border ${cardBorder} rounded-lg p-4 flex flex-col sm:flex-row justify-between items-start sm:items-center transition-all hover:shadow-md hover:border-indigo-300 ${!timer.enabled ? 'opacity-60' : ''}">
                    <div class="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-6 gap-x-4 gap-y-2 text-sm w-full">
                        <div class="font-bold text-gray-800 col-span-2 sm:col-span-1">📌 引脚 ${timer.pin}</div>
                        <div class="flex items-center">⏰ ${timer.hour.toString().padStart(2, '0')}:${timer.minute.toString().padStart(2, '0')}</div>
                        <div class="flex items-center">⏳ ${timer.duration}秒</div>
                        <div class="flex items-center">${timer.repeatDaily ? '🔄 每天' : '📅 单次'}</div>
                        <div class="flex items-center">${timer.isPWM ? `🔧 PWM ${Math.round((timer.pwmValue / 1023) * 100)}%` : '🔌 数字'}</div>
                        <div class="flex items-center font-semibold ${statusColor}">${statusText}</div>
                    </div>
                    <div class="flex space-x-2 mt-3 sm:mt-0 sm:ml-4 flex-shrink-0">
                        <button class="text-xs font-semibold py-1 px-3 rounded-md transition-colors ${timer.enabled ? 'bg-amber-100 text-amber-800 hover:bg-amber-200' : 'bg-green-100 text-green-800 hover:bg-green-200'}" onclick="toggleTimer(${index})">${timer.enabled ? '禁用' : '启用'}</button>
                        <button class="text-xs font-semibold py-1 px-3 rounded-md bg-red-100 text-red-800 hover:bg-red-200 transition-colors" onclick="deleteTimer(${index})">删除</button>
                    </div>
                </div>`;
            }).join('');
        }

        function updatePinsList() {
            const grid = document.getElementById('pins-grid');
            if (currentData.pins.length === 0) {
                grid.innerHTML = '<p class="text-gray-500 col-span-full">没有可用的引脚</p>';
                return;
            }
            grid.innerHTML = currentData.pins.map(pin => {
                let stateClass = 'bg-white hover:bg-gray-100';
                if (pin.inUse) stateClass = 'bg-yellow-100 text-yellow-800 border-yellow-300';
                else if (pin.state) stateClass = 'bg-red-100 text-red-800 border-red-300';
                
                return `
                <div class="border-2 rounded-lg p-3 text-center cursor-pointer transition-all ${stateClass}" onclick="togglePin(${pin.pin})" title="点击切换状态">
                    <div class="font-bold text-lg">${pin.pin}</div>
                    <div class="text-2xl">${pin.state ? '🔴' : '⚫'}</div>
                    ${pin.inUse ? '<div class="text-xs font-semibold">占用</div>' : '<div class="text-xs text-gray-400">空闲</div>'}
                </div>`;
            }).join('');
        }

        function updatePinSelects() {
            const selects = ['timer-pin', 'manual-pin'];
            const options = currentData.pins.map(pin => `<option value="${pin.pin}">引脚 ${pin.pin}</option>`).join('');
            selects.forEach(id => {
                const select = document.getElementById(id);
                const currentValue = select.value;
                select.innerHTML = `<option value="">选择引脚</option>${options}`;
                if (currentValue) select.value = currentValue;
            });
        }

        function updateSystemInfo(info) {
            document.getElementById('chip-id').textContent = info.chipId || '未知';
            document.getElementById('cpu-freq').textContent = `${info.cpuFreqMHz || 0} MHz`;
            document.getElementById('flash-size').textContent = formatBytes(info.flashChipSize || 0);
            document.getElementById('free-heap').textContent = formatBytes(info.freeHeap || 0);
            document.getElementById('heap-frag').textContent = `${info.heapFragmentation || 0}%`;
            document.getElementById('uptime').textContent = `${info.uptimeDays || 0}天 ${info.uptimeHours || 0}:${String(info.uptimeMinutes || 0).padStart(2, '0')}:${String(info.uptimeSeconds || 0).padStart(2, '0')}`;
            
            updateWiFiDetails(info.wifi || {});
            updateTimeDetails(info.time || {});
            updateTimerStats(info.timerStats || {});
            updateStorageDetails(info);
            updatePinDetails(info.pins || []);
        }

        function createInfoCard(label, value) {
            return `<div class="bg-gray-50 p-4 rounded-lg border border-gray-200">
                        <label class="block text-xs text-gray-500 font-medium mb-1">${label}</label>
                        <span class="text-base font-semibold text-gray-800">${value}</span>
                    </div>`;
        }

        function updateWiFiDetails(wifi) {
            const container = document.getElementById('wifi-details');
            let html = createInfoCard('连接状态', `<span class="inline-block w-2 h-2 rounded-full mr-2 ${wifi.status === 3 ? 'bg-green-500' : 'bg-red-500'}"></span>${wifi.statusText || '未知'}`)
                     + createInfoCard('工作模式', wifi.modeText || '未知');
            if (wifi.ssid) {
                html += createInfoCard('网络名称', wifi.ssid)
                      + createInfoCard('信号强度', `${wifi.rssi} dBm`)
                      + createInfoCard('本地 IP', wifi.localIP)
                      + createInfoCard('MAC 地址', wifi.macAddress);
            }
            if (wifi.apSSID) {
                html += createInfoCard('AP 名称', wifi.apSSID)
                      + createInfoCard('AP IP', wifi.apIP)
                      + createInfoCard('连接设备数', wifi.apStationCount || 0);
            }
            container.innerHTML = html;
        }

        function updateTimeDetails(time) {
            document.getElementById('time-details').innerHTML = 
                createInfoCard('同步状态', `<span class="inline-block w-2 h-2 rounded-full mr-2 ${time.hasValidTime ? 'bg-green-500' : 'bg-yellow-500'}"></span>${time.hasValidTime ? '已同步' : '未同步'}`)
              + createInfoCard('时间源', time.timeSource || '未知')
              + createInfoCard('当前时间', time.currentTime || '未知')
              + createInfoCard('当前日期', time.currentDate || '未知');
        }

        function updateTimerStats(stats) {
            const usage = Math.round((stats.total || 0) / (stats.maxTimers || 1) * 100);
            document.getElementById('timer-stats').innerHTML = 
                createInfoCard('总数', `${stats.total || 0} / ${stats.maxTimers || 0}`)
              + createInfoCard('使用率', `${usage}%`)
              + createInfoCard('已启用', stats.enabled || 0)
              + createInfoCard('运行中', stats.active || 0);
        }

        function updateStorageDetails(info) {
            const eeprom = info.eeprom || {};
            const eepromUsage = Math.round((eeprom.timerConfigUsed || 0) / (eeprom.size || 1) * 100);
            document.getElementById('storage-details').innerHTML = 
                createInfoCard('程序大小', formatBytes(info.sketchSize || 0))
              + createInfoCard('可用空间', formatBytes(info.freeSketchSpace || 0))
              + createInfoCard('EEPROM大小', `${eeprom.size || 0} 字节`)
              + createInfoCard('配置已用', `${eeprom.timerConfigUsed || 0} 字节 (${eepromUsage}%)`)
              + createInfoCard('重启原因', info.resetReason || '未知');
        }

        function updatePinDetails(pins) {
            document.getElementById('pin-details').innerHTML = pins.map(pin => {
                let stateClass = 'bg-gray-100 border-gray-200';
                if (pin.inUse) stateClass = 'bg-yellow-100 border-yellow-300';
                else if (pin.state) stateClass = 'bg-red-100 border-red-300';
                return `<div class="p-2 text-center rounded-md border ${stateClass}">
                            <div class="font-bold text-sm">${pin.pin}</div>
                            <div class="text-xs">${pin.inUse ? `🔒 #${pin.timerIndex}` : (pin.state ? 'ON' : 'OFF')}</div>
                        </div>`;
            }).join('');
        }

        function formatBytes(bytes) {
            if (bytes === 0) return '0 B';
            const k = 1024;
            const sizes = ['B', 'KB', 'MB', 'GB'];
            const i = Math.floor(Math.log(bytes) / Math.log(k));
            return `${parseFloat((bytes / Math.pow(k, i)).toFixed(2))} ${sizes[i]}`;
        }

        async function addTimer() {
            const pin = document.getElementById('timer-pin').value;
            const timeValue = document.getElementById('timer-time').value;
            const duration = document.getElementById('timer-duration').value;
            const repeatDaily = document.getElementById('timer-repeat').checked;
            const isPWM = document.getElementById('timer-pwm-mode').checked;
            const pwmValue = document.getElementById('timer-pwm-value').value;
            
            if (!pin || !timeValue || !duration) {
                showMessage('timer-message', '请填写完整信息', 'error');
                return;
            }
            
            const [hour, minute] = timeValue.split(':').map(Number);
            
            try {
                const body = { 
                    pin: parseInt(pin), 
                    hour, 
                    minute, 
                    duration: parseFloat(duration), 
                    repeatDaily,
                    isPWM: isPWM,
                    pwmValue: parseInt(pwmValue)
                };
                
                const response = await fetch('/api/timers', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify(body)
                });
                const result = await response.json();
                if (result.success) {
                    showMessage('timer-message', '定时器添加成功！', 'success');
                    loadData();
                } else {
                    showMessage('timer-message', result.message || '添加失败', 'error');
                }
            } catch (error) {
                showMessage('timer-message', '请求失败', 'error');
            }
        }

        async function deleteTimer(index) {
            if (!confirm('确定要删除这个定时器吗？')) return;
            try {
                const response = await fetch(`/api/timers/${index}`, {method: 'DELETE'});
                if (response.ok) loadData();
                else alert('删除失败');
            } catch (error) {
                alert('请求失败');
            }
        }

        async function toggleTimer(index) {
            const timer = currentData.timers[index];
            try {
                const response = await fetch(`/api/timers/${index}`, {
                    method: 'PUT',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({ ...timer, enabled: !timer.enabled })
                });
                if (response.ok) loadData();
                else alert('操作失败');
            } catch (error) {
                alert('请求失败');
            }
        }

        async function clearAllTimers() {
            if (!confirm('确定要清除所有定时器吗？此操作不可恢复！')) return;
            try {
                const response = await fetch('/api/timers/clear', {method: 'POST'});
                if (response.ok) {
                    showMessage('timer-message', '所有定时器已清除', 'success');
                    loadData();
                } else {
                    showMessage('timer-message', '清除失败', 'error');
                }
            } catch (error) {
                showMessage('timer-message', '请求失败', 'error');
            }
        }

        async function manualControl() {
            const pin = document.getElementById('manual-pin').value;
            const duration = document.getElementById('manual-duration').value;
            const isPWM = document.getElementById('manual-pwm-mode').checked;
            const pwmValue = document.getElementById('manual-pwm-value').value;
            
            if (!pin) {
                showMessage('manual-message', '请选择引脚', 'error');
                return;
            }
            try {
                const body = { 
                    pin: parseInt(pin), 
                    duration: parseFloat(duration) || 0,
                    isPWM: isPWM,
                    pwmValue: parseInt(pwmValue)
                };
                
                const response = await fetch('/api/manual', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify(body)
                });
                const result = await response.json();
                if (result.success) {
                    showMessage('manual-message', '控制命令执行成功', 'success');
                    loadData();
                } else {
                    showMessage('manual-message', `控制失败: ${result.message || '未知错误'}`, 'error');
                }
            } catch (error) {
                showMessage('manual-message', '请求失败', 'error');
            }
        }

        async function togglePin(pin) {
            try {
                const response = await fetch('/api/manual', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({
                        pin: pin, 
                        duration: 0,
                        isPWM: false,
                        pwmValue: 0
                    })
                });
                if (response.ok) {
                    loadData();
                }
            } catch (error) {
                console.error('引脚切换失败:', error);
            }
        }

        async function saveWiFi() {
            const ssid = document.getElementById('wifi-ssid').value;
            const password = document.getElementById('wifi-password').value;
            if (!ssid) {
                showMessage('wifi-message', '请输入 WiFi 名称', 'error');
                return;
            }
            showMessage('wifi-message', '正在保存设置...', 'info');
            try {
                const response = await fetch('/api/wifi', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({ssid, password})
                });
                const result = await response.json();
                if (result.success) {
                    showMessage('wifi-message', 'WiFi 设置已保存，设备将重启并尝试连接...', 'success');
                    setTimeout(() => window.location.reload(), 3000);
                } else {
                    showMessage('wifi-message', `设置失败: ${result.message || '未知错误'}`, 'error');
                }
            } catch (error) {
                showMessage('wifi-message', '请求失败', 'error');
            }
        }

        async function restartAP() {
            if (!confirm('确定要重启为 AP 模式吗？设备将断开当前 WiFi 连接。')) return;
            showMessage('wifi-message', '正在重启...', 'info');
            try {
                const response = await fetch('/api/wifi/reset', {method: 'POST'});
                if (response.ok) {
                    showMessage('wifi-message', '设备将重启为 AP 模式...', 'success');
                    setTimeout(() => window.location.reload(), 3000);
                }
            } catch (error) {
                showMessage('wifi-message', '请求失败', 'error');
            }
        }

        function togglePWMMode() {
            const pwmControls = document.getElementById('pwm-controls');
            const isPWMMode = document.getElementById('manual-pwm-mode').checked;
            
            if (isPWMMode) {
                pwmControls.classList.remove('hidden');
            } else {
                pwmControls.classList.add('hidden');
            }
        }

        function updatePWMPercentage(value) {
            const percentage = Math.round((value / 1023) * 100);
            document.getElementById('pwm-percentage').textContent = percentage + '%';
        }

        function toggleTimerPWMMode() {
            const pwmControls = document.getElementById('timer-pwm-controls');
            const isPWMMode = document.getElementById('timer-pwm-mode').checked;
            
            if (isPWMMode) {
                pwmControls.classList.remove('hidden');
            } else {
                pwmControls.classList.add('hidden');
            }
        }

        function updateTimerPWMPercentage(value) {
            const percentage = Math.round((value / 1023) * 100);
            document.getElementById('timer-pwm-percentage').textContent = percentage + '%';
        }

        // 固件更新相关函数
        document.addEventListener('DOMContentLoaded', function() {
            const fileInput = document.getElementById('firmware-file');
            const uploadBtn = document.getElementById('upload-btn');
            
            if (fileInput && uploadBtn) {
                fileInput.addEventListener('change', function() {
                    const file = this.files[0];
                    if (file) {
                        if (file.size > 2 * 1024 * 1024) { // 2MB限制
                            showMessage('firmware-message', '文件大小不能超过 2MB', 'error');
                            this.value = '';
                            uploadBtn.disabled = true;
                            return;
                        }
                        if (!file.name.toLowerCase().endsWith('.bin')) {
                            showMessage('firmware-message', '请选择 .bin 格式的固件文件', 'error');
                            this.value = '';
                            uploadBtn.disabled = true;
                            return;
                        }
                        uploadBtn.disabled = false;
                        showMessage('firmware-message', `已选择文件: ${file.name} (${(file.size / 1024).toFixed(1)} KB)`, 'info');
                    } else {
                        uploadBtn.disabled = true;
                    }
                });
            }
            
            // 加载当前固件信息
            loadFirmwareInfo();
        });

        async function loadFirmwareInfo() {
            try {
                const response = await fetch('/api/system');
                if (response.ok) {
                    const data = await response.json();
                    document.getElementById('current-version').textContent = data.firmwareVersion || 'v1.0.0';
                    document.getElementById('build-time').textContent = data.buildDateTime || '未知';
                    document.getElementById('firmware-chip-id').textContent = data.chipId || '未知';
                }
            } catch (error) {
                console.error('加载固件信息失败:', error);
            }
        }

        async function startFirmwareUpdate() {
            const fileInput = document.getElementById('firmware-file');
            const file = fileInput.files[0];
            
            if (!file) {
                showMessage('firmware-message', '请先选择固件文件', 'error');
                return;
            }
            
            if (!confirm('确定要更新固件吗？更新过程中请勿断电，设备将自动重启。')) {
                return;
            }
            
            const uploadBtn = document.getElementById('upload-btn');
            const progressDiv = document.getElementById('upload-progress');
            const progressBar = document.getElementById('progress-bar');
            const progressText = document.getElementById('progress-text');
            
            // 禁用上传按钮并显示进度条
            uploadBtn.disabled = true;
            progressDiv.classList.remove('hidden');
            progressBar.style.width = '0%';
            progressText.textContent = '正在上传固件...';
            
            const formData = new FormData();
            formData.append('firmware', file);
            
            console.log('开始上传固件文件:', file.name, file.size, 'bytes');
            
            try {
                const xhr = new XMLHttpRequest();
                
                // 监听上传进度
                xhr.upload.onprogress = function(e) {
                    if (e.lengthComputable) {
                        const percentComplete = Math.round((e.loaded / e.total) * 100);
                        progressBar.style.width = percentComplete + '%';
                        progressText.textContent = `上传中... ${percentComplete}%`;
                    }
                };
                
                // 监听状态变化
                xhr.onreadystatechange = function() {
                    if (xhr.readyState === XMLHttpRequest.DONE) {
                        if (xhr.status === 200) {
                            progressBar.style.width = '100%';
                            progressText.textContent = '上传完成，正在刷写固件...';
                            showMessage('firmware-message', '固件上传成功，正在更新...请等待设备重启', 'success');
                            
                            // 模拟刷写进度
                            setTimeout(() => {
                                progressText.textContent = '固件更新中，请稍候...';
                            }, 1000);
                            
                            setTimeout(() => {
                                progressText.textContent = '更新完成，设备即将重启...';
                                showMessage('firmware-message', '固件更新完成！设备将在几秒钟后重启', 'success');
                            }, 3000);
                            
                            setTimeout(() => {
                                window.location.reload();
                            }, 8000);
                        } else {
                            console.error('上传失败，状态码:', xhr.status);
                            let errorMessage = '未知错误';
                            try {
                                const response = JSON.parse(xhr.responseText);
                                errorMessage = response.message || errorMessage;
                            } catch (e) {
                                errorMessage = `服务器错误 (${xhr.status})`;
                            }
                            showMessage('firmware-message', `更新失败: ${errorMessage}`, 'error');
                            resetUploadUI();
                        }
                    }
                };
                
                xhr.onerror = function() {
                    console.error('网络错误或请求失败');
                    showMessage('firmware-message', '上传失败，请检查网络连接', 'error');
                    resetUploadUI();
                };
                
                xhr.onabort = function() {
                    console.error('上传被中止');
                    showMessage('firmware-message', '上传被中止', 'error');
                    resetUploadUI();
                };
                
                xhr.open('POST', '/api/firmware/update', true);
                console.log('发送固件上传请求到:', '/api/firmware/update');
                xhr.send(formData);
                
            } catch (error) {
                console.error('固件更新失败:', error);
                showMessage('firmware-message', '更新失败: ' + error.message, 'error');
                resetUploadUI();
            }
        }

        function resetUploadUI() {
            const uploadBtn = document.getElementById('upload-btn');
            const progressDiv = document.getElementById('upload-progress');
            const fileInput = document.getElementById('firmware-file');
            
            uploadBtn.disabled = false;
            progressDiv.classList.add('hidden');
            fileInput.value = '';
        }

        function showMessage(elementId, message, type) {
            const el = document.getElementById(elementId);
            el.textContent = message;
            const typeClasses = {
                success: 'bg-green-50 text-green-800 border-green-200',
                error: 'bg-red-50 text-red-800 border-red-200',
                info: 'bg-blue-50 text-blue-800 border-blue-200'
            };
            el.className = `p-3 mb-4 text-sm rounded-lg border ${typeClasses[type] || typeClasses.info}`;
            el.classList.remove('hidden');
            setTimeout(() => el.classList.add('hidden'), 5000);
        }
    </script>
</body>
</html>
)rawliteral";

#endif
