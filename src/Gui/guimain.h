#ifndef GUIMAIN_H
#define GUIMAIN_H

#include <string>
#include <vector>
#include "../MemoryManager/MemoryManager.h"

// 表格行结构（std::string 支持 UTF-8）
struct MemoryRow {
    std::string addressInput;
    int currentValue = 0;
    std::string valueInput;
    bool showModify = false;
};

// 注册主窗口
void RegisterMainWindow(MemoryManager*& memoryManager, bool& processAttached, std::string& statusMsg);

// 解析地址
uintptr_t ParseAddressWithPointers(const std::string& input, HANDLE hProcess, std::string& errorMsg, class ProcessHandler* processHandler);

#endif // GUIMAIN_H