# Dehonghao S3 LCD 1.3 开发板

## 硬件规格

- **主控芯片**: ESP32-S3-N16R8
- **显示屏**: 1.3寸 ST7789 IPS彩色屏幕
- **分辨率**: 240x240像素
- **接口**: SPI
- **音频**: 自带麦克风和扬声器
- **连接**: Type-C USB接口

## 硬件连接

### 显示屏连接 (SPI)
- **MOSI**: GPIO 4 (Pin 4 - 右侧)
- **SCLK**: GPIO 15 (Pin 8 - 左侧)  
- **CS**: GPIO 22 (Pin 18 - 右侧)
- **DC**: GPIO 21 (Pin 18 - 右侧)
- **RST**: GPIO 18 (Pin 11 - 左侧)
- **BL**: GPIO 23 (背光控制)

### 音频连接 (I2S)
- **麦克风**:
  - WS: GPIO 25
  - SCK: GPIO 26
  - DIN: GPIO 32

- **扬声器**:
  - BCLK: GPIO 14 (Pin 19 - 右侧)
  - LRCK: GPIO 27
  - DOUT: GPIO 33

### 按钮连接
- **Boot按钮**: GPIO 0 (Pin 14 - 右侧)
- **触摸按钮**: GPIO 5 (Pin 5 - 左侧)
- **ASR按钮**: GPIO 19 (Pin 13 - 左侧)
- **内置LED**: GPIO 2 (Pin 3 - 右侧)

### RGB彩灯
- **WS2812B**: GPIO 48 (Pin 16 - 右侧) - 板载RGB彩灯，用于显示对话状态
- **外接RGB灯**: GPIO 9, 10, 11 (Pin 17, 18, 19 - 右侧) - 支持MCP云端控制的PWM RGB灯

### 舵机控制
- **舵机信号**: GPIO 12 (Pin 20 - 右侧) - 支持MCP云端控制的舵机，型号GDW DS031MG

### 其他引脚
- **电源**: 3V3 (Pin 2, 21 - 左侧), 5V0 (Pin 21 - 右侧)
- **地**: GND (Pin 1 - 左右两侧)
- **复位**: RST (Pin 3 - 左侧)

## 编译和烧录

### 编译固件
```bash
python scripts/release.py dehonghao-s3-lcd-1.3
```

### 烧录固件
```bash
# 使用生成的烧录脚本
./scripts/flash.sh
```

## 功能特性

1. **显示屏**: 1.3寸240x240 IPS彩色屏幕，支持中文显示
2. **音频**: 支持语音输入和输出，使用NoAudioCodecSimplex
3. **按钮控制**:
   - Boot按钮: 切换聊天状态
   - 触摸按钮: 语音识别控制
   - ASR按钮: 唤醒词触发
4. **RGB彩灯**: 
   - 板载WS2812B可编程RGB彩灯控制，显示对话状态
   - 外接PWM RGB灯，支持MCP云端控制
5. **舵机控制**: 
   - 支持GDW DS031MG数字舵机控制
   - 支持角度设置、预设位置、摆动等高级功能
6. **WiFi连接**: 支持WiFi网络连接
7. **MCP协议**: 支持云端控制外接RGB灯和舵机

## 配置说明

### 显示屏配置
- 驱动芯片: ST7789
- 分辨率: 240x240
- 颜色格式: RGB565
- SPI模式: 0
- 时钟频率: 40MHz

### 音频配置
- 输入采样率: 16kHz
- 输出采样率: 24kHz
- 模式: Simplex (麦克风和扬声器分离)

### RGB灯配置
- **板载WS2812B**: GPIO48，用于显示对话状态
- **外接PWM RGB灯**: GPIO9(R), GPIO10(G), GPIO11(B)，支持MCP云端控制

### 舵机配置
- **舵机信号**: GPIO12，支持GDW DS031MG数字舵机
- **PWM频率**: 50Hz
- **角度范围**: 0-180度
- **脉宽范围**: 500-2500微秒

## MCP控制功能

### 外接RGB灯控制
通过MCP协议可以远程控制外接的RGB灯：

1. **打开RGB灯**: `rgb_light.turn_on`
2. **关闭RGB灯**: `rgb_light.turn_off`  
3. **设置RGB颜色**: `rgb_light.set_rgb` (参数: r, g, b，范围0-255)

### 舵机控制
通过MCP协议可以远程控制舵机：

1. **设置角度**: `servo.set_angle` (参数: angle，范围0-180度)
2. **预设角度**: `servo.set_preset` (参数: preset，可选"0", "90", "180")
3. **摆动控制**: `servo.oscillate` (参数: start_angle, end_angle, cycles, period_ms)
4. **停止摆动**: `servo.stop`
5. **获取角度**: `servo.get_angle`

### 使用示例
```python
# RGB灯控制
# 打开红色灯
mcp.rgb_light.turn_on()

# 设置绿色
mcp.rgb_light.set_rgb(r=0, g=255, b=0)

# 设置蓝色
mcp.rgb_light.set_rgb(r=0, g=0, b=255)

# 关闭灯
mcp.rgb_light.turn_off()

# 舵机控制
# 设置到90度
mcp.servo.set_angle(angle=90)

# 设置到预设位置
mcp.servo.set_preset(preset="180")

# 在0-90度之间摆动3次
mcp.servo.oscillate(start_angle=0, end_angle=90, cycles=3, period_ms=1000)

# 停止摆动
mcp.servo.stop()

# 获取当前角度
current_angle = mcp.servo.get_angle()
```

## 注意事项

1. 确保使用正确的ESP32S3开发板配置
2. 显示屏的SPI引脚连接正确
3. 音频I2S引脚配置正确
4. 首次使用需要配置WiFi网络
5. 引脚定义基于ESP32S3-DevKitC-1开发板，其他开发板可能需要调整
6. 板载RGB彩灯使用WS2812B协议，需要3.3V供电
7. 外接RGB灯使用PWM控制，共阴极连接，支持全彩控制
8. 舵机需要稳定的5V电源供应，避免长时间堵转
9. 避免使用GPIO33-GPIO37，这些引脚被Octal SPI PSRAM占用

## 故障排除

### 显示屏不显示
- 检查SPI引脚连接
- 确认显示屏电源供电
- 检查背光控制引脚

### 音频无输出
- 检查I2S引脚连接
- 确认扬声器连接
- 检查音频编解码器配置

### 无法连接WiFi
- 检查WiFi凭据配置
- 确认网络环境
- 检查天线连接

### 板载RGB彩灯不工作
- 检查GPIO48连接
- 确认3.3V电源供电
- 检查WS2812B数据线连接
- 确认RGB彩灯数量配置正确

### 外接RGB灯不工作
- 检查GPIO9, 10, 11连接
- 确认共阴极连接正确
- 检查PWM频率设置
- 确认LEDC定时器配置正确

### 系统不稳定或屏幕闪烁
- 检查是否使用了被占用的GPIO引脚
- 确认LEDC定时器没有冲突
- 检查PWM通道分配是否正确

### 舵机不工作
- 检查GPIO12信号线连接
- 确认舵机5V电源供电稳定
- 检查舵机地线连接
- 确认舵机型号为GDW DS031MG或兼容型号
- 检查PWM频率是否为50Hz

## 开发者信息

- **开发者**: 郝德宏 (RogerHao)
- **基于**: bread-compact-esp32-lcd
- **适配**: ESP32S3 + 1.3寸ST7789显示屏
- **参考硬件**: ESP32S3-DevKitC-1开发板
- **板载RGB彩灯**: WS2812B协议，GPIO48控制
- **外接RGB灯**: PWM控制，GPIO9/10/11，支持MCP云端控制
- **舵机控制**: PWM控制，GPIO12，支持GDW DS031MG舵机，支持MCP云端控制 