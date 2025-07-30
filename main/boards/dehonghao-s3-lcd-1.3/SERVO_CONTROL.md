# 舵机控制功能说明

## 概述

本项目为dehonghao-s3-lcd-1.3开发板添加了舵机控制功能，支持通过MCP协议远程控制舵机。舵机型号为GDW DS031MG数字舵机，控制引脚为GPIO 12。

## 硬件连接

- 舵机信号线连接到GPIO 12
- 舵机电源线连接到5V电源
- 舵机地线连接到GND

## MCP工具列表

### 1. 基础角度控制

**工具名称**: `servo.set_angle`

**功能**: 设置舵机到指定角度

**参数**:
- `angle`: 整数，角度值（0-180度）

**示例**:
```json
{
  "tool": "servo.set_angle",
  "parameters": {
    "angle": 90
  }
}
```

### 2. 预设角度控制

**工具名称**: `servo.set_preset`

**功能**: 设置舵机到预设角度（0、90、180度）

**参数**:
- `preset`: 字符串，预设值（"0", "90", "180"）

**示例**:
```json
{
  "tool": "servo.set_preset",
  "parameters": {
    "preset": "90"
  }
}
```

### 3. 摆动控制

**工具名称**: `servo.oscillate`

**功能**: 让舵机在指定角度范围内摆动

**参数**:
- `start_angle`: 整数，起始角度（0-180度）
- `end_angle`: 整数，结束角度（0-180度）
- `cycles`: 整数，摆动次数（1-10次）
- `period_ms`: 整数，摆动周期（500-5000毫秒）

**示例**:
```json
{
  "tool": "servo.oscillate",
  "parameters": {
    "start_angle": 0,
    "end_angle": 90,
    "cycles": 3,
    "period_ms": 1000
  }
}
```

### 4. 停止舵机

**工具名称**: `servo.stop`

**功能**: 停止舵机摆动

**参数**: 无

**示例**:
```json
{
  "tool": "servo.stop",
  "parameters": {}
}
```

### 5. 获取当前角度

**工具名称**: `servo.get_angle`

**功能**: 获取舵机当前角度

**参数**: 无

**返回值**: 整数，当前角度（0-180度）

**示例**:
```json
{
  "tool": "servo.get_angle",
  "parameters": {}
}
```

## 使用场景示例

### 场景1：机械臂控制
```json
// 抬起机械臂
{
  "tool": "servo.set_angle",
  "parameters": {"angle": 180}
}

// 放下机械臂
{
  "tool": "servo.set_angle", 
  "parameters": {"angle": 0}
}
```

### 场景2：摆动动作
```json
// 在0-90度之间摆动3次，每次1秒
{
  "tool": "servo.oscillate",
  "parameters": {
    "start_angle": 0,
    "end_angle": 90,
    "cycles": 3,
    "period_ms": 1000
  }
}
```

### 场景3：预设位置切换
```json
// 切换到0度位置
{
  "tool": "servo.set_preset",
  "parameters": {"preset": "0"}
}

// 切换到中间位置
{
  "tool": "servo.set_preset",
  "parameters": {"preset": "90"}
}

// 切换到180度位置
{
  "tool": "servo.set_preset",
  "parameters": {"preset": "180"}
}
```

## 技术参数

- **舵机型号**: GDW DS031MG
- **控制引脚**: GPIO 12
- **PWM频率**: 50Hz
- **角度范围**: 0-180度
- **脉宽范围**: 500-2500微秒
- **分辨率**: 13位（8192级）

## 注意事项

1. 舵机需要稳定的5V电源供应
2. 避免舵机长时间堵转，可能损坏舵机
3. 摆动功能会创建独立任务，可以随时通过stop命令停止
4. 角度值会自动限制在0-180度范围内
5. 支持舵机微调功能，可以在代码中调整trim值

## 代码修改

如果需要调整舵机参数，可以修改以下文件：

- `config.h`: 修改舵机引脚定义
- `servo_controller.h`: 修改角度范围、脉宽等参数
- `servo_controller.cc`: 修改PWM配置、摆动逻辑等

## 故障排除

1. **舵机不转动**: 检查电源连接和信号线连接
2. **角度不准确**: 调整舵机微调值或检查机械结构
3. **摆动异常**: 检查摆动参数是否合理
4. **编译错误**: 确保所有源文件都已正确添加 