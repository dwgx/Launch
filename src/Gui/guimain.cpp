#include "guimain.h"
#include "imgui.h"
#include "../ProcessHandler/ProcessHandler.h"
#include "../Utils/utils.h"  // split 函数
#include <sstream>
#include <iomanip>
#include <regex>
#include <windows.h>  // ReadProcessMemory

void RegisterMainWindow(MemoryManager*& memoryManager, bool& processAttached, std::string& statusMsg) {
    static int pid = 0;
    static std::vector<MemoryRow> rows;
    static std::string newAddressInput;
    static std::vector<char> addressInputBuffer(128, 0);  // 缓冲区 for ImGui
    static ProcessHandler* processHandler = nullptr;

    ImGui::Begin("主界面");

    // PID 输入和附加
    ImGui::InputInt("进程 PID", &pid);
    if (ImGui::Button("附加进程")) {
        processHandler = new ProcessHandler(pid);
        HANDLE hProcess = processHandler->OpenProcessHandle();
        if (hProcess == INVALID_HANDLE_VALUE || hProcess == NULL) {
            statusMsg = "附加失败: 无效 PID 或权限不足（请以管理员运行）";
            processAttached = false;
        } else {
            memoryManager = new MemoryManager(hProcess);
            statusMsg = "附加成功";
            processAttached = true;
        }
    }
    ImGui::Text("%s", statusMsg.c_str());

    if (processAttached) {
        // 智能添加行
        ImGui::InputText("添加地址", addressInputBuffer.data(), addressInputBuffer.size());
        if (ImGui::IsItemDeactivatedAfterEdit()) {  // Enter 后添加
            newAddressInput = addressInputBuffer.data();  // 同步到 std::string
            rows.emplace_back();
            rows.back().addressInput = newAddressInput;
            std::fill(addressInputBuffer.begin(), addressInputBuffer.end(), 0);  // 清空缓冲区
        }

        // 表格
        if (ImGui::BeginTable("内存表格", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("地址");
            ImGui::TableSetupColumn("当前值");
            ImGui::TableSetupColumn("修改值");
            ImGui::TableSetupColumn("操作");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < rows.size(); ++i) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", rows[i].addressInput.c_str());

                std::string errorMsg;
                uintptr_t addr = ParseAddressWithPointers(rows[i].addressInput, memoryManager->GetHandle(), errorMsg, processHandler);
                if (addr == 0) {
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("错误: %s", errorMsg.c_str());
                    continue;
                }

                memoryManager->ReadMemory<int>((LPCVOID)addr, rows[i].currentValue);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", rows[i].currentValue);

                ImGui::TableSetColumnIndex(2);
                std::vector<char> valueInputBuffer(32, 0);  // 每行独立缓冲区
                std::copy(rows[i].valueInput.begin(), rows[i].valueInput.end(), valueInputBuffer.begin());
                ImGui::PushID(static_cast<int>(i));  // 为每行添加唯一 ID
                ImGui::InputText("##value", valueInputBuffer.data(), valueInputBuffer.size());
                rows[i].valueInput = valueInputBuffer.data();  // 同步回 std::string
                ImGui::PopID();

                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(static_cast<int>(i));  // 为按钮添加唯一 ID
                if (ImGui::Button("修改")) {
                    try {
                        int newValue = std::stoi(rows[i].valueInput, nullptr, 10);
                        memoryManager->WriteMemory<int>((LPVOID)addr, newValue);
                    } catch (...) {
                        statusMsg = "修改失败: 无效值";
                    }
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }

    ImGui::End();
}

// 指针链解析
uintptr_t ParseAddressWithPointers(const std::string& input, HANDLE hProcess, std::string& errorMsg, ProcessHandler* processHandler) {
    std::vector<std::string> parts = split(input, "->");  // 处理 '->'

    if (parts.empty()) {
        errorMsg = "无效输入";
        return 0;
    }

    // 解析基址
    uintptr_t addr = 0;
    size_t plusPos = parts[0].find('+');
    if (plusPos != std::string::npos) {  // 模块+偏移
        std::string module = parts[0].substr(0, plusPos);
        std::string offsetStr = parts[0].substr(plusPos + 1);
        size_t idx = 0;
        try {
            addr = processHandler->GetModuleBaseAddress(module) + std::stoull(offsetStr, &idx, 16);
        } catch (...) {
            errorMsg = "无效偏移";
            return 0;
        }
    } else {  // 绝对地址
        size_t idx = 0;
        try {
            addr = std::stoull(parts[0], &idx, 16);
        } catch (...) {
            errorMsg = "无效地址";
            return 0;
        }
    }

    if (addr == 0) {
        errorMsg = "无效基址";
        return 0;
    }

    // 逐级指针
    for (size_t i = 1; i < parts.size(); ++i) {
        size_t idx = 0;
        uintptr_t offset;
        try {
            offset = std::stoull(parts[i], &idx, 16);
        } catch (...) {
            errorMsg = "无效指针偏移";
            return 0;
        }
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(hProcess, (LPCVOID)addr, &addr, sizeof(uintptr_t), &bytesRead) || bytesRead != sizeof(uintptr_t)) {
            errorMsg = "指针读取失败";
            return 0;
        }
        if (addr == 0) {
            errorMsg = "空指针";
            return 0;
        }
        addr += offset;
    }

    return addr;
}