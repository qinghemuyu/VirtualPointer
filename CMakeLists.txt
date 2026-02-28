cmake_minimum_required(VERSION 3.16)
project(VirtualPointerDLL VERSION 1.0.0 LANGUAGES CXX C)

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义导出宏
add_definitions(-DVPTR_DLL_EXPORTS -D_WINSOCK_DEPRECATED_NO_WARNINGS)

# 源文件
set(SOURCES
    src/dllmain.cpp
    src/VPointerAdapter.cpp
    src/VirtualPointerCAPI.cpp
)

# 头文件
set(HEADERS
    include/VirtualPointerInterface.h
    include/VPointerAdapter.h
    include/VirtualPointerCAPI.h
)

# 创建DLL
add_library(VirtualPointerDLL SHARED ${SOURCES} ${HEADERS})

# 包含目录
target_include_directories(VirtualPointerDLL PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 链接Winsock库
target_link_libraries(VirtualPointerDLL ws2_32)

# 设置输出名称
set_target_properties(VirtualPointerDLL PROPERTIES
    OUTPUT_NAME "VirtualPointer"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# MSVC特定设置
if(MSVC)
    target_compile_options(VirtualPointerDLL PRIVATE /W4 /WX- /utf-8)
    set_property(TARGET VirtualPointerDLL PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# 安装规则
install(TARGETS VirtualPointerDLL
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS} DESTINATION include)

# 示例程序选项
option(BUILD_EXAMPLES "Build example programs" ON)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 打印配置信息
message(STATUS "")
message(STATUS "VirtualPointerDLL Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "")
