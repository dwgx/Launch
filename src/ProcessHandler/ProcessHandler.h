#ifndef PROCESSHANDLER_H
#define PROCESSHANDLER_H

#include <windows.h>
#include <string>

class ProcessHandler {
public:
    ProcessHandler(int pid);
    ~ProcessHandler();

    HANDLE OpenProcessHandle();
    uintptr_t GetModuleBaseAddress(const std::string& moduleName);

private:
    int pid;
    HANDLE hProcess;
};

#endif