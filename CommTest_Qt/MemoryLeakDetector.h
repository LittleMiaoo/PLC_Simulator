#ifndef MEMORY_LEAK_DETECTOR_H
#define MEMORY_LEAK_DETECTOR_H

#ifdef _WIN32
#ifdef _DEBUG

// 包含Windows和CRT的调试头文件
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// 定义内存泄漏检测宏
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW

// 内存泄漏检测类
class MemoryLeakDetector {
public:
    // 初始化内存泄漏检测
    static void EnableMemoryLeakChecks() {
        // 获取当前的内存状态
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        
        // 设置在程序退出时报告内存泄漏
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
        
        // 输出详细的内存泄漏报告
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    }
    
    // 在特定点检查内存泄漏
    static void CheckForLeaksAtPoint() {
        _CrtDumpMemoryLeaks();
    }
    
    // 设置断点在特定的内存分配编号上（用于调试）
    static void SetBreakAlloc(long allocNumber) {
        _CrtSetBreakAlloc(allocNumber);
    }
    
    // 获取当前内存状态快照
    static _CrtMemState* CreateMemorySnapshot() {
        _CrtMemState* state = new _CrtMemState;
        _CrtMemCheckpoint(state);
        return state;
    }
    
    // 比较两个内存状态，找出差异（泄漏）
    static void CompareMemorySnapshots(_CrtMemState* oldState, _CrtMemState* newState) {
        _CrtMemState diff;
        if (_CrtMemDifference(&diff, oldState, newState)) {
            _CrtMemDumpStatistics(&diff);
        }
    }
    
    // 释放内存快照
    static void DeleteMemorySnapshot(_CrtMemState* state) {
        delete state;
    }
};

#endif // _DEBUG
#endif // _WIN32

#endif // MEMORY_LEAK_DETECTOR_H