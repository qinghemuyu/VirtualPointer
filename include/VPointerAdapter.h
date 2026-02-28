/**
 * @file VPointerAdapter.h
 * @brief vPointer APP 适配器实现
 * 
 * 实现与vPointer APP的UDP通信协议
 * 协议格式：发送 "x,y,show,down,orientation"，接收单字节方向
 * 
 * @author Auto-generated for vPointer Integration
 * @version 1.0.0
 */

#ifndef VPOINTER_ADAPTER_H
#define VPOINTER_ADAPTER_H

#include "VirtualPointerInterface.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

namespace vptr {

// 错误代码定义（内部使用）
enum class VPtrErrorCode : int32_t {
    None = 0,
    InitFailed = 1,
    ConnectFailed = 2,
    SendFailed = 3,
    InvalidParam = 4,
    NotConnected = 5,
    SocketError = 6,
    Timeout = 7,
    Unknown = 99
};

/**
 * @brief vPointer UDP协议适配器
 * 
 * 通信协议详情：
 * - 端口：6533 (UDP)
 * - 发送格式："x,y,show,down,orientation" (5个整数，逗号分隔)
 * - 接收格式：单字节 (0x00-0x03 表示方向)
 * - 心跳：定期发送保持连接
 */
class VPTR_API VPointerAdapter : public IVirtualPointerAdapter {
public:
    VPointerAdapter();
    ~VPointerAdapter() override;

    // 禁止拷贝和赋值
    VPointerAdapter(const VPointerAdapter&) = delete;
    VPointerAdapter& operator=(const VPointerAdapter&) = delete;

    // IVirtualPointerAdapter 接口实现
    bool Initialize(const AdapterConfig& config) override;
    void Shutdown() override;
    bool Connect() override;
    void Disconnect() override;
    bool MovePointer(int32_t x, int32_t y) override;
    bool SetPointerPosition(const PointerPosition& position) override;
    bool ShowPointer() override;
    bool HidePointer() override;
    bool SetPointerPressed(bool pressed) override;
    PointerState GetPointerState() const override;
    PointerPosition GetPointerPosition() const override;
    ConnectionState GetConnectionState() const override;
    DeviceInfo GetDeviceInfo() const override;
    void SetOrientationCallback(OrientationCallback callback) override;
    void SetConnectionCallback(ConnectionCallback callback) override;
    void SetErrorCallback(ErrorCallback callback) override;
    bool SendRawData(const uint8_t* data, size_t length) override;
    const char* GetAdapterName() const override;
    const char* GetProtocolVersion() const override;
    bool IsDeviceOnline() const override;
    bool SendKeepAlive() override;
    std::string GetLastErrorMessage() const override;

    /**
     * @brief 设置发送端口（vPointer默认6533）
     * @param port 端口号
     */
    void SetTargetPort(uint16_t port);

    /**
     * @brief 获取最后错误信息
     * @return 错误信息
     */
    std::string GetLastError() const;

    /**
     * @brief 获取接收到的方向信息
     * @return 设备方向
     */
    DeviceOrientation GetReceivedOrientation() const;

    /**
     * @brief 设置调试模式
     * @param enable 是否启用
     */
    void SetDebugMode(bool enable);

private:
    // 内部方法
    bool InitializeWinsock();
    void CleanupWinsock();
    bool CreateSocket();
    void CloseSocket();
    bool SendPacket(const std::string& data);
    void StartReceiveThread();
    void StopReceiveThread();
    void ReceiveLoop();
    void UpdateConnectionState(ConnectionState state, const std::string& msg = "");
    void NotifyError(int32_t code, const std::string& msg);
    std::string BuildCommandString(int32_t x, int32_t y, int32_t show, int32_t down, int32_t orientation);
    void HandleReceivedOrientation(uint8_t orientationByte);

    // 配置
    AdapterConfig config_;
    
    // 状态
    std::atomic<ConnectionState> connectionState_;
    std::atomic<PointerState> pointerState_;
    std::atomic<DeviceOrientation> receivedOrientation_;
    PointerPosition currentPosition_;
    DeviceInfo deviceInfo_;
    std::atomic<bool> isPressed_;
    std::atomic<bool> isVisible_;
    
    // 网络
    SOCKET sendSocket_;
    SOCKET recvSocket_;
    sockaddr_in targetAddr_;
    sockaddr_in localAddr_;
    std::atomic<bool> recvRunning_;
    std::thread recvThread_;
    
    // 回调
    OrientationCallback orientationCallback_;
    ConnectionCallback connectionCallback_;
    ErrorCallback errorCallback_;
    
    // 同步
    mutable std::mutex stateMutex_;
    mutable std::mutex socketMutex_;
    
    // 其他
    std::atomic<bool> initialized_;
    std::atomic<bool> debugMode_;
    std::string lastError_;
    std::atomic<uint32_t> sequence_;
    std::chrono::steady_clock::time_point lastSendTime_;
};

/**
 * @brief 创建vPointer适配器实例
 * @return 适配器智能指针
 */
VPTR_API IVirtualPointerAdapterPtr CreateVPointerAdapter();

} // namespace vptr

#endif // VPOINTER_ADAPTER_H
