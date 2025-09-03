#ifndef GUIMAIN_H
#define GUIMAIN_H

#include <string>
#include <vector>
#include "MemoryManager/MemoryManager.h"

// 表格行结构（支持指针链，使用 char 数组）
struct MemoryRow {
    char addressInput[64] = {0};  // 显式初始化为全零（空字符串）
    int currentValue = 0;
    char valueInput[32] = {0};    // 显式初始化为全零（空字符串）
    bool showModify = false;
};

// 注册主窗口
void RegisterMainWindow(MemoryManager*& memoryManager, bool& processAttached, std::string& statusMsg);

// 解析地址（支持多级指针链和 16 进制地址）
uintptr_t ParseAddressWithPointers(const std::string& input, HANDLE hProcess, std::string& errorMsg);

#endif // GUIMAIN_H