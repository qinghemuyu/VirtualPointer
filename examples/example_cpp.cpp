/**
 * @file example_cpp.cpp
 * @brief C++语言使用示例
 * 
 * 演示如何使用C++接口调用VirtualPointerDLL
 * 展示面向对象的使用方式
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>
#include "VirtualPointerInterface.h"
#include "VPointerAdapter.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace vptr;

// 方向变化处理函数
void OnOrientationChanged(DeviceOrientation orientation) {
    const char* orientStr;
    switch (orientation) {
        case DeviceOrientation::Portrait: orientStr = "竖屏(0度)"; break;
        case DeviceOrientation::LandscapeRight: orientStr = "横屏右(90度)"; break;
        case DeviceOrientation::PortraitUpsideDown: orientStr = "竖屏倒置(180度)"; break;
        case DeviceOrientation::LandscapeLeft: orientStr = "横屏左(270度)"; break;
        default: orientStr = "未知"; break;
    }
    std::cout << "[回调] 设备方向变化: " << orientStr << std::endl;
}

// 连接状态处理函数
void OnConnectionChanged(ConnectionState state, const std::string& message) {
    const char* stateStr;
    switch (state) {
        case ConnectionState::Disconnected: stateStr = "未连接"; break;
        case ConnectionState::Connecting: stateStr = "连接中"; break;
        case ConnectionState::Connected: stateStr = "已连接"; break;
        case ConnectionState::Error: stateStr = "错误"; break;
        default: stateStr = "未知"; break;
    }
    std::cout << "[回调] 连接状态: " << stateStr << " - " << message << std::endl;
}

// 错误处理函数
void OnError(int32_t code, const std::string& message) {
    std::cerr << "[回调] 错误 [" << code << "]: " << message << std::endl;
}

// 演示基本操作
void DemoBasicOperations(IVirtualPointerAdapterPtr adapter) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  基本操作演示" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 显示指针
    std::cout << "1. 显示指针在 (100, 100)..." << std::endl;
    adapter->SetPointerPosition(PointerPosition(100, 100));
    adapter->ShowPointer();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // 移动指针 - 画一个正方形
    std::cout << "2. 移动指针画正方形..." << std::endl;
    
    std::cout << "   移动到 (300, 100)..." << std::endl;
    adapter->MovePointer(300, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "   移动到 (300, 300)..." << std::endl;
    adapter->MovePointer(300, 300);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "   移动到 (100, 300)..." << std::endl;
    adapter->MovePointer(100, 300);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "   移动到 (100, 100)..." << std::endl;
    adapter->MovePointer(100, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // 按下效果演示
    std::cout << "\n3. 按下效果演示..." << std::endl;
    for (int i = 0; i < 3; i++) {
        std::cout << "   按下..." << std::endl;
        adapter->SetPointerPressed(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        std::cout << "   释放..." << std::endl;
        adapter->SetPointerPressed(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // 隐藏指针
    std::cout << "\n4. 隐藏指针..." << std::endl;
    adapter->HidePointer();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

// 演示高级操作
void DemoAdvancedOperations(IVirtualPointerAdapterPtr adapter) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  高级操作演示" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 获取设备信息
    std::cout << "设备信息:" << std::endl;
    DeviceInfo info = adapter->GetDeviceInfo();
    std::cout << "  设备ID: " << info.deviceId << std::endl;
    std::cout << "  设备名称: " << info.deviceName << std::endl;
    std::cout << "  协议版本: " << info.protocolVer << std::endl;
    std::cout << "  屏幕尺寸: " << info.screenWidth << "x" << info.screenHeight << std::endl;
    std::cout << "  当前方向: " << static_cast<int>(info.orientation) << std::endl;

    // 获取当前状态
    std::cout << "\n当前状态:" << std::endl;
    std::cout << "  连接状态: " << static_cast<int>(adapter->GetConnectionState()) << std::endl;
    std::cout << "  指针状态: " << static_cast<int>(adapter->GetPointerState()) << std::endl;
    PointerPosition pos = adapter->GetPointerPosition();
    std::cout << "  指针位置: (" << pos.x << ", " << pos.y << ")" << std::endl;
    std::cout << "  在线状态: " << (adapter->IsDeviceOnline() ? "是" : "否") << std::endl;

    // 发送心跳
    std::cout << "\n发送心跳包..." << std::endl;
    if (adapter->SendKeepAlive()) {
        std::cout << "  心跳发送成功" << std::endl;
    } else {
        std::cout << "  心跳发送失败" << std::endl;
    }

    // 发送原始数据（高级用法）
    std::cout << "\n发送原始数据..." << std::endl;
    std::string rawCmd = "100,100,1,0,0";  // x,y,show,down,orientation
    if (adapter->SendRawData(reinterpret_cast<const uint8_t*>(rawCmd.c_str()), 
                              rawCmd.length())) {
        std::cout << "  原始数据发送成功" << std::endl;
    }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // 设置控制台为UTF-8
#endif

    std::cout << "========================================" << std::endl;
    std::cout << "  VirtualPointer DLL - C++示例程序" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 目标设备IP（请根据实际情况修改）
    std::string targetIp = (argc > 1) ? argv[1] : "192.168.1.100";
    uint16_t targetPort = (argc > 2) ? static_cast<uint16_t>(std::stoi(argv[2])) : 6533;

    std::cout << "目标设备: " << targetIp << ":" << targetPort << "\n" << std::endl;

    // ========================================
    // 方式1：使用工厂函数创建适配器
    // ========================================
    std::cout << "[方式1] 使用工厂函数创建适配器" << std::endl;
    IVirtualPointerAdapterPtr adapter = CreateVPointerAdapter();
    
    if (!adapter) {
        std::cerr << "错误: 无法创建适配器" << std::endl;
        return 1;
    }

    // 配置参数
    AdapterConfig config;
    config.targetIp = targetIp;
    config.targetPort = targetPort;
    config.localPort = 0;  // 0表示不接收回调
    config.connectionTimeout = 5000;
    config.keepAliveInterval = 30000;
    config.autoReconnect = true;
    config.maxReconnectAttempts = 3;

    // 初始化
    std::cout << "初始化适配器..." << std::endl;
    if (!adapter->Initialize(config)) {
        std::cerr << "错误: 初始化失败" << std::endl;
        return 1;
    }

    // 设置回调
    std::cout << "设置回调函数..." << std::endl;
    adapter->SetOrientationCallback(OnOrientationChanged);
    adapter->SetConnectionCallback(OnConnectionChanged);
    adapter->SetErrorCallback(OnError);

    // 连接设备
    std::cout << "连接设备..." << std::endl;
    if (!adapter->Connect()) {
        std::cerr << "错误: 连接失败" << std::endl;
        std::cerr << "请确保:" << std::endl;
        std::cerr << "  1. vPointer APP已在Android设备上运行" << std::endl;
        std::cerr << "  2. 设备和电脑在同一网络" << std::endl;
        std::cerr << "  3. IP地址和端口正确" << std::endl;
        return 1;
    }

    // 输出适配器信息
    std::cout << "\n适配器信息:" << std::endl;
    std::cout << "  名称: " << adapter->GetAdapterName() << std::endl;
    std::cout << "  协议版本: " << adapter->GetProtocolVersion() << std::endl;

    // 执行演示
    DemoBasicOperations(adapter);
    DemoAdvancedOperations(adapter);

    // ========================================
    // 方式2：直接使用VPointerAdapter类
    // ========================================
    std::cout << "\n========================================" << std::endl;
    std::cout << "  方式2：直接使用具体类" << std::endl;
    std::cout << "========================================" << std::endl;
    
    {
        auto vptrAdapter = std::make_shared<VPointerAdapter>();
        vptrAdapter->SetDebugMode(true);  // 启用调试模式
        
        if (vptrAdapter->Initialize(config)) {
            std::cout << "VPointerAdapter初始化成功" << std::endl;
            
            if (vptrAdapter->Connect()) {
                std::cout << "连接成功，发送测试数据..." << std::endl;
                vptrAdapter->ShowPointer();
                vptrAdapter->MovePointer(500, 500);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                vptrAdapter->HidePointer();
                vptrAdapter->Disconnect();
            }
        }
    }

    // 断开连接
    std::cout << "\n断开连接并清理..." << std::endl;
    adapter->Disconnect();
    adapter->Shutdown();

    std::cout << "示例程序结束." << std::endl;
    std::cout << "按任意键退出..." << std::endl;
    system("pause");
    return 0;
}
