#include "MemoryManager.h"
#include <iostream>

MemoryManager::MemoryManager(HANDLE hProcess) : hProcess(hProcess) {}

MemoryManager::~MemoryManager() {}

template <typename T>
bool MemoryManager::ReadMemory(LPCVOID address, T& buffer) {
    SIZE_T bytesRead;
    return ReadProcessMemory(hProcess, address, &buffer, sizeof(T), &bytesRead) && bytesRead == sizeof(T);
}

template <typename T>
bool MemoryManager::WriteMemory(LPVOID address, const T& value) {
    SIZE_T bytesWritten;
    return WriteProcessMemory(hProcess, address, &value, sizeof(T), &bytesWritten) && bytesWritten == sizeof(T);
}

// 实例化
template bool MemoryManager::ReadMemory<int>(LPCVOID, int&);
template bool MemoryManager::WriteMemory<int>(LPVOID, const int&);