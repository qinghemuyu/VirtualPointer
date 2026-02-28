# VirtualPointer DLL

Windows平台虚拟指针控制DLL，用于对接Android vPointer APP，实现远程虚拟光标控制。

## 📢 重要：立即可用的版本

由于编译环境限制，DLL需要你自己编译。但为了方便使用，我们提供了**立即可用的替代方案**：

- **C#版本** (`python_version/vpointer_cs.cs`) - 直接添加到项目中使用


---

## 功能特性

- ✅ 完整的vPointer协议支持（UDP 6533端口）
- ✅ C++ 面向对象接口
- ✅ C 语言兼容接口
- ✅ Python/C# 实现（无需编译）
- ✅ 可扩展架构（易于添加新APP支持）
- ✅ 回调机制（方向变化、连接状态、错误处理）
- ✅ 连接状态管理
- ✅ 心跳保持
- ✅ 多设备支持

## 项目结构

```
vpointer_dll/
├── include/                    # 头文件
│   ├── VirtualPointerInterface.h   # C++抽象接口
│   ├── VPointerAdapter.h           # vPointer适配器
│   └── VirtualPointerCAPI.h        # C接口
├── src/                        # 源文件
│   ├── VPointerAdapter.cpp         # vPointer协议实现
│   ├── VirtualPointerCAPI.cpp      # C接口实现
│   └── dllmain.cpp                 # DLL入口
├── examples/                   # 示例程序
│   ├── example_c.c                 # C示例
│   ├── example_cpp.cpp             # C++示例
│   └── test_tool.cpp               # 测试工具
├── python_version/             # Python/C#实现（立即可用）
│   ├── vpointer.py                 # Python版本
│   ├── vpointer_cs.cs              # C#版本
│   └── README.md                   # 使用说明
├── docs/                       # 文档
│   └── Integration_Guide.md        # 对接文档
├── CMakeLists.txt              # CMake配置
├── VirtualPointer.sln          # Visual Studio解决方案
└── VirtualPointer.vcxproj      # Visual Studio项目
```

### 方式1：使用C#版本

将 `vpointer_cs.cs` 添加到你的项目中，直接使用：

```csharp
using VPointer;

var adapter = VPointerHelper.CreateAndConnect("192.168.1.100", 6533);
adapter.ShowPointer();
adapter.MovePointer(100, 200);
adapter.HidePointer();
adapter.Shutdown();
```

### 方式2：编译C++ DLL

#### 使用Visual Studio

1. 打开 `VirtualPointer.sln`
2. 选择 Release/x64 配置
3. 生成解决方案

#### 使用CMake

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### 使用build.bat

```bash
build.bat        # Release版本
build.bat debug  # Debug版本
```

### 使用编译后的DLL

```cpp
#include "VirtualPointerCAPI.h"

VPTR_HANDLE handle = VPTR_CreateAndConnect("192.168.1.100", 6533);
VPTR_ShowPointer(handle);
VPTR_MovePointer(handle, 100, 200);
VPTR_HidePointer(handle);
VPTR_DestroyAdapter(handle);
```

## vPointer 通信协议

### 发送格式（Windows → Android）

```
x,y,show,down,orientation
```

- `x`, `y`: 光标坐标（像素）
- `show`: 1=显示, 0=隐藏
- `down`: 1=按下, 0=正常
- `orientation`: 方向（0-3）

**示例：**
```
"100,200,1,0,0"   - 在(100,200)显示光标
"300,400,1,1,0"   - 在(300,400)显示光标，按下状态
```

### 接收格式（Android → Windows）

单字节方向信息：
- 0x00: 竖屏 (0°)
- 0x01: 横屏右 (90°)
- 0x02: 竖屏倒置 (180°)
- 0x03: 横屏左 (270°)

## 架构设计

```
┌─────────────────────────────────────┐
│         应用程序 (Your App)          │
├─────────────────────────────────────┤
│  C++接口  │  C接口  │ Python │ C#   │
├─────────────────────────────────────┤
│      VirtualPointerDLL              │
├─────────────────────────────────────┤
│  IVirtualPointerAdapter (抽象接口)   │
│         │                           │
│    VPointerAdapter (具体实现)        │
├─────────────────────────────────────┤
│         UDP通信层                    │
└─────────────────────────────────────┘
```

## 扩展开发

如需支持其他虚拟指针APP，只需实现`IVirtualPointerAdapter`接口：

```cpp
class MyAppAdapter : public IVirtualPointerAdapter {
    // 实现接口方法...
};
```

## 文档

- [对接文档](docs/Integration_Guide.md) - 详细的API文档和使用说明

## 依赖

### C++ DLL编译
- Windows SDK
- Visual Studio 2019+ 或 MinGW-w64
- CMake 3.16+

### C#版本
- .NET Framework 4.5+ 或 .NET Core 2.0+

## 相关项目

- [vPointer](https://github.com/RiderLty/vPointer) - Android虚拟光标APP

## 许可证

MIT License
