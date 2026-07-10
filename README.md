# Launch

> Windows 进程内存查看/编辑器 — 我的第一个 C++ / DirectX 11 / ImGui 项目，留着当成长记录。
>
> Windows process memory viewer/editor — my first C++ / DirectX 11 / ImGui project, kept as a growth log.

<!-- TODO: 如有截图，可在此处添加 GUI 界面图 / add a screenshot of the GUI if available -->

## Overview / 概述

Launch is a desktop tool built with Dear ImGui and DirectX 11 that attaches to Windows processes and lets you read/write their memory in real time. You punch in a PID, add memory addresses to a table, and see or change the values live. Think of it as a stripped-down Cheat Engine — useful for game memory editing, debugging, or just learning how Win32 process memory works.

Launch 是一个基于 Dear ImGui 和 DirectX 11 的桌面工具，可以附加到 Windows 进程并实时读写内存。输入目标进程 PID，在表格里填入内存地址，即可查看和修改对应的值。功能上就是极简版的 Cheat Engine —— 可以用来改游戏内存、调试进程，或者单纯学习 Win32 进程内存操作。

## Features / 功能

- 输入目标进程 PID 附加进程（需管理员权限）
- 表格界面逐行管理内存地址，支持多种格式：
  - 绝对十六进制地址：`0x605FFDA00` / `605FFDA00`
  - 模块 + 偏移：`th12.exe+0xB0C44`
  - 多级指针链：`base->0x10->0x20`（逐级 ReadProcessMemory 解析）
- 实时读取并显示当前值，支持直接写入修改（当前为 `int` 类型）
- 输入框回车智能添加新行
- 暗色 / 亮色主题一键切换
- 中文字体自动加载（msyh.ttc → simsun.ttc → meiryo.ttc → ImGui 默认）

---

- Attach to any process by PID (requires Administrator)
- Table-based UI for managing memory addresses, supporting:
  - Absolute hex addresses: `0x605FFDA00` / `605FFDA00`
  - Module + offset: `th12.exe+0xB0C44`
  - Multi-level pointer chains: `base->0x10->0x20` (resolved via chained ReadProcessMemory)
- Real-time value display with direct write-back (currently `int` type)
- Smart row addition via Enter key
- Dark / Light theme toggle
- Auto-loads Chinese fonts (msyh.ttc → simsun.ttc → meiryo.ttc → ImGui default)

## Tech Stack / 技术栈

| Component | Detail |
|-----------|--------|
| Language | C++20 |
| GUI | Dear ImGui 1.92.2b (vendored in `include/imgui-1.92.2b/`) |
| Rendering | DirectX 11 + Win32 backend |
| Platform API | Windows API (`ReadProcessMemory`, `WriteProcessMemory`, `OpenProcess`, Toolhelp32) |
| Build | CMake 3.10+ / Visual Studio (`Launch.sln`) |
| Linked libs | d3d11, dxguid, dwmapi, d3dcompiler + standard Win32 libs |

## Project Structure / 项目结构

```
Launch/
├── CMakeLists.txt              # CMake 构建脚本
├── Launch.sln                  # Visual Studio 解决方案
├── Launch/                     # VS 工程文件 (.vcxproj)
├── include/
│   └── imgui-1.92.2b/          # Dear ImGui 源码 + DX11/Win32 后端
└── src/
    ├── main.cpp                # 入口：管理员权限检查，初始化 GUI 循环
    ├── Gui/
    │   ├── gui.cpp/.h          # DX11/ImGui 初始化、渲染循环
    │   ├── guimain.cpp/.h      # 主界面：PID 附加、内存表格、指针链解析
    │   └── guisetting.cpp/.h   # 设置：主题切换
    ├── MemoryManager/          # 模板化 ReadMemory / WriteMemory 封装
    ├── ProcessHandler/         # 打开进程句柄、获取模块基址
    └── Utils/                  # 工具函数（字符串 split 等）
```

<!-- 注意: CMakeLists 中源目录为 src/GUI（大写），仓库实际目录为 src/Gui；大小写敏感文件系统上需要对齐。 -->

## Getting Started / 快速开始

### Prerequisites / 前置条件

- Windows
- C++20 编译器（MSVC 或 MinGW）
- CMake 3.10+
- Windows SDK（默认 `10.0.19041.0`，可通过 `WIN_SDK_INCLUDE_DIR` / `WIN_SDK_LIB_DIR` 覆盖）

### Build with CMake / CMake 构建

```bash
git clone https://github.com/dwgx/Launch.git
cd Launch
cmake -S . -B build
cmake --build build
```

<!-- TODO: 依生成器不同确认最终 exe 路径 / confirm exe path per generator -->

### Build with Visual Studio

直接打开 `Launch.sln`，生成 `Launch` 工程。

Open `Launch.sln` and build the `Launch` project.

<!-- TODO: 确认 vcxproj 中 include 路径和目标平台设置 / confirm include paths & target platform -->

### Run / 运行

以**管理员身份**运行 `Launch.exe`。附加进程需要 `PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION` 权限，非管理员下大多数进程会附加失败。

Run `Launch.exe` **as Administrator**. Attaching requires VM read/write/operation rights — without elevation, most targets will fail.

## Usage / 使用方法

1. 以管理员身份启动。
2. 输入目标进程 PID，点击「附加进程」。
3. 在地址输入框填入地址并回车添加行：
   - `0x605FFDA00`（绝对地址）
   - `th12.exe+0xB0C44`（模块 + 偏移）
   - `th12.exe+0xB0C44->0x10->0x20`（指针链）
4. 表格实时显示每行地址的值；在「修改值」列填入新值点「修改」写入。
5. 设置界面切换暗色/亮色主题。

---

1. Run as Administrator.
2. Enter target PID and click Attach.
3. Type addresses into the input box and hit Enter:
   - `0x605FFDA00` (absolute)
   - `th12.exe+0xB0C44` (module + offset)
   - `th12.exe+0xB0C44->0x10->0x20` (pointer chain)
4. The table shows live values; edit the value column and click Write to modify.
5. Switch dark/light theme in Settings.

## Configuration / 配置

构建期变量：

- `WIN_SDK_INCLUDE_DIR` — Windows SDK `um` 头文件目录
- `WIN_SDK_LIB_DIR` — Windows SDK x64 库目录

字体路径硬编码为 Windows 系统字体目录（`C:/Windows/Fonts/msyh.ttc` 等），找不到时自动回退。

---

Build-time variables:

- `WIN_SDK_INCLUDE_DIR` — Windows SDK `um` include path
- `WIN_SDK_LIB_DIR` — Windows SDK x64 lib path

Font paths are hardcoded to standard Windows font locations; falls back to ImGui default if not found.

## Status / 状态

练习项目，当成长记录保留。功能有限——只支持 int 读写、单进程附加。内存扫描、更多值类型、多级指针扫描等还没做。

Learning project, kept as a growth log. Feature set is intentionally minimal: int-only read/write, single process attach. Memory scanning, additional value types, and pointer scanning are not yet implemented.

## License / 许可证

MIT License — see [`LICENSE`](LICENSE).

`include/imgui-1.92.2b/` 下的 Dear ImGui 源码版权归 Omar Cornut 及贡献者所有，遵循其自身 MIT 许可证。
