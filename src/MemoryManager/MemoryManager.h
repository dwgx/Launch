#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <windows.h>

class MemoryManager {
public:
    MemoryManager(HANDLE hProcess);
    ~MemoryManager();

    template <typename T>
    bool ReadMemory(LPCVOID address, T& buffer);

    template <typename T>
    bool WriteMemory(LPVOID address, const T& value);

private:
    HANDLE hProcess;
};

#endif