#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "display/lcd_display.h"
#include "system_reset.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "mcp_server.h"
#include "lamp_controller.h"
#include "led/single_led.h"
#include "led/pwm_rgb_led.h"
#include "servo_controller.h"

#include <wifi_station.h>
#include <esp_log.h>
#include <driver/i2c_master.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <driver/spi_common.h>

// 定义日志标签，用于调试输出
#define TAG "DehonghaoS3Lcd13Board"

// 声明字体资源，这些字体文件会被编译进固件
LV_FONT_DECLARE(font_puhui_16_4);      // 中文字体
LV_FONT_DECLARE(font_awesome_16_4);    // 图标字体

/**
 * @brief Dehonghao S3 LCD 1.3 开发板类
 * 
 * 这个类继承自WifiBoard，实现了特定开发板的硬件初始化和管理功能。
 * 包括显示屏、音频、按钮、LED等外设的配置和控制。
 */
class DehonghaoS3Lcd13Board : public WifiBoard {
private:
    // 硬件组件实例
    Button boot_button_;        // Boot按钮对象，用于检测按键事件
    LcdDisplay* display_;       // 显示屏对象指针，管理LCD显示
    PwmRgbLed* rgb_led_;       // PWM RGB灯对象指针，控制外接RGB灯
    ServoController* servo_;    // 舵机控制器对象指针，控制舵机

    /**
     * @brief 初始化SPI总线
     * 
     * SPI (Serial Peripheral Interface) 是一种同步串行通信协议，
     * 用于连接显示屏、SD卡等外设。这里配置SPI3总线用于驱动显示屏。
     */
    void InitializeSpi() {
        // 定义SPI总线配置结构体
        spi_bus_config_t buscfg = {};
        
        // 配置SPI引脚
        buscfg.mosi_io_num = DISPLAY_MOSI_PIN;    // 主输出从输入引脚 (Master Out Slave In)
        buscfg.miso_io_num = GPIO_NUM_NC;         // 主输入从输出引脚，这里不使用设为NC (No Connection)
        buscfg.sclk_io_num = DISPLAY_CLK_PIN;     // 时钟引脚
        buscfg.quadwp_io_num = GPIO_NUM_NC;       // 四线SPI写保护引脚，不使用
        buscfg.quadhd_io_num = GPIO_NUM_NC;       // 四线SPI保持引脚，不使用
        
        // 设置最大传输大小，这里设置为屏幕分辨率 * 每个像素的字节数
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        
        // 初始化SPI总线，使用SPI3_HOST，自动分配DMA通道
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    /**
     * @brief 初始化LCD显示屏
     * 
     * 配置ST7789显示屏驱动芯片，设置显示参数如分辨率、颜色格式等。
     * 使用ESP-IDF的LCD驱动框架来管理显示屏。
     */
    void InitializeLcdDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;  // LCD面板IO句柄
        esp_lcd_panel_handle_t panel = nullptr;        // LCD面板句柄
        
        // 初始化LCD面板IO (SPI接口)
        ESP_LOGD(TAG, "Install panel IO");
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_CS_PIN;        // 片选引脚
        io_config.dc_gpio_num = DISPLAY_DC_PIN;        // 数据/命令选择引脚
        io_config.spi_mode = DISPLAY_SPI_MODE;         // SPI模式 (0-3)
        io_config.pclk_hz = 40 * 1000 * 1000;         // SPI时钟频率 (40MHz)
        io_config.trans_queue_depth = 10;              // 传输队列深度
        io_config.lcd_cmd_bits = 8;                    // 命令位宽
        io_config.lcd_param_bits = 8;                  // 参数位宽
        
        // 创建SPI LCD面板IO
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        // 初始化ST7789显示屏驱动芯片
        ESP_LOGD(TAG, "Install LCD driver");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RST_PIN;  // 复位引脚
        panel_config.rgb_ele_order = DISPLAY_RGB_ORDER; // RGB颜色顺序
        panel_config.bits_per_pixel = 16;               // 每像素位数 (RGB565)
        
        // 创建ST7789面板
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));
        
        // 执行硬件复位
        esp_lcd_panel_reset(panel);
        
        // 初始化面板
        esp_lcd_panel_init(panel);
        
        // 配置显示参数
        esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);  // 颜色反转
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);            // 交换X/Y坐标
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y); // 镜像设置
        
        // 创建SpiLcdDisplay对象，管理显示内容和字体
        display_ = new SpiLcdDisplay(panel_io, panel,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                                    DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,
                                    {
                                        .text_font = &font_puhui_16_4,      // 文本字体
                                        .icon_font = &font_awesome_16_4,    // 图标字体
#if CONFIG_USE_WECHAT_MESSAGE_STYLE
                                        .emoji_font = font_emoji_32_init(), // 表情字体
#else
                                        .emoji_font = DISPLAY_HEIGHT >= 240 ? font_emoji_64_init() : font_emoji_32_init(),
#endif
                                    });
    }

    /**
     * @brief 初始化按钮
     * 
     * 配置Boot按钮的点击事件处理函数。
     * 当按钮被按下时，根据当前系统状态执行相应操作。
     */
    void InitializeButtons() {
        // 设置按钮点击回调函数 (使用Lambda表达式)
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();  // 获取应用程序实例
            
            // 如果设备正在启动且WiFi未连接，则重置WiFi配置
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            
            // 切换聊天状态 (开始/停止语音对话)
            app.ToggleChatState();
        });
    }

    /**
     * @brief 初始化物联网工具 (MCP协议)
     * 
     * 注册MCP (Model Context Protocol) 工具，使外接RGB灯可以通过云端控制。
     * MCP是一种AI模型与外部工具交互的协议。
     */
    void InitializeTools() {
        auto& mcp_server = McpServer::GetInstance();  // 获取MCP服务器实例
        
        // 创建PWM RGB灯对象，使用GPIO 9, 10, 11控制红绿蓝三色
        rgb_led_ = new PwmRgbLed(RGB_R_PIN, RGB_G_PIN, RGB_B_PIN);

        // 创建舵机控制器对象，使用GPIO 12控制舵机
        servo_ = new ServoController(SERVO_PIN);
        servo_->Initialize();

        // 注册"打开RGB灯"工具
        mcp_server.AddTool("rgb_light.turn_on", "打开RGB灯", PropertyList(), 
            [this](const PropertyList& properties) -> ReturnValue {
                rgb_led_->TurnOn();  // 调用RGB灯的打开方法
                return true;         // 返回成功
            });

        // 注册"关闭RGB灯"工具
        mcp_server.AddTool("rgb_light.turn_off", "关闭RGB灯", PropertyList(), 
            [this](const PropertyList& properties) -> ReturnValue {
                rgb_led_->TurnOff(); // 调用RGB灯的关闭方法
                return true;         // 返回成功
            });

        // 注册"设置RGB颜色"工具，接受r, g, b三个参数
        mcp_server.AddTool("rgb_light.set_rgb", "设置RGB颜色", PropertyList({
            Property("r", kPropertyTypeInteger, 0, 255),  // 红色分量，范围0-255
            Property("g", kPropertyTypeInteger, 0, 255),  // 绿色分量，范围0-255
            Property("b", kPropertyTypeInteger, 0, 255)   // 蓝色分量，范围0-255
        }), [this](const PropertyList& properties) -> ReturnValue {
            // 从参数中提取RGB值
            int r = properties["r"].value<int>();
            int g = properties["g"].value<int>();
            int b = properties["b"].value<int>();
            
            // 设置RGB灯颜色
            rgb_led_->SetColor(r, g, b);
            
            // 如果灯当前是关闭状态，则自动打开
            if (!rgb_led_->IsOn()) {
                rgb_led_->TurnOn();
            }
            return true;  // 返回成功
        });

        // 注册舵机控制工具
        // 基础角度控制
        mcp_server.AddTool("servo.set_angle", "设置舵机角度", PropertyList({
            Property("angle", kPropertyTypeInteger, 0, 180)  // 角度范围0-180度
        }), [this](const PropertyList& properties) -> ReturnValue {
            int angle = properties["angle"].value<int>();
            esp_err_t ret = servo_->SetAngle(angle);
            return ret == ESP_OK;
        });

        // 预设角度控制
        mcp_server.AddTool("servo.set_preset", "设置舵机到预设角度", PropertyList({
            Property("preset", kPropertyTypeString)  // 预设值：0, 90, 180
        }), [this](const PropertyList& properties) -> ReturnValue {
            const std::string& preset = properties["preset"].value<std::string>();
            int angle = 90;  // 默认中间位置
            
            if (preset == "0") {
                angle = 0;
            } else if (preset == "90") {
                angle = 90;
            } else if (preset == "180") {
                angle = 180;
            } else {
                return false;  // 无效的预设值
            }
            
            esp_err_t ret = servo_->SetAngle(angle);
            return ret == ESP_OK;
        });

        // 摆动控制
        mcp_server.AddTool("servo.oscillate", "舵机摆动", PropertyList({
            Property("start_angle", kPropertyTypeInteger, 0, 180),  // 起始角度
            Property("end_angle", kPropertyTypeInteger, 0, 180),    // 结束角度
            Property("cycles", kPropertyTypeInteger, 1, 10),        // 摆动次数，1-10次
            Property("period_ms", kPropertyTypeInteger, 500, 5000)  // 摆动周期，500-5000毫秒
        }), [this](const PropertyList& properties) -> ReturnValue {
            int start_angle = properties["start_angle"].value<int>();
            int end_angle = properties["end_angle"].value<int>();
            int cycles = properties["cycles"].value<int>();
            int period_ms = properties["period_ms"].value<int>();
            
            esp_err_t ret = servo_->Oscillate(start_angle, end_angle, cycles, period_ms);
            return ret == ESP_OK;
        });

        // 停止舵机
        mcp_server.AddTool("servo.stop", "停止舵机摆动", PropertyList(), 
            [this](const PropertyList& properties) -> ReturnValue {
                servo_->Stop();
                return true;
            });

        // 获取当前角度
        mcp_server.AddTool("servo.get_angle", "获取舵机当前角度", PropertyList(), 
            [this](const PropertyList& properties) -> ReturnValue {
                return servo_->GetAngle();
            });
    }

public:
    /**
     * @brief 构造函数
     * 
     * 在对象创建时自动调用，负责初始化所有硬件组件。
     * 初始化顺序很重要：先初始化底层硬件，再初始化应用层功能。
     */
    DehonghaoS3Lcd13Board() :
        boot_button_(BOOT_BUTTON_GPIO) {  // 初始化Boot按钮，指定GPIO引脚
        
        // 按顺序初始化各个硬件模块
        InitializeSpi();           // 1. 初始化SPI总线
        InitializeLcdDisplay();    // 2. 初始化LCD显示屏
        InitializeButtons();       // 3. 初始化按钮
        InitializeTools();         // 4. 初始化MCP工具
        
        // 5. 恢复显示屏背光亮度 (如果背光引脚已配置)
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            GetBacklight()->RestoreBrightness();
        }
    }

    /**
     * @brief 获取内置LED对象
     * 
     * 返回控制板载LED的对象，用于显示系统状态。
     * 这里使用静态对象，确保LED在整个程序运行期间都存在。
     */
    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);  // 创建单色LED对象
        return &led;
    }

    /**
     * @brief 获取音频编解码器对象
     * 
     * 根据编译配置选择不同的音频编解码器实现。
     * Simplex模式：麦克风和扬声器使用不同的I2S配置
     * Duplex模式：麦克风和扬声器共享I2S配置
     */
    virtual AudioCodec* GetAudioCodec() override {
#ifdef AUDIO_I2S_METHOD_SIMPLEX
        // Simplex模式：分离的I2S配置
        static NoAudioCodecSimplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,  // 扬声器引脚
            AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);     // 麦克风引脚
#else
        // Duplex模式：共享的I2S配置
        static NoAudioCodecDuplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN);
#endif
        return &audio_codec;
    }

    /**
     * @brief 获取显示屏对象
     * 
     * 返回LCD显示屏对象，用于显示文本、图像等内容。
     */
    virtual Display* GetDisplay() override {
        return display_;
    }

    /**
     * @brief 获取背光控制对象
     * 
     * 如果配置了背光引脚，则返回PWM背光控制对象。
     * 用于调节显示屏亮度。
     */
    virtual Backlight* GetBacklight() override {
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            // 创建PWM背光控制对象，支持亮度调节
            static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
            return &backlight;
        }
        return nullptr;  // 如果没有配置背光引脚，返回空指针
    }
};

// 声明这个开发板类，使其可以被系统识别和使用
DECLARE_BOARD(DehonghaoS3Lcd13Board); 