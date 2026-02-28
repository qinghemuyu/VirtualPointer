@echo off
chcp 65001 >nul
echo ========================================
echo   VirtualPointer DLL 构建脚本
echo ========================================
echo.

set BUILD_DIR=build
set CONFIG=Release

if "%1"=="debug" (
    set CONFIG=Debug
)

echo 构建配置: %CONFIG%
echo.

if not exist %BUILD_DIR% (
    echo 创建构建目录...
    mkdir %BUILD_DIR%
)

cd %BUILD_DIR%

echo 生成项目文件...
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_EXAMPLES=ON
if errorlevel 1 (
    echo 错误: CMake生成失败
    cd ..
    exit /b 1
)

echo.
echo 编译项目...
cmake --build . --config %CONFIG%
if errorlevel 1 (
    echo 错误: 编译失败
    cd ..
    exit /b 1
)

cd ..

echo.
echo ========================================
echo   构建成功!
echo ========================================
echo 输出文件: bin\%CONFIG%\
echo.

pause
