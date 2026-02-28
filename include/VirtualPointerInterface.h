/**
 * @file VirtualPointerInterface.h
 * @brief 虚拟指针适配器通用接口定义
 * 
 * 本文件定义了虚拟指针APP的抽象接口，支持多种实现（vPointer等）
 * 设计原则：高内聚、低耦合、可扩展
 * 
 * @author Auto-generated for vPointer Integration
 * @version 1.0.0
 */

#ifndef VIRTUAL_POINTER_INTERFACE_H
#define VIRTUAL_POINTER_INTERFACE_H

#include <cstdint>
#include <string>
#include <functional>
#include <memory>

#ifdef VPTR_DLL_EXPORTS
#define VPTR_API __declspec(dllexport)
#else
#define VPTR_API __declspec(dllimport)
#endif

namespace vptr {

/**
 * @brief 指针状态枚举
 */
enum class PointerState : uint8_t {
    Hidden = 0,     ///< 指针隐藏
    Visible = 1,    ///< 指针可见
    Pressed = 2,    ///< 指针按下状态
    Dragging = 3    ///< 指针拖拽状态
};

/**
 * @brief 设备方向枚举
 */
enum class DeviceOrientation : uint8_t {
    Portrait = 0,           ///< 竖屏正常 (0度)
    LandscapeRight = 1,     ///< 横屏向右 (90度)
    PortraitUpsideDown = 2, ///< 竖屏倒置 (180度)
    LandscapeLeft = 3       ///< 横屏向左 (270度)
};

/**
 * @brief 连接状态枚举
 */
enum class ConnectionState : uint8_t {
    Disconnected = 0,   ///< 未连接
    Connecting = 1,     ///< 连接中
    Connected = 2,      ///< 已连接
    Error = 3           ///< 连接错误
};

/**
 * @brief 指针位置结构
 */
struct PointerPosition {
    int32_t x;  ///< X坐标（像素）
    int32_t y;  ///< Y坐标（像素）
    
    PointerPosition() : x(0), y(0) {}
    PointerPosition(int32_t xPos, int32_t yPos) : x(xPos), y(yPos) {}
};

/**
 * @brief 指针事件结构
 */
struct PointerEvent {
    PointerPosition position;   ///< 位置信息
    PointerState state;         ///< 指针状态
    int64_t timestamp;          ///< 时间戳（毫秒）
    uint32_t sequence;          ///< 序列号
    
    PointerEvent() : state(PointerState::Hidden), timestamp(0), sequence(0) {}
};

/**
 * @brief 设备信息结构
 */
struct DeviceInfo {
    std::string deviceId;       ///< 设备标识
    std::string deviceName;     ///< 设备名称
    std::string protocolVer;    ///< 协议版本
    int32_t screenWidth;        ///< 屏幕宽度
    int32_t screenHeight;       ///< 屏幕高度
    DeviceOrientation orientation; ///< 当前方向
    
    DeviceInfo() : screenWidth(0), screenHeight(0), orientation(DeviceOrientation::Portrait) {}
};

/**
 * @brief 配置参数结构
 */
struct AdapterConfig {
    std::string targetIp;       ///< 目标IP地址
    uint16_t targetPort;        ///< 目标端口
    uint16_t localPort;         ///< 本地端口（用于接收回调）
    int32_t connectionTimeout;  ///< 连接超时（毫秒）
    int32_t keepAliveInterval;  ///< 心跳间隔（毫秒）
    bool autoReconnect;         ///< 自动重连
    int32_t maxReconnectAttempts; ///< 最大重连次数
    
    AdapterConfig() 
        : targetPort(6533)
        , localPort(0)
        , connectionTimeout(5000)
        , keepAliveInterval(30000)
        , autoReconnect(true)
        , maxReconnectAttempts(3) {}
};

/**
 * @brief 回调函数类型定义
 */
using OrientationCallback = std::function<void(DeviceOrientation)>;
using ConnectionCallback = std::function<void(ConnectionState, const std::string&)>;
using ErrorCallback = std::function<void(int32_t, const std::string&)>;

/**
 * @brief 虚拟指针适配器抽象接口
 * 
 * 所有虚拟指针APP适配器必须实现此接口
 */
class VPTR_API IVirtualPointerAdapter {
public:
    virtual ~IVirtualPointerAdapter() = default;

    /**
     * @brief 初始化适配器
     * @param config 配置参数
     * @return 是否成功
     */
    virtual bool Initialize(const AdapterConfig& config) = 0;

    /**
     * @brief 关闭适配器
     */
    virtual void Shutdown() = 0;

    /**
     * @brief 连接设备
     * @return 是否成功
     */
    virtual bool Connect() = 0;

    /**
     * @brief 断开连接
     */
    virtual void Disconnect() = 0;

    /**
     * @brief 更新指针位置
     * @param x X坐标
     * @param y Y坐标
     * @return 是否成功
     */
    virtual bool MovePointer(int32_t x, int32_t y) = 0;

    /**
     * @brief 设置指针位置（绝对坐标）
     * @param position 位置结构
     * @return 是否成功
     */
    virtual bool SetPointerPosition(const PointerPosition& position) = 0;

    /**
     * @brief 显示指针
     * @return 是否成功
     */
    virtual bool ShowPointer() = 0;

    /**
     * @brief 隐藏指针
     * @return 是否成功
     */
    virtual bool HidePointer() = 0;

    /**
     * @brief 设置指针按下状态
     * @param pressed 是否按下
     * @return 是否成功
     */
    virtual bool SetPointerPressed(bool pressed) = 0;

    /**
     * @brief 获取当前指针状态
     * @return 指针状态
     */
    virtual PointerState GetPointerState() const = 0;

    /**
     * @brief 获取当前指针位置
     * @return 指针位置
     */
    virtual PointerPosition GetPointerPosition() const = 0;

    /**
     * @brief 获取连接状态
     * @return 连接状态
     */
    virtual ConnectionState GetConnectionState() const = 0;

    /**
     * @brief 获取设备信息
     * @return 设备信息
     */
    virtual DeviceInfo GetDeviceInfo() const = 0;

    /**
     * @brief 设置方向变化回调
     * @param callback 回调函数
     */
    virtual void SetOrientationCallback(OrientationCallback callback) = 0;

    /**
     * @brief 设置连接状态回调
     * @param callback 回调函数
     */
    virtual void SetConnectionCallback(ConnectionCallback callback) = 0;

    /**
     * @brief 设置错误回调
     * @param callback 回调函数
     */
    virtual void SetErrorCallback(ErrorCallback callback) = 0;

    /**
     * @brief 发送原始数据（高级用法）
     * @param data 数据指针
     * @param length 数据长度
     * @return 是否成功
     */
    virtual bool SendRawData(const uint8_t* data, size_t length) = 0;

    /**
     * @brief 获取适配器名称
     * @return 适配器名称
     */
    virtual const char* GetAdapterName() const = 0;

    /**
     * @brief 获取协议版本
     * @return 协议版本
     */
    virtual const char* GetProtocolVersion() const = 0;

    /**
     * @brief 检查设备是否在线
     * @return 是否在线
     */
    virtual bool IsDeviceOnline() const = 0;

    /**
     * @brief 发送心跳包
     * @return 是否成功
     */
    virtual bool SendKeepAlive() = 0;

    /**
     * @brief 获取最后错误信息
     * @return 错误信息
     */
    virtual std::string GetLastErrorMessage() const = 0;
};

/**
 * @brief 适配器智能指针类型
 */
using IVirtualPointerAdapterPtr = std::shared_ptr<IVirtualPointerAdapter>;

/**
 * @brief 创建适配器的工厂函数类型
 */
using AdapterFactoryFunc = IVirtualPointerAdapterPtr(*)();

} // namespace vptr

#endif // VIRTUAL_POINTER_INTERFACE_H
