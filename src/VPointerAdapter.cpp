/**
 * @file VPointerAdapter.cpp
 * @brief vPointer APP 适配器实现
 */

#include "VPointerAdapter.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <iostream>

namespace vptr {

// 协议常量
constexpr uint16_t VPTR_DEFAULT_PORT = 6533;
constexpr size_t RECV_BUFFER_SIZE = 1024;
constexpr int SOCKET_TIMEOUT_MS = 1000;

VPointerAdapter::VPointerAdapter()
    : connectionState_(ConnectionState::Disconnected)
    , pointerState_(PointerState::Hidden)
    , receivedOrientation_(DeviceOrientation::Portrait)
    , isPressed_(false)
    , isVisible_(false)
    , sendSocket_(INVALID_SOCKET)
    , recvSocket_(INVALID_SOCKET)
    , recvRunning_(false)
    , initialized_(false)
    , debugMode_(false)
    , sequence_(0) {
    memset(&targetAddr_, 0, sizeof(targetAddr_));
    memset(&localAddr_, 0, sizeof(localAddr_));
    deviceInfo_.deviceId = "vPointer";
    deviceInfo_.deviceName = "vPointer Virtual Cursor";
    deviceInfo_.protocolVer = "1.0";
}

VPointerAdapter::~VPointerAdapter() {
    Shutdown();
}

bool VPointerAdapter::Initialize(const AdapterConfig& config) {
    if (initialized_) {
        return true;
    }

    config_ = config;
    
    // 设置默认端口
    if (config_.targetPort == 0) {
        config_.targetPort = VPTR_DEFAULT_PORT;
    }

    // 初始化Winsock
    if (!InitializeWinsock()) {
        lastError_ = "Failed to initialize Winsock";
        NotifyError(static_cast<int32_t>(VPtrErrorCode::InitFailed), lastError_);
        return false;
    }

    // 创建发送套接字
    if (!CreateSocket()) {
        CleanupWinsock();
        return false;
    }

    // 设置目标地址
    targetAddr_.sin_family = AF_INET;
    targetAddr_.sin_port = htons(config_.targetPort);
    inet_pton(AF_INET, config_.targetIp.c_str(), &targetAddr_.sin_addr);

    initialized_ = true;
    
    if (debugMode_) {
        std::cout << "[VPointerAdapter] Initialized, target: " 
                  << config_.targetIp << ":" << config_.targetPort << std::endl;
    }

    return true;
}

void VPointerAdapter::Shutdown() {
    Disconnect();
    CloseSocket();
    CleanupWinsock();
    initialized_ = false;
}

bool VPointerAdapter::Connect() {
    if (!initialized_) {
        lastError_ = "Adapter not initialized";
        NotifyError(static_cast<int32_t>(VPtrErrorCode::NotConnected), lastError_);
        return false;
    }

    if (connectionState_ == ConnectionState::Connected) {
        return true;
    }

    UpdateConnectionState(ConnectionState::Connecting, "Connecting to device...");

    // 发送初始数据包建立连接
    std::string initCmd = BuildCommandString(0, 0, 0, 0, 0);
    if (!SendPacket(initCmd)) {
        lastError_ = "Failed to send initial packet";
        UpdateConnectionState(ConnectionState::Error, lastError_);
        return false;
    }

    // 启动接收线程
    StartReceiveThread();

    UpdateConnectionState(ConnectionState::Connected, "Connected successfully");
    
    if (debugMode_) {
        std::cout << "[VPointerAdapter] Connected to " << config_.targetIp << std::endl;
    }

    return true;
}

void VPointerAdapter::Disconnect() {
    StopReceiveThread();
    
    // 发送隐藏指针命令
    if (connectionState_ == ConnectionState::Connected) {
        HidePointer();
    }

    UpdateConnectionState(ConnectionState::Disconnected, "Disconnected");
    
    if (debugMode_) {
        std::cout << "[VPointerAdapter] Disconnected" << std::endl;
    }
}

bool VPointerAdapter::MovePointer(int32_t x, int32_t y) {
    if (!IsDeviceOnline()) {
        lastError_ = "Device not connected";
        return false;
    }

    currentPosition_.x = x;
    currentPosition_.y = y;

    int32_t show = isVisible_ ? 1 : 0;
    int32_t down = isPressed_ ? 1 : 0;
    int32_t orientation = static_cast<int32_t>(receivedOrientation_.load());

    std::string cmd = BuildCommandString(x, y, show, down, orientation);
    
    if (!SendPacket(cmd)) {
        return false;
    }

    sequence_++;
    lastSendTime_ = std::chrono::steady_clock::now();
    
    return true;
}

bool VPointerAdapter::SetPointerPosition(const PointerPosition& position) {
    return MovePointer(position.x, position.y);
}

bool VPointerAdapter::ShowPointer() {
    if (!IsDeviceOnline()) {
        lastError_ = "Device not connected";
        return false;
    }

    isVisible_ = true;
    pointerState_ = isPressed_ ? PointerState::Pressed : PointerState::Visible;

    int32_t down = isPressed_ ? 1 : 0;
    int32_t orientation = static_cast<int32_t>(receivedOrientation_.load());
    
    std::string cmd = BuildCommandString(
        currentPosition_.x, 
        currentPosition_.y, 
        1,  // show = 1
        down, 
        orientation
    );
    
    return SendPacket(cmd);
}

bool VPointerAdapter::HidePointer() {
    if (!IsDeviceOnline()) {
        lastError_ = "Device not connected";
        return false;
    }

    isVisible_ = false;
    pointerState_ = PointerState::Hidden;

    int32_t orientation = static_cast<int32_t>(receivedOrientation_.load());
    std::string cmd = BuildCommandString(
        currentPosition_.x, 
        currentPosition_.y, 
        0,  // show = 0
        0, 
        orientation
    );
    
    return SendPacket(cmd);
}

bool VPointerAdapter::SetPointerPressed(bool pressed) {
    if (!IsDeviceOnline()) {
        lastError_ = "Device not connected";
        return false;
    }

    isPressed_ = pressed;
    pointerState_ = pressed ? PointerState::Pressed : 
                    (isVisible_ ? PointerState::Visible : PointerState::Hidden);

    int32_t show = isVisible_ ? 1 : 0;
    int32_t down = pressed ? 1 : 0;
    int32_t orientation = static_cast<int32_t>(receivedOrientation_.load());
    
    std::string cmd = BuildCommandString(
        currentPosition_.x, 
        currentPosition_.y, 
        show, 
        down, 
        orientation
    );
    
    return SendPacket(cmd);
}

PointerState VPointerAdapter::GetPointerState() const {
    return pointerState_;
}

PointerPosition VPointerAdapter::GetPointerPosition() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return currentPosition_;
}

ConnectionState VPointerAdapter::GetConnectionState() const {
    return connectionState_;
}

DeviceInfo VPointerAdapter::GetDeviceInfo() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return deviceInfo_;
}

void VPointerAdapter::SetOrientationCallback(OrientationCallback callback) {
    orientationCallback_ = callback;
}

void VPointerAdapter::SetConnectionCallback(ConnectionCallback callback) {
    connectionCallback_ = callback;
}

void VPointerAdapter::SetErrorCallback(ErrorCallback callback) {
    errorCallback_ = callback;
}

bool VPointerAdapter::SendRawData(const uint8_t* data, size_t length) {
    if (!IsDeviceOnline() || sendSocket_ == INVALID_SOCKET) {
        lastError_ = "Socket not available";
        return false;
    }

    std::lock_guard<std::mutex> lock(socketMutex_);
    int result = sendto(sendSocket_, 
                        reinterpret_cast<const char*>(data), 
                        static_cast<int>(length), 
                        0, 
                        reinterpret_cast<sockaddr*>(&targetAddr_), 
                        sizeof(targetAddr_));
    
    return result != SOCKET_ERROR;
}

const char* VPointerAdapter::GetAdapterName() const {
    return "vPointer";
}

const char* VPointerAdapter::GetProtocolVersion() const {
    return "1.0.0";
}

bool VPointerAdapter::IsDeviceOnline() const {
    return initialized_ && connectionState_ == ConnectionState::Connected;
}

bool VPointerAdapter::SendKeepAlive() {
    if (!IsDeviceOnline()) {
        return false;
    }

    // 发送当前状态作为心跳
    int32_t show = isVisible_ ? 1 : 0;
    int32_t down = isPressed_ ? 1 : 0;
    int32_t orientation = static_cast<int32_t>(receivedOrientation_.load());
    
    std::string cmd = BuildCommandString(
        currentPosition_.x, 
        currentPosition_.y, 
        show, 
        down, 
        orientation
    );
    
    return SendPacket(cmd);
}

void VPointerAdapter::SetTargetPort(uint16_t port) {
    config_.targetPort = port;
    targetAddr_.sin_port = htons(port);
}

std::string VPointerAdapter::GetLastError() const {
    return lastError_;
}

std::string VPointerAdapter::GetLastErrorMessage() const {
    return lastError_;
}

DeviceOrientation VPointerAdapter::GetReceivedOrientation() const {
    return receivedOrientation_;
}

void VPointerAdapter::SetDebugMode(bool enable) {
    debugMode_ = enable;
}

// 私有方法实现

bool VPointerAdapter::InitializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    return result == 0;
}

void VPointerAdapter::CleanupWinsock() {
    WSACleanup();
}

bool VPointerAdapter::CreateSocket() {
    sendSocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sendSocket_ == INVALID_SOCKET) {
        lastError_ = "Failed to create socket";
        return false;
    }

    // 设置发送超时
    int timeout = SOCKET_TIMEOUT_MS;
    setsockopt(sendSocket_, SOL_SOCKET, SO_SNDTIMEO, 
               reinterpret_cast<const char*>(&timeout), sizeof(timeout));

    // 如果需要接收回调，创建接收套接字
    if (config_.localPort > 0) {
        recvSocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (recvSocket_ == INVALID_SOCKET) {
            closesocket(sendSocket_);
            sendSocket_ = INVALID_SOCKET;
            lastError_ = "Failed to create receive socket";
            return false;
        }

        localAddr_.sin_family = AF_INET;
        localAddr_.sin_port = htons(config_.localPort);
        localAddr_.sin_addr.s_addr = INADDR_ANY;

        if (bind(recvSocket_, reinterpret_cast<sockaddr*>(&localAddr_), 
                 sizeof(localAddr_)) == SOCKET_ERROR) {
            closesocket(recvSocket_);
            closesocket(sendSocket_);
            recvSocket_ = INVALID_SOCKET;
            sendSocket_ = INVALID_SOCKET;
            lastError_ = "Failed to bind receive socket";
            return false;
        }

        // 设置接收超时
        setsockopt(recvSocket_, SOL_SOCKET, SO_RCVTIMEO, 
                   reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    }

    return true;
}

void VPointerAdapter::CloseSocket() {
    if (sendSocket_ != INVALID_SOCKET) {
        closesocket(sendSocket_);
        sendSocket_ = INVALID_SOCKET;
    }
    if (recvSocket_ != INVALID_SOCKET) {
        closesocket(recvSocket_);
        recvSocket_ = INVALID_SOCKET;
    }
}

bool VPointerAdapter::SendPacket(const std::string& data) {
    if (sendSocket_ == INVALID_SOCKET) {
        lastError_ = "Socket not created";
        return false;
    }

    std::lock_guard<std::mutex> lock(socketMutex_);
    int result = sendto(sendSocket_, 
                        data.c_str(), 
                        static_cast<int>(data.length()), 
                        0, 
                        reinterpret_cast<sockaddr*>(&targetAddr_), 
                        sizeof(targetAddr_));
    
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        lastError_ = "Send failed, error: " + std::to_string(error);
        
        if (debugMode_) {
            std::cerr << "[VPointerAdapter] " << lastError_ << std::endl;
        }
        
        return false;
    }

    if (debugMode_) {
        std::cout << "[VPointerAdapter] Sent: " << data << std::endl;
    }

    return true;
}

void VPointerAdapter::StartReceiveThread() {
    if (recvRunning_ || recvSocket_ == INVALID_SOCKET) {
        return;
    }

    recvRunning_ = true;
    recvThread_ = std::thread(&VPointerAdapter::ReceiveLoop, this);
}

void VPointerAdapter::StopReceiveThread() {
    recvRunning_ = false;
    if (recvThread_.joinable()) {
        recvThread_.join();
    }
}

void VPointerAdapter::ReceiveLoop() {
    char buffer[RECV_BUFFER_SIZE];
    sockaddr_in fromAddr;
    int fromLen = sizeof(fromAddr);

    while (recvRunning_) {
        int received = recvfrom(recvSocket_, buffer, RECV_BUFFER_SIZE - 1, 0,
                               reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);
        
        if (received > 0) {
            buffer[received] = '\0';
            
            if (debugMode_) {
                std::cout << "[VPointerAdapter] Received " << received 
                          << " bytes from " << inet_ntoa(fromAddr.sin_addr) << std::endl;
            }

            // vPointer协议：接收单字节方向信息
            if (received == 1) {
                uint8_t orientationByte = static_cast<uint8_t>(buffer[0]);
                HandleReceivedOrientation(orientationByte);
            }
        }
    }
}

void VPointerAdapter::HandleReceivedOrientation(uint8_t orientationByte) {
    DeviceOrientation newOrientation = DeviceOrientation::Portrait;
    
    switch (orientationByte) {
        case 0x00: newOrientation = DeviceOrientation::Portrait; break;
        case 0x01: newOrientation = DeviceOrientation::LandscapeRight; break;
        case 0x02: newOrientation = DeviceOrientation::PortraitUpsideDown; break;
        case 0x03: newOrientation = DeviceOrientation::LandscapeLeft; break;
        default: 
            if (debugMode_) {
                std::cerr << "[VPointerAdapter] Unknown orientation: " 
                          << static_cast<int>(orientationByte) << std::endl;
            }
            return;
    }

    receivedOrientation_ = newOrientation;
    deviceInfo_.orientation = newOrientation;

    if (debugMode_) {
        std::cout << "[VPointerAdapter] Orientation changed to " 
                  << static_cast<int>(orientationByte) << std::endl;
    }

    if (orientationCallback_) {
        orientationCallback_(newOrientation);
    }
}

void VPointerAdapter::UpdateConnectionState(ConnectionState state, const std::string& msg) {
    connectionState_ = state;
    
    if (connectionCallback_) {
        connectionCallback_(state, msg);
    }
}

void VPointerAdapter::NotifyError(int32_t code, const std::string& msg) {
    if (errorCallback_) {
        errorCallback_(code, msg);
    }
}

std::string VPointerAdapter::BuildCommandString(int32_t x, int32_t y, 
                                                 int32_t show, int32_t down, 
                                                 int32_t orientation) {
    std::ostringstream oss;
    oss << x << "," << y << "," << show << "," << down << "," << orientation;
    return oss.str();
}

// 工厂函数
IVirtualPointerAdapterPtr CreateVPointerAdapter() {
    return std::make_shared<VPointerAdapter>();
}

} // namespace vptr
