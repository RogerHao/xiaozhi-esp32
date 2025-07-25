# 小智 ESP32 屏幕旋转配置

## 概述

这是小智 ESP32 项目的屏幕旋转配置版本，专门为 Waveshare ESP32-C6 LCD 1.69 英寸屏幕添加了顺时针旋转90度的配置。

## 修改内容

### 屏幕旋转配置

在 `main/boards/waveshare-c6-lcd-1.69/config.h` 中进行了以下修改：

```cpp
// 顺时针旋转90度：交换宽高，启用XY交换
#define DISPLAY_WIDTH           280
#define DISPLAY_HEIGHT          240
#define DISPLAY_MIRROR_X        false
#define DISPLAY_MIRROR_Y        false
#define DISPLAY_SWAP_XY         true
#define DISPLAY_RGB_ORDER       LCD_RGB_ELEMENT_ORDER_RGB
#define DISPLAY_INVERT_COLOR    true

// 调整偏移以适应旋转后的显示
#define DISPLAY_OFFSET_X        20
#define DISPLAY_OFFSET_Y        0
```

### 修改说明

1. **交换宽高**: 将 `DISPLAY_WIDTH` 和 `DISPLAY_HEIGHT` 的值互换
2. **启用XY交换**: 设置 `DISPLAY_SWAP_XY` 为 `true`
3. **调整偏移**: 将 `DISPLAY_OFFSET_X` 和 `DISPLAY_OFFSET_Y` 的值互换

## 使用方法

### 编译固件

```bash
# 设置目标芯片
idf.py set-target esp32c6

# 配置板子类型
idf.py menuconfig
# 选择: Component config -> Board Type -> ESP32C6 LCD 1.69

# 编译
idf.py build

# 烧录
idf.py flash
```

### 分支管理

本项目使用独立分支管理旋转配置：

- `main`: 原始代码
- `waveshare-c6-rotated`: 包含屏幕旋转配置的分支

## 同步上游更新

```bash
# 获取上游更新
git fetch upstream

# 更新主分支
git checkout main
git merge upstream/main

# 更新旋转分支
git checkout waveshare-c6-rotated
git rebase main
```

## 注意事项

1. **避免冲突**: 使用独立分支可以避免与原始项目代码产生冲突
2. **测试验证**: 编译后请测试屏幕显示是否正常，如有问题可以微调偏移参数
3. **版本管理**: 建议在每次上游更新后重新测试旋转配置

## 其他旋转角度

如果需要其他旋转角度，可以参考以下配置：

### 逆时针90度
```cpp
#define DISPLAY_WIDTH           280
#define DISPLAY_HEIGHT          240
#define DISPLAY_SWAP_XY         true
#define DISPLAY_MIRROR_X        true
#define DISPLAY_MIRROR_Y        true
#define DISPLAY_OFFSET_X        20
#define DISPLAY_OFFSET_Y        0
```

### 180度
```cpp
#define DISPLAY_WIDTH           240
#define DISPLAY_HEIGHT          280
#define DISPLAY_SWAP_XY         false
#define DISPLAY_MIRROR_X        true
#define DISPLAY_MIRROR_Y        true
#define DISPLAY_OFFSET_X        0
#define DISPLAY_OFFSET_Y        20
```

## 项目信息

- **原项目**: [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)
- **Fork 版本**: [RogerHao/xiaozhi-esp32](https://github.com/RogerHao/xiaozhi-esp32)
- **旋转分支**: `waveshare-c6-rotated`

## 许可证

本项目基于原项目的 MIT 许可证。 