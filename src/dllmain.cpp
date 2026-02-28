/**
 * @file dllmain.cpp
 * @brief DLL入口点
 */

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            // DLL被加载到进程
            DisableThreadLibraryCalls(hModule);
            break;
            
        case DLL_THREAD_ATTACH:
            // 线程创建
            break;
            
        case DLL_THREAD_DETACH:
            // 线程结束
            break;
            
        case DLL_PROCESS_DETACH:
            // DLL从进程卸载
            break;
    }
    
    return TRUE;
}
