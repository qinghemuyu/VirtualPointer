/**
 * @file VirtualPointerCAPI.h
 * @brief C++ 虚拟指针接口头文件
 * 
 * 为 C++ 程序提供虚拟指针适配器接口
 * 
 * @author Auto-generated for vPointer Integration
 * @version 1.0.0
 */

#ifndef VIRTUAL_POINTER_C_API_H
#define VIRTUAL_POINTER_C_API_H

#include <stdint.h>
#include <stdbool.h>

#define VPTR_C_API __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 类型定义 ==================== */

/**
 * @brief 适配器句柄
 */
typedef void* VPTR_HANDLE;

/**
 * @brief 指针状态
 */
typedef enum {
    VPTR_STATE_HIDDEN = 0,      ///< 隐藏
    VPTR_STATE_VISIBLE = 1,     ///< 可见
    VPTR_STATE_PRESSED = 2,     ///< 按下
    VPTR_STATE_DRAGGING = 3     ///< 拖拽
} VPTR_POINTER_STATE;

/**
 * @brief 设备方向
 */
typedef enum {
    VPTR_ORIENT_PORTRAIT = 0,           ///< 竖屏 (0度)
    VPTR_ORIENT_LANDSCAPE_RIGHT = 1,    ///< 横屏右 (90度)
    VPTR_ORIENT_PORTRAIT_UPSIDE = 2,    ///< 竖屏倒置 (180度)
    VPTR_ORIENT_LANDSCAPE_LEFT = 3      ///< 横屏左 (270度)
} VPTR_ORIENTATION;

/**
 * @brief 连接状态
 */
typedef enum {
    VPTR_CONN_DISCONNECTED = 0, ///< 未连接
    VPTR_CONN_CONNECTING = 1,   ///< 连接中
    VPTR_CONN_CONNECTED = 2,    ///< 已连接
    VPTR_CONN_ERROR = 3         ///< 错误
} VPTR_CONNECTION_STATE;

/**
 * @brief 错误代码
 */
typedef enum {
    VPTR_ERR_NONE = 0,          ///< 无错误
    VPTR_ERR_INIT_FAILED = 1,   ///< 初始化失败
    VPTR_ERR_CONNECT_FAILED = 2,///< 连接失败
    VPTR_ERR_SEND_FAILED = 3,   ///< 发送失败
    VPTR_ERR_INVALID_PARAM = 4, ///< 无效参数
    VPTR_ERR_NOT_CONNECTED = 5, ///< 未连接
    VPTR_ERR_SOCKET_ERROR = 6,  ///< 套接字错误
    VPTR_ERR_TIMEOUT = 7,       ///< 超时
    VPTR_ERR_UNKNOWN = 99       ///< 未知错误
} VPTR_ERROR_CODE;

/**
 * @brief 配置结构
 */
typedef struct {
    const char* targetIp;       ///< 目标IP地址
    uint16_t targetPort;        ///< 目标端口 (默认6533)
    uint16_t localPort;         ///< 本地端口 (0表示自动)
    int32_t connectionTimeout;  ///< 连接超时(ms)
    int32_t keepAliveInterval;  ///< 心跳间隔(ms)
    bool autoReconnect;         ///< 自动重连
    int32_t maxReconnectAttempts; ///< 最大重连次数
} VPTR_CONFIG;

/**
 * @brief 指针位置结构
 */
typedef struct {
    int32_t x;
    int32_t y;
} VPTR_POSITION;

/**
 * @brief 设备信息结构
 */
typedef struct {
    char deviceId[64];          ///< 设备ID
    char deviceName[128];       ///< 设备名称
    char protocolVer[32];       ///< 协议版本
    int32_t screenWidth;        ///< 屏幕宽度
    int32_t screenHeight;       ///< 屏幕高度
    VPTR_ORIENTATION orientation; ///< 当前方向
} VPTR_DEVICE_INFO;

/* ==================== 回调函数类型 ==================== */

/**
 * @brief 方向变化回调
 * @param orientation 新方向
 * @param userData 用户数据
 */
typedef void (*VPTR_ORIENTATION_CALLBACK)(VPTR_ORIENTATION orientation, void* userData);

/**
 * @brief 连接状态回调
 * @param state 连接状态
 * @param message 状态消息
 * @param userData 用户数据
 */
typedef void (*VPTR_CONNECTION_CALLBACK)(VPTR_CONNECTION_STATE state, const char* message, void* userData);

/**
 * @brief 错误回调
 * @param code 错误代码
 * @param message 错误消息
 * @param userData 用户数据
 */
typedef void (*VPTR_ERROR_CALLBACK)(int32_t code, const char* message, void* userData);

/* ==================== 核心API函数 ==================== */

/**
 * @brief 创建适配器实例
 * @param adapterType 适配器类型 ("vPointer" 或其他)
 * @return 适配器句柄，失败返回NULL
 */
VPTR_C_API VPTR_HANDLE VPTR_CreateAdapter(const char* adapterType);

/**
 * @brief 销毁适配器实例
 * @param handle 适配器句柄
 */
VPTR_C_API void VPTR_DestroyAdapter(VPTR_HANDLE handle);

/**
 * @brief 初始化适配器
 * @param handle 适配器句柄
 * @param config 配置参数
 * @return 是否成功
 */
VPTR_C_API bool VPTR_Initialize(VPTR_HANDLE handle, const VPTR_CONFIG* config);

/**
 * @brief 关闭适配器
 * @param handle 适配器句柄
 */
VPTR_C_API void VPTR_Shutdown(VPTR_HANDLE handle);

/**
 * @brief 连接设备
 * @param handle 适配器句柄
 * @return 是否成功
 */
VPTR_C_API bool VPTR_Connect(VPTR_HANDLE handle);

/**
 * @brief 断开连接
 * @param handle 适配器句柄
 */
VPTR_C_API void VPTR_Disconnect(VPTR_HANDLE handle);

/**
 * @brief 移动指针到指定位置
 * @param handle 适配器句柄
 * @param x X坐标
 * @param y Y坐标
 * @return 是否成功
 */
VPTR_C_API bool VPTR_MovePointer(VPTR_HANDLE handle, int32_t x, int32_t y);

/**
 * @brief 设置指针位置
 * @param handle 适配器句柄
 * @param position 位置结构
 * @return 是否成功
 */
VPTR_C_API bool VPTR_SetPointerPosition(VPTR_HANDLE handle, const VPTR_POSITION* position);

/**
 * @brief 显示指针
 * @param handle 适配器句柄
 * @return 是否成功
 */
VPTR_C_API bool VPTR_ShowPointer(VPTR_HANDLE handle);

/**
 * @brief 隐藏指针
 * @param handle 适配器句柄
 * @return 是否成功
 */
VPTR_C_API bool VPTR_HidePointer(VPTR_HANDLE handle);

/**
 * @brief 设置指针按下状态
 * @param handle 适配器句柄
 * @param pressed 是否按下
 * @return 是否成功
 */
VPTR_C_API bool VPTR_SetPointerPressed(VPTR_HANDLE handle, bool pressed);

/**
 * @brief 获取指针状态
 * @param handle 适配器句柄
 * @return 指针状态
 */
VPTR_C_API VPTR_POINTER_STATE VPTR_GetPointerState(VPTR_HANDLE handle);

/**
 * @brief 获取指针位置
 * @param handle 适配器句柄
 * @param position 输出位置结构
 * @return 是否成功
 */
VPTR_C_API bool VPTR_GetPointerPosition(VPTR_HANDLE handle, VPTR_POSITION* position);

/**
 * @brief 获取连接状态
 * @param handle 适配器句柄
 * @return 连接状态
 */
VPTR_C_API VPTR_CONNECTION_STATE VPTR_GetConnectionState(VPTR_HANDLE handle);

/**
 * @brief 获取设备信息
 * @param handle 适配器句柄
 * @param info 输出设备信息结构
 * @return 是否成功
 */
VPTR_C_API bool VPTR_GetDeviceInfo(VPTR_HANDLE handle, VPTR_DEVICE_INFO* info);

/**
 * @brief 检查设备是否在线
 * @param handle 适配器句柄
 * @return 是否在线
 */
VPTR_C_API bool VPTR_IsDeviceOnline(VPTR_HANDLE handle);

/**
 * @brief 发送心跳包
 * @param handle 适配器句柄
 * @return 是否成功
 */
VPTR_C_API bool VPTR_SendKeepAlive(VPTR_HANDLE handle);

/**
 * @brief 设置方向变化回调
 * @param handle 适配器句柄
 * @param callback 回调函数
 * @param userData 用户数据
 */
VPTR_C_API void VPTR_SetOrientationCallback(VPTR_HANDLE handle, VPTR_ORIENTATION_CALLBACK callback, void* userData);

/**
 * @brief 设置连接状态回调
 * @param handle 适配器句柄
 * @param callback 回调函数
 * @param userData 用户数据
 */
VPTR_C_API void VPTR_SetConnectionCallback(VPTR_HANDLE handle, VPTR_CONNECTION_CALLBACK callback, void* userData);

/**
 * @brief 设置错误回调
 * @param handle 适配器句柄
 * @param callback 回调函数
 * @param userData 用户数据
 */
VPTR_C_API void VPTR_SetErrorCallback(VPTR_HANDLE handle, VPTR_ERROR_CALLBACK callback, void* userData);

/**
 * @brief 发送原始数据
 * @param handle 适配器句柄
 * @param data 数据指针
 * @param length 数据长度
 * @return 是否成功
 */
VPTR_C_API bool VPTR_SendRawData(VPTR_HANDLE handle, const uint8_t* data, size_t length);

/**
 * @brief 获取适配器名称
 * @param handle 适配器句柄
 * @param buffer 输出缓冲区
 * @param bufferSize 缓冲区大小
 * @return 是否成功
 */
VPTR_C_API bool VPTR_GetAdapterName(VPTR_HANDLE handle, char* buffer, size_t bufferSize);

/**
 * @brief 获取协议版本
 * @param handle 适配器句柄
 * @param buffer 输出缓冲区
 * @param bufferSize 缓冲区大小
 * @return 是否成功
 */
VPTR_C_API bool VPTR_GetProtocolVersion(VPTR_HANDLE handle, char* buffer, size_t bufferSize);

/**
 * @brief 获取最后错误信息
 * @param handle 适配器句柄
 * @param buffer 输出缓冲区
 * @param bufferSize 缓冲区大小
 * @return 是否成功
 */
VPTR_C_API bool VPTR_GetLastError(VPTR_HANDLE handle, char* buffer, size_t bufferSize);

/* ==================== 便捷函数 ==================== */

/**
 * @brief 快速创建并连接（一站式函数）
 * @param targetIp 目标IP地址
 * @param targetPort 目标端口
 * @return 适配器句柄，失败返回NULL
 */
VPTR_C_API VPTR_HANDLE VPTR_CreateAndConnect(const char* targetIp, uint16_t targetPort);

/**
 * @brief 更新指针（位置+显示状态+按下状态）
 * @param handle 适配器句柄
 * @param x X坐标
 * @param y Y坐标
 * @param visible 是否可见
 * @param pressed 是否按下
 * @return 是否成功
 */
VPTR_C_API bool VPTR_UpdatePointer(VPTR_HANDLE handle, int32_t x, int32_t y, bool visible, bool pressed);

/**
 * @brief 获取版本信息
 * @param buffer 输出缓冲区
 * @param bufferSize 缓冲区大小
 */
VPTR_C_API void VPTR_GetVersion(char* buffer, size_t bufferSize);

/**
 * @brief 获取支持的适配器类型列表
 * @param types 输出类型数组（以NULL结尾的字符串数组）
 * @param maxCount 最大数量
 * @return 实际数量
 */
VPTR_C_API int32_t VPTR_GetSupportedAdapterTypes(const char** types, int32_t maxCount);

#ifdef __cplusplus
}
#endif

#endif // VIRTUAL_POINTER_C_API_H
