# MemoryEditor 项目

## 项目概述
这是一个基于ImGui和DirectX11的内存编辑器工具，支持Windows进程内存读取/写入。目标是创建一个简单、可扩展的GUI界面，用于游戏或应用内存修改，类似于Cheat Engine (CE) 或 RECLASS 的功能。

## 主要目标和功能
- **核心功能**：
  - 输入目标进程PID，附加进程。
  - 支持添加表格行，每行可输入内存地址，支持多种格式：
    - 绝对 16 进制地址（如 `0x605FFDA00` 或 `605FFDA00`）。
    - 模块+偏移（如 `th12.exe+0xB0C44`）。
    - 指针链（如 `base->0x10->0x20`）。
  - 读取/写入内存值（当前支持int类型，可扩展）。
  - 显示当前值，并允许修改。
- **智能添加行**：
  - 支持在添加行时直接输入地址，自动填充到新行。
- **扩展性**：
  - GUI采用注册式设计：主界面（GuiMain）注册窗口，设置界面（GuiSetting）注册外观修改（如主题、字体）。
  - 便于未来添加更多功能，如内存扫描、值类型转换、多级指针扫描。
- **技术栈**：
  - C++20 with MinGW.
  - ImGui 1.92.2b for GUI.
  - DirectX11 for rendering.
  - Windows API for process/memory handling.

## 项目结构
- **src/**: 源代码。
  - main.cpp: 主入口，初始化DX11、ImGui，驱动GUI渲染循环。
  - gui.h/cpp: 通用GUI框架，处理ImGui上下文、字体加载、渲染。
  - guimain.h/cpp: 主界面逻辑（PID输入、表格添加、指针链解析，支持 CE/RECLASS 格式）。
  - guisetting.h/cpp: 设置界面（主题切换、字体大小等，便于修改外观）。
  - MemoryManager/: 内存读写管理。
- **include/**: 头文件和ImGui库。
- **CMakeLists.txt**: 构建文件。

## 构建和运行
- 依赖：MinGW, CMake, DirectX11 SDK, Windows SDK。
- 命令：cmake -S . -B build
  cmake --build build
  ./build/Launch.exe

## 未来计划
- 支持多级指针链扫描（动态解析）。
- 添加内存扫描功能（值搜索）。
- 主题自定义（dark/light）和字体调整。
- 扩展值类型（int, float, byte 等）。
- 错误日志和调试支持。

## 已知问题
- 字体支持：优先加载msyh.ttc以支持中文。
- 渲染：使用ClearRenderTargetView避免artifact。

作者：基于用户需求开发。日期：2025-09-03。