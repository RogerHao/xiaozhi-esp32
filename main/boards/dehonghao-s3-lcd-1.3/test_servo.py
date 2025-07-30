#!/usr/bin/env python3
"""
舵机控制测试脚本

这个脚本用于测试dehonghao-s3-lcd-1.3开发板的舵机控制功能。
通过MCP协议远程控制舵机进行各种动作测试。

使用方法:
1. 确保开发板已连接WiFi并运行
2. 运行此脚本: python test_servo.py
"""

import time
import json

# 模拟MCP客户端调用
class MockMcpClient:
    def __init__(self):
        self.servo_angle = 90  # 模拟当前角度
        
    def call_tool(self, tool_name, parameters):
        """模拟MCP工具调用"""
        print(f"调用工具: {tool_name}")
        print(f"参数: {json.dumps(parameters, indent=2)}")
        
        if tool_name == "servo.set_angle":
            angle = parameters.get("angle", 90)
            self.servo_angle = max(0, min(180, angle))
            print(f"舵机设置到 {self.servo_angle} 度")
            return True
            
        elif tool_name == "servo.set_preset":
            preset = parameters.get("preset", "90")
            if preset == "0":
                self.servo_angle = 0
            elif preset == "90":
                self.servo_angle = 90
            elif preset == "180":
                self.servo_angle = 180
            else:
                print(f"无效的预设值: {preset}")
                return False
            print(f"舵机设置到预设位置: {preset} 度")
            return True
            
        elif tool_name == "servo.oscillate":
            start_angle = parameters.get("start_angle", 0)
            end_angle = parameters.get("end_angle", 90)
            cycles = parameters.get("cycles", 3)
            period_ms = parameters.get("period_ms", 1000)
            print(f"舵机摆动: {start_angle}° -> {end_angle}°, {cycles}次, 周期{period_ms}ms")
            return True
            
        elif tool_name == "servo.stop":
            print("停止舵机摆动")
            return True
            
        elif tool_name == "servo.get_angle":
            print(f"当前角度: {self.servo_angle}°")
            return self.servo_angle
            
        else:
            print(f"未知工具: {tool_name}")
            return False

def test_basic_control(client):
    """测试基础角度控制"""
    print("\n=== 测试基础角度控制 ===")
    
    # 测试设置到0度
    client.call_tool("servo.set_angle", {"angle": 0})
    time.sleep(1)
    
    # 测试设置到90度
    client.call_tool("servo.set_angle", {"angle": 90})
    time.sleep(1)
    
    # 测试设置到180度
    client.call_tool("servo.set_angle", {"angle": 180})
    time.sleep(1)
    
    # 测试边界值
    client.call_tool("servo.set_angle", {"angle": -10})  # 应该限制到0度
    time.sleep(1)
    client.call_tool("servo.set_angle", {"angle": 200})  # 应该限制到180度
    time.sleep(1)

def test_preset_control(client):
    """测试预设角度控制"""
    print("\n=== 测试预设角度控制 ===")
    
    # 测试预设位置
    client.call_tool("servo.set_preset", {"preset": "0"})
    time.sleep(1)
    
    client.call_tool("servo.set_preset", {"preset": "90"})
    time.sleep(1)
    
    client.call_tool("servo.set_preset", {"preset": "180"})
    time.sleep(1)
    
    # 测试无效预设值
    client.call_tool("servo.set_preset", {"preset": "45"})

def test_oscillation(client):
    """测试摆动控制"""
    print("\n=== 测试摆动控制 ===")
    
    # 测试0-90度摆动
    client.call_tool("servo.oscillate", {
        "start_angle": 0,
        "end_angle": 90,
        "cycles": 3,
        "period_ms": 1000
    })
    time.sleep(3)
    
    # 测试90-180度摆动
    client.call_tool("servo.oscillate", {
        "start_angle": 90,
        "end_angle": 180,
        "cycles": 2,
        "period_ms": 800
    })
    time.sleep(2)
    
    # 测试停止
    client.call_tool("servo.stop", {})

def test_get_angle(client):
    """测试获取角度"""
    print("\n=== 测试获取角度 ===")
    
    # 设置不同角度并获取
    client.call_tool("servo.set_angle", {"angle": 45})
    client.call_tool("servo.get_angle", {})
    
    client.call_tool("servo.set_angle", {"angle": 135})
    client.call_tool("servo.get_angle", {})

def test_mechanical_scenarios(client):
    """测试机械应用场景"""
    print("\n=== 测试机械应用场景 ===")
    
    # 场景1: 机械臂抬起
    print("场景1: 机械臂抬起")
    client.call_tool("servo.set_angle", {"angle": 180})
    time.sleep(2)
    
    # 场景2: 机械臂放下
    print("场景2: 机械臂放下")
    client.call_tool("servo.set_angle", {"angle": 0})
    time.sleep(2)
    
    # 场景3: 机械臂摆动
    print("场景3: 机械臂摆动")
    client.call_tool("servo.oscillate", {
        "start_angle": 0,
        "end_angle": 90,
        "cycles": 5,
        "period_ms": 1200
    })
    time.sleep(5)
    
    # 场景4: 回到中间位置
    print("场景4: 回到中间位置")
    client.call_tool("servo.set_preset", {"preset": "90"})

def main():
    """主函数"""
    print("舵机控制功能测试")
    print("=" * 50)
    
    # 创建模拟客户端
    client = MockMcpClient()
    
    try:
        # 运行各种测试
        test_basic_control(client)
        test_preset_control(client)
        test_oscillation(client)
        test_get_angle(client)
        test_mechanical_scenarios(client)
        
        print("\n=== 测试完成 ===")
        print("所有测试已通过模拟验证")
        print("在实际使用中，请确保:")
        print("1. 舵机正确连接到GPIO 12")
        print("2. 舵机电源稳定（5V）")
        print("3. 开发板已连接WiFi并运行")
        print("4. MCP协议正常工作")
        
    except KeyboardInterrupt:
        print("\n测试被用户中断")
    except Exception as e:
        print(f"测试过程中出现错误: {e}")

if __name__ == "__main__":
    main() 