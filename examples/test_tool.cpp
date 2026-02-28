/**
 * @file test_tool.cpp
 * @brief 简单测试工具
 * 
 * 命令行工具，用于测试vPointer连接
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include "VirtualPointerCAPI.h"

#ifdef _WIN32
#include <windows.h>
#define SLEEP(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP(ms) usleep((ms) * 1000)
#endif

void PrintUsage(const char* programName) {
    std::cout << "用法: " << programName << " <IP地址> [端口] [命令]\n" << std::endl;
    std::cout << "命令:" << std::endl;
    std::cout << "  move <x> <y>  - 移动指针到指定位置" << std::endl;
    std::cout << "  show          - 显示指针" << std::endl;
    std::cout << "  hide          - 隐藏指针" << std::endl;
    std::cout << "  press         - 按下指针" << std::endl;
    std::cout << "  release       - 释放指针" << std::endl;
    std::cout << "  demo          - 运行演示动画" << std::endl;
    std::cout << "  test          - 测试连接" << std::endl;
    std::cout << "\n示例:" << std::endl;
    std::cout << "  " << programName << " 192.168.1.100" << std::endl;
    std::cout << "  " << programName << " 192.168.1.100 6533 move 100 200" << std::endl;
    std::cout << "  " << programName << " 192.168.1.100 6533 demo" << std::endl;
}

void RunDemo(VPTR_HANDLE handle) {
    std::cout << "运行演示动画..." << std::endl;
    
    // 显示并移动指针
    std::cout << "显示指针..." << std::endl;
    VPTR_ShowPointer(handle);
    SLEEP(500);
    
    // 画正方形
    int positions[][2] = {{100, 100}, {400, 100}, {400, 400}, {100, 400}, {100, 100}};
    for (int i = 0; i < 5; i++) {
        std::cout << "移动到 (" << positions[i][0] << ", " << positions[i][1] << ")" << std::endl;
        VPTR_MovePointer(handle, positions[i][0], positions[i][1]);
        SLEEP(300);
    }
    
    // 按下演示
    std::cout << "按下..." << std::endl;
    VPTR_SetPointerPressed(handle, true);
    SLEEP(500);
    
    std::cout << "释放..." << std::endl;
    VPTR_SetPointerPressed(handle, false);
    SLEEP(500);
    
    // 隐藏
    std::cout << "隐藏指针..." << std::endl;
    VPTR_HidePointer(handle);
}

void TestConnection(VPTR_HANDLE handle) {
    std::cout << "测试连接..." << std::endl;
    
    // 获取设备信息
    VPTR_DEVICE_INFO info;
    if (VPTR_GetDeviceInfo(handle, &info)) {
        std::cout << "设备信息:" << std::endl;
        std::cout << "  设备ID: " << info.deviceId << std::endl;
        std::cout << "  设备名称: " << info.deviceName << std::endl;
        std::cout << "  协议版本: " << info.protocolVer << std::endl;
    }
    
    // 获取状态
    VPTR_CONNECTION_STATE connState = VPTR_GetConnectionState(handle);
    VPTR_POINTER_STATE ptrState = VPTR_GetPointerState(handle);
    VPTR_POSITION pos;
    VPTR_GetPointerPosition(handle, &pos);
    
    std::cout << "\n状态信息:" << std::endl;
    std::cout << "  连接状态: " << connState << std::endl;
    std::cout << "  指针状态: " << ptrState << std::endl;
    std::cout << "  指针位置: (" << pos.x << ", " << pos.y << ")" << std::endl;
    std::cout << "  在线状态: " << (VPTR_IsDeviceOnline(handle) ? "是" : "否") << std::endl;
    
    // 发送测试数据
    std::cout << "\n发送测试数据..." << std::endl;
    if (VPTR_SendKeepAlive(handle)) {
        std::cout << "  心跳发送成功" << std::endl;
    } else {
        std::cout << "  心跳发送失败" << std::endl;
    }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // 设置控制台为UTF-8
#endif

    if (argc < 2) {
        PrintUsage(argv[0]);
        std::cout << "按任意键退出..." << std::endl;
        system("pause");
        return 1;
    }

    const char* ip = argv[1];
    uint16_t port = (argc > 2) ? static_cast<uint16_t>(std::atoi(argv[2])) : 6533;
    const char* command = (argc > 3) ? argv[3] : "test";

    std::cout << "VirtualPointer 测试工具" << std::endl;
    std::cout << "=======================" << std::endl;
    std::cout << "目标: " << ip << ":" << port << std::endl;
    std::cout << "命令: " << command << "\n" << std::endl;

    // 创建并连接
    std::cout << "连接到设备..." << std::endl;
    VPTR_HANDLE handle = VPTR_CreateAndConnect(ip, port);
    
    if (!handle) {
        std::cerr << "错误: 无法连接到设备" << std::endl;
        return 1;
    }

    std::cout << "连接成功!\n" << std::endl;

    // 执行命令
    if (strcmp(command, "move") == 0 && argc >= 6) {
        int x = std::atoi(argv[4]);
        int y = std::atoi(argv[5]);
        std::cout << "移动指针到 (" << x << ", " << y << ")" << std::endl;
        VPTR_MovePointer(handle, x, y);
    }
    else if (strcmp(command, "show") == 0) {
        std::cout << "显示指针" << std::endl;
        VPTR_ShowPointer(handle);
    }
    else if (strcmp(command, "hide") == 0) {
        std::cout << "隐藏指针" << std::endl;
        VPTR_HidePointer(handle);
    }
    else if (strcmp(command, "press") == 0) {
        std::cout << "按下指针" << std::endl;
        VPTR_SetPointerPressed(handle, true);
    }
    else if (strcmp(command, "release") == 0) {
        std::cout << "释放指针" << std::endl;
        VPTR_SetPointerPressed(handle, false);
    }
    else if (strcmp(command, "demo") == 0) {
        RunDemo(handle);
    }
    else if (strcmp(command, "test") == 0) {
        TestConnection(handle);
    }
    else {
        std::cout << "未知命令: " << command << std::endl;
        PrintUsage(argv[0]);
    }

    // 清理
    VPTR_Disconnect(handle);
    VPTR_DestroyAdapter(handle);

    std::cout << "操作完成." << std::endl;
    return 0;
}
