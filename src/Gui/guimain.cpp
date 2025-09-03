#include "guimain.h"
#include <algorithm>
#include <sstream>
#include <tlhelp32.h>
#include <iomanip>  // 用于 hex 输出
#include "imgui.h"
#include <iostream>

// 全局状态（表格行）
static std::vector<MemoryRow> memoryRows;
static char pidInput[16] = {0};  // 显式初始化
static HANDLE hProcess = nullptr;

// 去除空格
std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

// 规范化 16 进制字符串（添加 0x 前缀并去除前导零）
std::string NormalizeHex(const std::string& input) {
    std::string str = Trim(input);
    if (str.empty()) return "";

    // 如果已包含 0x，则直接返回
    if (str.substr(0, 2) == "0x" || str.substr(0, 2) == "0X") {
        return str;
    }
    // 尝试解析为 16 进制
    try {
        std::stringstream ss;
        uintptr_t value = std::stoul(str, nullptr, 16);
        ss << std::hex << "0x" << value;
        return ss.str();
    } catch (...) {
        return str;  // 无效时保持原样
    }
}

// 解析地址，支持多种格式（绝对地址、模块+偏移、指针链）
uintptr_t ParseAddressWithPointers(const std::string& input, HANDLE hProcess, std::string& errorMsg) {
    if (!hProcess) {
        errorMsg = "进程句柄无效";
        std::cerr << errorMsg << std::endl;
        return 0;
    }

    std::string strInput = Trim(input);
    if (strInput.empty()) {
        errorMsg = "输入地址为空";
        return 0;
    }

    std::vector<std::string> parts;
    std::stringstream ss(strInput);
    std::string part;
    // 先按 '>' 分割指针链
    while (std::getline(ss, part, '>')) {
        parts.push_back(Trim(part));
    }

    // 处理第一部分：基地址或模块+偏移
    std::string basePart = parts[0];
    size_t plusPos = basePart.find("+");
    uintptr_t currentAddr = 0;

    // 情况 1：纯 16 进制地址（如 0x605FFDA00 或 605FFDA00）
    if (plusPos == std::string::npos) {
        try {
            std::string normalized = NormalizeHex(basePart);
            if (normalized.substr(0, 2) == "0x" || normalized.substr(0, 2) == "0X") {
                currentAddr = std::stoul(normalized.substr(2), nullptr, 16);  // 去除 0x
            } else {
                currentAddr = std::stoul(normalized, nullptr, 16);  // 尝试直接解析
            }
            std::cout << "解析纯 16 进制地址: 0x" << std::hex << currentAddr << std::endl;
        } catch (const std::exception& e) {
            errorMsg = "无效地址格式: " + basePart + " (" + e.what() + ")";
            std::cerr << errorMsg << std::endl;
            return 0;
        }
    } else {
        // 情况 2：模块+偏移（如 module+0xB0C44）
        std::string moduleName = Trim(basePart.substr(0, plusPos));
        std::string offsetStr = Trim(basePart.substr(plusPos + 1));
        uintptr_t offset;
        try {
            offset = std::stoul(NormalizeHex(offsetStr).substr(2), nullptr, 16);
        } catch (const std::exception& e) {
            errorMsg = "无效基偏移: " + offsetStr + " (" + e.what() + ")";
            std::cerr << errorMsg << std::endl;
            return 0;
        }
        MODULEENTRY32 modEntry = { sizeof(modEntry) };
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(hProcess));
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            errorMsg = "模块快照失败: " + std::to_string(GetLastError());
            std::cerr << errorMsg << std::endl;
            return 0;
        }
        if (Module32First(hSnapshot, &modEntry)) {
            do {
                if (moduleName == modEntry.szModule) {
                    currentAddr = (uintptr_t)modEntry.modBaseAddr + offset;
                    std::cout << "解析模块+偏移: 0x" << std::hex << currentAddr << " (模块: " << moduleName << ")" << std::endl;
                    break;
                }
            } while (Module32Next(hSnapshot, &modEntry));
        }
        CloseHandle(hSnapshot);
        if (currentAddr == 0) {
            errorMsg = "模块未找到: " + moduleName;
            std::cerr << errorMsg << std::endl;
            return 0;
        }
    }

    // 处理后续指针偏移
    for (size_t i = 1; i < parts.size(); ++i) {
        std::string offsetStr = parts[i];
        uintptr_t offset;
        try {
            offset = std::stoul(NormalizeHex(offsetStr).substr(2), nullptr, 16);
        } catch (const std::exception& e) {
            errorMsg = "无效指针偏移: " + offsetStr + " (" + e.what() + ")";
            std::cerr << errorMsg << std::endl;
            return 0;
        }
        uintptr_t nextAddr;
        SIZE_T bytesRead;
        if (!ReadProcessMemory(hProcess, (LPCVOID)currentAddr, &nextAddr, sizeof(nextAddr), &bytesRead)) {
            errorMsg = "指针读取失败: " + std::to_string(GetLastError()) + " (读取 " + std::to_string(bytesRead) + " 字节)";
            std::cerr << errorMsg << std::endl;
            return 0;
        }
        currentAddr = nextAddr + offset;
        std::cout << "解析指针偏移 " << i << ": 0x" << std::hex << currentAddr << std::endl;
    }

    return currentAddr;
}

// 将 uintptr_t 转换为 16 进制字符串
std::string ToHexString(uintptr_t value) {
    std::stringstream ss;
    ss << "0x" << std::hex << value;
    return ss.str();
}

void RegisterMainWindow(MemoryManager*& memoryManager, bool& processAttached, std::string& statusMsg) {
    ImGui::Begin("内存编辑器 - 主界面");

    ImGui::Text("请输入目标进程 PID");
    ImGui::InputText("PID", pidInput, sizeof(pidInput), ImGuiInputTextFlags_CharsDecimal);
    if (ImGui::Button("附加进程")) {
        int pid = std::atoi(pidInput);
        if (pid > 0) {
            if (hProcess) {
                CloseHandle(hProcess);
                hProcess = nullptr;
            }
            if (memoryManager) {
                delete memoryManager;
                memoryManager = nullptr;
            }
            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);  // 使用 PROCESS_ALL_ACCESS 确保权限
            if (hProcess) {
                memoryManager = new MemoryManager(hProcess);
                processAttached = true;
                statusMsg = "附加成功，PID: " + std::to_string(pid);
                std::cout << "成功附加进程，PID: " << pid << ", 句柄: 0x" << std::hex << (uintptr_t)hProcess << std::endl;
            } else {
                processAttached = false;
                statusMsg = "附加失败，错误码: " + std::to_string(GetLastError());
                std::cerr << statusMsg << std::endl;
            }
        } else {
            statusMsg = "无效 PID！";
        }
    }

    if (processAttached && memoryManager && hProcess) {
        // 智能添加新行
        static char newAddressInput[64] = {0};  // 临时输入框
        ImGui::InputText("新地址 (如 605FFDA00 或 module+offset)", newAddressInput, sizeof(newAddressInput));
        if (ImGui::Button("添加新行")) {
            if (strlen(newAddressInput) > 0) {
                MemoryRow row = {};
                strncpy(row.addressInput, newAddressInput, sizeof(row.addressInput) - 1);
                row.addressInput[sizeof(row.addressInput) - 1] = 0;  // 确保以 \0 结尾
                memoryRows.push_back(row);
                newAddressInput[0] = 0;  // 清空输入框
                statusMsg = "添加新行成功，地址: " + std::string(row.addressInput);
            } else {
                statusMsg = "地址不能为空！";
            }
        }

        // 表格
        if (ImGui::BeginTable("内存表格", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("地址 (支持 16 进制或指针链)");
            ImGui::TableSetupColumn("当前值");
            ImGui::TableSetupColumn("新值");
            ImGui::TableSetupColumn("操作");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < memoryRows.size(); ++i) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::InputText(("##addr" + std::to_string(i)).c_str(), memoryRows[i].addressInput, sizeof(memoryRows[i].addressInput));

                ImGui::TableSetColumnIndex(3);
                if (ImGui::Button(("读取##" + std::to_string(i)).c_str())) {
                    std::string errorMsg;
                    std::string addressStr = memoryRows[i].addressInput;
                    if (addressStr.empty()) {
                        statusMsg = "地址为空，行 " + std::to_string(i);
                        continue;
                    }
                    uintptr_t address = ParseAddressWithPointers(addressStr, hProcess, errorMsg);
                    if (address) {
                        SIZE_T bytesRead;
                        if (memoryManager->ReadMemory<int>((LPCVOID)address, memoryRows[i].currentValue)) {
                            statusMsg = "读取成功，行 " + std::to_string(i) + " 值: " + std::to_string(memoryRows[i].currentValue);
                            memoryRows[i].showModify = true;
                            std::cout << "读取成功，地址: " << ToHexString(address) << ", 值: " << memoryRows[i].currentValue << ", 字节读取: " << bytesRead << std::endl;
                        } else {
                            DWORD error = GetLastError();
                            statusMsg = "读取失败，错误码: " + std::to_string(error) + " (地址: " + ToHexString(address) + ")";
                            std::cerr << statusMsg << std::endl;
                            char errorMsgBuf[256];
                            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, 0, errorMsgBuf, sizeof(errorMsgBuf), nullptr);
                            std::cerr << "错误详情: " << errorMsgBuf << std::endl;
                        }
                    } else {
                        statusMsg = "解析地址失败: " + errorMsg;
                        std::cerr << statusMsg << std::endl;
                    }
                }

                if (memoryRows[i].showModify) {
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", memoryRows[i].currentValue);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::InputText(("##val" + std::to_string(i)).c_str(), memoryRows[i].valueInput, sizeof(memoryRows[i].valueInput), ImGuiInputTextFlags_CharsDecimal);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::SameLine();
                    if (ImGui::Button(("写入##" + std::to_string(i)).c_str())) {
                        std::stringstream ss(memoryRows[i].valueInput);
                        int newValue;
                        if (ss >> newValue) {
                            std::string errorMsg;
                            std::string addressStr = memoryRows[i].addressInput;
                            if (addressStr.empty()) {
                                statusMsg = "地址为空，行 " + std::to_string(i);
                                continue;
                            }
                            uintptr_t address = ParseAddressWithPointers(addressStr, hProcess, errorMsg);
                            if (address && memoryManager->WriteMemory((LPVOID)address, newValue)) {
                                statusMsg = "写入成功，行 " + std::to_string(i) + " 新值: " + std::to_string(newValue);
                                memoryRows[i].currentValue = newValue;
                                std::cout << "写入成功，地址: " << ToHexString(address) << ", 值: " << newValue << std::endl;
                            } else {
                                DWORD error = GetLastError();
                                statusMsg = "写入失败，错误码: " + std::to_string(error) + " (地址: " + ToHexString(address) + ")";
                                std::cerr << statusMsg << std::endl;
                                char errorMsgBuf[256];
                                FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, 0, errorMsgBuf, sizeof(errorMsgBuf), nullptr);
                                std::cerr << "错误详情: " << errorMsgBuf << std::endl;
                            }
                        } else {
                            statusMsg = "无效新值，行 " + std::to_string(i);
                        }
                    }
                }
            }
            ImGui::EndTable();
        }
    } else if (processAttached && (!memoryManager || !hProcess)) {
        statusMsg = "进程已附加但句柄或内存管理器无效，请重试！";
        std::cerr << statusMsg << std::endl;
    }

    if (!statusMsg.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", statusMsg.c_str());
    }

    if (ImGui::Button("关闭")) {
        PostQuitMessage(0);
    }
    ImGui::End();
}