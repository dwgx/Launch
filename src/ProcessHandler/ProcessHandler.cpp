#include "ProcessHandler.h"
#include <tlhelp32.h>

ProcessHandler::ProcessHandler(int pid) : pid(pid), hProcess(NULL) {}

ProcessHandler::~ProcessHandler() {
    if (hProcess) {
        CloseHandle(hProcess);
    }
}

HANDLE ProcessHandler::OpenProcessHandle() {
    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
    return hProcess;
}

uintptr_t ProcessHandler::GetModuleBaseAddress(const std::string& moduleName) {
    MODULEENTRY32 modEntry;
    modEntry.dwSize = sizeof(modEntry);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Module32First(hSnapshot, &modEntry)) {
        do {
            if (moduleName == modEntry.szModule) {
                CloseHandle(hSnapshot);
                return (uintptr_t)modEntry.modBaseAddr;
            }
        } while (Module32Next(hSnapshot, &modEntry));
    }
    CloseHandle(hSnapshot);

    return 0;
}