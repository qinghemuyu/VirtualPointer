/**
 * @file VirtualPointerCAPI.cpp
 * @brief C语言接口实现
 */

#include <cstring>
#include <map>
#include <mutex>
#include "VirtualPointerCAPI.h"
#include "VPointerAdapter.h"

// 版本信息
#define VPTR_DLL_VERSION "1.0.0"

// 句柄管理
namespace {
    std::mutex g_handleMutex;
    std::map<VPTR_HANDLE, std::shared_ptr<vptr::IVirtualPointerAdapter>> g_adapters;
    uint64_t g_nextHandle = 1;

    // 回调包装结构
    struct CallbackData {
        VPTR_ORIENTATION_CALLBACK orientationCb;
        VPTR_CONNECTION_CALLBACK connectionCb;
        VPTR_ERROR_CALLBACK errorCb;
        void* userData;
    };
    std::map<VPTR_HANDLE, CallbackData> g_callbacks;

    VPTR_HANDLE RegisterAdapter(std::shared_ptr<vptr::IVirtualPointerAdapter> adapter) {
        std::lock_guard<std::mutex> lock(g_handleMutex);
        VPTR_HANDLE handle = reinterpret_cast<VPTR_HANDLE>(g_nextHandle++);
        g_adapters[handle] = adapter;
        g_callbacks[handle] = {nullptr, nullptr, nullptr, nullptr};
        return handle;
    }

    std::shared_ptr<vptr::IVirtualPointerAdapter> GetAdapter(VPTR_HANDLE handle) {
        std::lock_guard<std::mutex> lock(g_handleMutex);
        auto it = g_adapters.find(handle);
        if (it != g_adapters.end()) {
            return it->second;
        }
        return nullptr;
    }

    void UnregisterAdapter(VPTR_HANDLE handle) {
        std::lock_guard<std::mutex> lock(g_handleMutex);
        g_adapters.erase(handle);
        g_callbacks.erase(handle);
    }

    // C++回调到C回调的桥接函数
    void OrientationBridge(vptr::DeviceOrientation orientation, VPTR_HANDLE handle) {
        std::lock_guard<std::mutex> lock(g_handleMutex);
        auto it = g_callbacks.find(handle);
        if (it != g_callbacks.end() && it->second.orientationCb) {
            it->second.orientationCb(static_cast<VPTR_ORIENTATION>(orientation), 
                                     it->second.userData);
        }
    }

    void ConnectionBridge(vptr::ConnectionState state, const std::string& msg, 
                          VPTR_HANDLE handle) {
        std::lock_guard<std::mutex> lock(g_handleMutex);
        auto it = g_callbacks.find(handle);
        if (it != g_callbacks.end() && it->second.connectionCb) {
            it->second.connectionCb(static_cast<VPTR_CONNECTION_STATE>(state), 
                                    msg.c_str(), 
                                    it->second.userData);
        }
    }

    void ErrorBridge(int32_t code, const std::string& msg, VPTR_HANDLE handle) {
        std::lock_guard<std::mutex> lock(g_handleMutex);
        auto it = g_callbacks.find(handle);
        if (it != g_callbacks.end() && it->second.errorCb) {
            it->second.errorCb(code, msg.c_str(), it->second.userData);
        }
    }
}

extern "C" {

VPTR_C_API VPTR_HANDLE VPTR_CreateAdapter(const char* adapterType) {
    if (!adapterType) {
        return nullptr;
    }

    std::shared_ptr<vptr::IVirtualPointerAdapter> adapter;
    
    if (strcmp(adapterType, "vPointer") == 0 || strcmp(adapterType, "vpointer") == 0) {
        adapter = vptr::CreateVPointerAdapter();
    }
    // 可以在这里添加其他适配器类型
    // else if (strcmp(adapterType, "OtherApp") == 0) { ... }
    
    if (adapter) {
        return RegisterAdapter(adapter);
    }
    
    return nullptr;
}

VPTR_C_API void VPTR_DestroyAdapter(VPTR_HANDLE handle) {
    if (!handle) return;
    
    auto adapter = GetAdapter(handle);
    if (adapter) {
        adapter->Shutdown();
    }
    
    UnregisterAdapter(handle);
}

VPTR_C_API bool VPTR_Initialize(VPTR_HANDLE handle, const VPTR_CONFIG* config) {
    if (!handle || !config) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    vptr::AdapterConfig cfg;
    if (config->targetIp) {
        cfg.targetIp = config->targetIp;
    }
    cfg.targetPort = config->targetPort;
    cfg.localPort = config->localPort;
    cfg.connectionTimeout = config->connectionTimeout;
    cfg.keepAliveInterval = config->keepAliveInterval;
    cfg.autoReconnect = config->autoReconnect;
    cfg.maxReconnectAttempts = config->maxReconnectAttempts;

    return adapter->Initialize(cfg);
}

VPTR_C_API void VPTR_Shutdown(VPTR_HANDLE handle) {
    if (!handle) return;
    
    auto adapter = GetAdapter(handle);
    if (adapter) {
        adapter->Shutdown();
    }
}

VPTR_C_API bool VPTR_Connect(VPTR_HANDLE handle) {
    if (!handle) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    return adapter->Connect();
}

VPTR_C_API void VPTR_Disconnect(VPTR_HANDLE handle) {
    if (!handle) return;
    
    auto adapter = GetAdapter(handle);
    if (adapter) {
        adapter->Disconnect();
    }
}

VPTR_C_API bool VPTR_MovePointer(VPTR_HANDLE handle, int32_t x, int32_t y) {
    if (!handle) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    return adapter->MovePointer(x, y);
}

VPTR_C_API bool VPTR_SetPointerPosition(VPTR_HANDLE handle, const VPTR_POSITION* position) {
    if (!handle || !position) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    vptr::PointerPosition pos(position->x, position->y);
    return adapter->SetPointerPosition(pos);
}

VPTR_C_API bool VPTR_ShowPointer(VPTR_HANDLE handle) {
    if (!handle) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    return adapter->ShowPointer();
}

VPTR_C_API bool VPTR_HidePointer(VPTR_HANDLE handle) {
    if (!handle) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    return adapter->HidePointer();
}

VPTR_C_API bool VPTR_SetPointerPressed(VPTR_HANDLE handle, bool pressed) {
    if (!handle) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    return adapter->SetPointerPressed(pressed);
}

VPTR_C_API VPTR_POINTER_STATE VPTR_GetPointerState(VPTR_HANDLE handle) {
    if (!handle) return VPTR_STATE_HIDDEN;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return VPTR_STATE_HIDDEN;

    return static_cast<VPTR_POINTER_STATE>(adapter->GetPointerState());
}

VPTR_C_API bool VPTR_GetPointerPosition(VPTR_HANDLE handle, VPTR_POSITION* position) {
    if (!handle || !position) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    auto pos = adapter->GetPointerPosition();
    position->x = pos.x;
    position->y = pos.y;
    return true;
}

VPTR_C_API VPTR_CONNECTION_STATE VPTR_GetConnectionState(VPTR_HANDLE handle) {
    if (!handle) return VPTR_CONN_DISCONNECTED;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return VPTR_CONN_DISCONNECTED;

    return static_cast<VPTR_CONNECTION_STATE>(adapter->GetConnectionState());
}

VPTR_C_API bool VPTR_GetDeviceInfo(VPTR_HANDLE handle, VPTR_DEVICE_INFO* info) {
    if (!handle || !info) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    auto devInfo = adapter->GetDeviceInfo();
    
    strncpy_s(info->deviceId, sizeof(info->deviceId), 
              devInfo.deviceId.c_str(), _TRUNCATE);
    strncpy_s(info->deviceName, sizeof(info->deviceName), 
              devInfo.deviceName.c_str(), _TRUNCATE);
    strncpy_s(info->protocolVer, sizeof(info->protocolVer), 
              devInfo.protocolVer.c_str(), _TRUNCATE);
    
    info->screenWidth = devInfo.screenWidth;
    info->screenHeight = devInfo.screenHeight;
    info->orientation = static_cast<VPTR_ORIENTATION>(devInfo.orientation);
    
    return true;
}

VPTR_C_API bool VPTR_IsDeviceOnline(VPTR_HANDLE handle) {
    if (!handle) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    return adapter->IsDeviceOnline();
}

VPTR_C_API bool VPTR_SendKeepAlive(VPTR_HANDLE handle) {
    if (!handle) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    return adapter->SendKeepAlive();
}

VPTR_C_API void VPTR_SetOrientationCallback(VPTR_HANDLE handle, 
                                             VPTR_ORIENTATION_CALLBACK callback, 
                                             void* userData) {
    if (!handle) return;
    
    {
        std::lock_guard<std::mutex> lock(g_handleMutex);
        auto it = g_callbacks.find(handle);
        if (it != g_callbacks.end()) {
            it->second.orientationCb = callback;
            it->second.userData = userData;
        }
    }
    
    auto adapter = GetAdapter(handle);
    if (adapter) {
        if (callback) {
            adapter->SetOrientationCallback(
                [handle](vptr::DeviceOrientation ori) {
                    OrientationBridge(ori, handle);
                });
        } else {
            adapter->SetOrientationCallback(nullptr);
        }
    }
}

VPTR_C_API void VPTR_SetConnectionCallback(VPTR_HANDLE handle, 
                                            VPTR_CONNECTION_CALLBACK callback, 
                                            void* userData) {
    if (!handle) return;
    
    {
        std::lock_guard<std::mutex> lock(g_handleMutex);
        auto it = g_callbacks.find(handle);
        if (it != g_callbacks.end()) {
            it->second.connectionCb = callback;
            it->second.userData = userData;
        }
    }
    
    auto adapter = GetAdapter(handle);
    if (adapter) {
        if (callback) {
            adapter->SetConnectionCallback(
                [handle](vptr::ConnectionState state, const std::string& msg) {
                    ConnectionBridge(state, msg, handle);
                });
        } else {
            adapter->SetConnectionCallback(nullptr);
        }
    }
}

VPTR_C_API void VPTR_SetErrorCallback(VPTR_HANDLE handle, 
                                       VPTR_ERROR_CALLBACK callback, 
                                       void* userData) {
    if (!handle) return;
    
    {
        std::lock_guard<std::mutex> lock(g_handleMutex);
        auto it = g_callbacks.find(handle);
        if (it != g_callbacks.end()) {
            it->second.errorCb = callback;
            it->second.userData = userData;
        }
    }
    
    auto adapter = GetAdapter(handle);
    if (adapter) {
        if (callback) {
            adapter->SetErrorCallback(
                [handle](int32_t code, const std::string& msg) {
                    ErrorBridge(code, msg, handle);
                });
        } else {
            adapter->SetErrorCallback(nullptr);
        }
    }
}

VPTR_C_API bool VPTR_SendRawData(VPTR_HANDLE handle, const uint8_t* data, size_t length) {
    if (!handle || !data || length == 0) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    return adapter->SendRawData(data, length);
}

VPTR_C_API bool VPTR_GetAdapterName(VPTR_HANDLE handle, char* buffer, size_t bufferSize) {
    if (!handle || !buffer || bufferSize == 0) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    strncpy_s(buffer, bufferSize, adapter->GetAdapterName(), _TRUNCATE);
    return true;
}

VPTR_C_API bool VPTR_GetProtocolVersion(VPTR_HANDLE handle, char* buffer, size_t bufferSize) {
    if (!handle || !buffer || bufferSize == 0) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    strncpy_s(buffer, bufferSize, adapter->GetProtocolVersion(), _TRUNCATE);
    return true;
}

VPTR_C_API bool VPTR_GetLastError(VPTR_HANDLE handle, char* buffer, size_t bufferSize) {
    if (!handle || !buffer || bufferSize == 0) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    std::string error = adapter->GetLastErrorMessage();
    strncpy_s(buffer, bufferSize, error.c_str(), _TRUNCATE);
    return true;
}

// 便捷函数
VPTR_C_API VPTR_HANDLE VPTR_CreateAndConnect(const char* targetIp, uint16_t targetPort) {
    if (!targetIp) return nullptr;
    
    VPTR_HANDLE handle = VPTR_CreateAdapter("vPointer");
    if (!handle) return nullptr;

    VPTR_CONFIG config = {};
    config.targetIp = targetIp;
    config.targetPort = targetPort;
    config.localPort = 0;
    config.connectionTimeout = 5000;
    config.autoReconnect = true;

    if (!VPTR_Initialize(handle, &config)) {
        VPTR_DestroyAdapter(handle);
        return nullptr;
    }

    if (!VPTR_Connect(handle)) {
        VPTR_DestroyAdapter(handle);
        return nullptr;
    }

    return handle;
}

VPTR_C_API bool VPTR_UpdatePointer(VPTR_HANDLE handle, int32_t x, int32_t y, 
                                    bool visible, bool pressed) {
    if (!handle) return false;
    
    auto adapter = GetAdapter(handle);
    if (!adapter) return false;

    // 先设置位置
    if (!adapter->MovePointer(x, y)) {
        return false;
    }

    // 设置可见性
    if (visible) {
        if (!adapter->ShowPointer()) return false;
    } else {
        if (!adapter->HidePointer()) return false;
    }

    // 设置按下状态
    return adapter->SetPointerPressed(pressed);
}

VPTR_C_API void VPTR_GetVersion(char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize == 0) return;
    strncpy_s(buffer, bufferSize, VPTR_DLL_VERSION, _TRUNCATE);
}

VPTR_C_API int32_t VPTR_GetSupportedAdapterTypes(const char** types, int32_t maxCount) {
    if (!types || maxCount <= 0) return 0;
    
    int32_t count = 0;
    
    if (count < maxCount) {
        types[count++] = "vPointer";
    }
    
    // 可以在这里添加更多适配器类型
    
    return count;
}

} // extern "C"
