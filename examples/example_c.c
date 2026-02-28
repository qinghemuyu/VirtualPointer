/**
 * @file example_c.c
 * @brief C语言使用示例
 * 
 * 演示如何使用C接口调用VirtualPointerDLL
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "VirtualPointerCAPI.h"

// 方向变化回调
void OnOrientationChanged(VPTR_ORIENTATION orientation, void* userData) {
    const char* orientStr;
    switch (orientation) {
        case VPTR_ORIENT_PORTRAIT: orientStr = "竖屏(0度)"; break;
        case VPTR_ORIENT_LANDSCAPE_RIGHT: orientStr = "横屏右(90度)"; break;
        case VPTR_ORIENT_PORTRAIT_UPSIDE: orientStr = "竖屏倒置(180度)"; break;
        case VPTR_ORIENT_LANDSCAPE_LEFT: orientStr = "横屏左(270度)"; break;
        default: orientStr = "未知"; break;
    }
    printf("[回调] 设备方向变化: %s\n", orientStr);
}

// 连接状态回调
void OnConnectionChanged(VPTR_CONNECTION_STATE state, const char* message, void* userData) {
    const char* stateStr;
    switch (state) {
        case VPTR_CONN_DISCONNECTED: stateStr = "未连接"; break;
        case VPTR_CONN_CONNECTING: stateStr = "连接中"; break;
        case VPTR_CONN_CONNECTED: stateStr = "已连接"; break;
        case VPTR_CONN_ERROR: stateStr = "错误"; break;
        default: stateStr = "未知"; break;
    }
    printf("[回调] 连接状态: %s - %s\n", stateStr, message);
}

// 错误回调
void OnError(int32_t code, const char* message, void* userData) {
    printf("[回调] 错误 [%d]: %s\n", code, message);
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // 设置控制台为UTF-8
#endif

    printf("========================================\n");
    printf("  VirtualPointer DLL - C示例程序\n");
    printf("========================================\n\n");

    // 获取版本信息
    char version[32];
    VPTR_GetVersion(version, sizeof(version));
    printf("DLL版本: %s\n\n", version);

    // 获取支持的适配器类型
    const char* types[10];
    int32_t typeCount = VPTR_GetSupportedAdapterTypes(types, 10);
    printf("支持的适配器类型 (%d个):\n", typeCount);
    for (int32_t i = 0; i < typeCount; i++) {
        printf("  - %s\n", types[i]);
    }
    printf("\n");

    // 目标设备IP（请根据实际情况修改）
    const char* targetIp = (argc > 1) ? argv[1] : "192.168.1.100";
    uint16_t targetPort = (argc > 2) ? (uint16_t)atoi(argv[2]) : 6533;

    printf("目标设备: %s:%d\n\n", targetIp, targetPort);

    // 方式1：使用便捷函数快速创建并连接
    printf("[方式1] 使用VPTR_CreateAndConnect...\n");
    VPTR_HANDLE handle = VPTR_CreateAndConnect(targetIp, targetPort);
    
    if (!handle) {
        printf("错误: 无法创建或连接到设备\n");
        printf("请确保:\n");
        printf("  1. vPointer APP已在Android设备上运行\n");
        printf("  2. 设备和电脑在同一网络\n");
        printf("  3. IP地址和端口正确\n");
        return 1;
    }

    // 设置回调
    printf("设置回调函数...\n");
    VPTR_SetOrientationCallback(handle, OnOrientationChanged, NULL);
    VPTR_SetConnectionCallback(handle, OnConnectionChanged, NULL);
    VPTR_SetErrorCallback(handle, OnError, NULL);

    // 获取适配器信息
    char adapterName[64];
    char protocolVer[32];
    VPTR_GetAdapterName(handle, adapterName, sizeof(adapterName));
    VPTR_GetProtocolVersion(handle, protocolVer, sizeof(protocolVer));
    printf("适配器: %s (协议版本: %s)\n\n", adapterName, protocolVer);

    // 获取设备信息
    VPTR_DEVICE_INFO deviceInfo;
    if (VPTR_GetDeviceInfo(handle, &deviceInfo)) {
        printf("设备信息:\n");
        printf("  设备ID: %s\n", deviceInfo.deviceId);
        printf("  设备名称: %s\n", deviceInfo.deviceName);
        printf("  屏幕尺寸: %dx%d\n", deviceInfo.screenWidth, deviceInfo.screenHeight);
        printf("  当前方向: %d\n", deviceInfo.orientation);
        printf("\n");
    }

    // 检查连接状态
    VPTR_CONNECTION_STATE connState = VPTR_GetConnectionState(handle);
    printf("连接状态: %d\n\n", connState);

    // 演示指针控制
    printf("========================================\n");
    printf("  开始指针控制演示\n");
    printf("========================================\n\n");

    // 显示指针
    printf("1. 显示指针在 (100, 100)...\n");
    VPTR_UpdatePointer(handle, 100, 100, true, false);
    Sleep(1000);

    // 移动指针
    printf("2. 移动指针到 (200, 100)...\n");
    VPTR_MovePointer(handle, 200, 100);
    Sleep(500);

    printf("3. 移动指针到 (200, 200)...\n");
    VPTR_MovePointer(handle, 200, 200);
    Sleep(500);

    printf("4. 移动指针到 (100, 200)...\n");
    VPTR_MovePointer(handle, 100, 200);
    Sleep(500);

    printf("5. 移动指针到 (100, 100)...\n");
    VPTR_MovePointer(handle, 100, 100);
    Sleep(500);

    // 按下效果
    printf("6. 模拟按下...\n");
    VPTR_SetPointerPressed(handle, true);
    Sleep(500);

    printf("7. 释放按下...\n");
    VPTR_SetPointerPressed(handle, false);
    Sleep(500);

    // 隐藏指针
    printf("8. 隐藏指针...\n");
    VPTR_HidePointer(handle);
    Sleep(1000);

    // 获取当前状态
    VPTR_POINTER_STATE state = VPTR_GetPointerState(handle);
    VPTR_POSITION pos;
    VPTR_GetPointerPosition(handle, &pos);
    printf("\n当前状态: 状态=%d, 位置=(%d, %d)\n", state, pos.x, pos.y);

    // 发送心跳
    printf("\n发送心跳包...\n");
    VPTR_SendKeepAlive(handle);

    // 清理
    printf("\n断开连接并清理...\n");
    VPTR_Disconnect(handle);
    VPTR_DestroyAdapter(handle);

    printf("\n示例程序结束.\n");
    printf("按任意键退出...\n");
    system("pause");
    return 0;
}
