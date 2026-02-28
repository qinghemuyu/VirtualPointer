cmake_minimum_required(VERSION 3.16)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# C示例
add_executable(VPtrExample_C example_c.c)
target_link_libraries(VPtrExample_C VirtualPointerDLL)
set_target_properties(VPtrExample_C PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# C++示例
add_executable(VPtrExample_CPP example_cpp.cpp)
target_link_libraries(VPtrExample_CPP VirtualPointerDLL)
set_target_properties(VPtrExample_CPP PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 简单测试工具
add_executable(VPtrTestTool test_tool.cpp)
target_link_libraries(VPtrTestTool VirtualPointerDLL)
set_target_properties(VPtrTestTool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
