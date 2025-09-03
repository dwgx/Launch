# Launch

## 项目概述
这是一个基于 ImGui 和 DirectX11 的内存编辑器工具，支持 Windows 进程内存读取/写入。目标是创建一个简单、可扩展的 GUI 界面，用于游戏或应用内存修改，类似于 Cheat Engine (CE) 或 RECLASS 的功能。

## 主要目标和功能
- **核心功能**：
  - 输入目标进程 PID，附加进程。
  - 支持添加表格行，每行可输入内存地址，支持多种格式：
    - 绝对 16 进制地址（如 `0x605FFDA00` 或 `605FFDA00`）。
    - 模块+偏移（如 `th12.exe+0xB0C44`）。
    - 指针链（如 `base->0x10->0x20`）。
  - 读取/写入内存值（当前支持 `int` 类型，可扩展）。
  - 显示当前值，并允许修改。
- **智能添加行**：
  - 支持在添加行时直接输入地址，自动填充到新行。
- **扩展性**：
  - GUI 采用注册式设计：主界面（`GuiMain`）注册窗口，设置界面（`GuiSetting`）注册外观修改（如主题、字体）。
  - 便于未来添加更多功能，如内存扫描、值类型转换、多级指针扫描。
- **技术栈**：
  - C++20 with MinGW.
  - ImGui 1.92.2b for GUI.
  - DirectX11 for rendering.
  - Windows API for process/memory handling.

## 项目结构
- **src/**: 源代码。
  - `main.cpp`: 主入口，初始化 DX11、ImGui，驱动 GUI 渲染循环。
  - `gui.h/cpp`: 通用 GUI 框架，处理 ImGui 上下文、字体加载、渲染。
  - `guimain.h/cpp`: 主界面逻辑（PID 输入、表格添加、指针链解析，支持 CE/RECLASS 格式）。
  - `guisetting.h/cpp`: 设置界面（主题切换、字体大小等，便于修改外观）。
  - `MemoryManager/`: 内存读写管理。
  - `ProcessHandler/`: 进程句柄和模块基址管理。
  - `Utils/`: 工具函数（如字符串分割）。
- **include/**: 头文件和 ImGui 库。
- **CMakeLists.txt**: 构建文件。

## 构建和运行
1. **依赖**：
  - MinGW
  - CMake
  - DirectX11 SDK
  - Windows SDK
2. **构建步骤**：
  - 克隆仓库：`git clone https://github.com/dwgx1337/Launch.git`
  - 进入项目目录：`cd Launch`
  - 创建构建目录并配置：`cmake -S . -B build`
  - 编译项目：`cmake --build build`
  - 运行：`./build/Launch.exe`（需以管理员权限运行以附加进程）。
3. **注意**：
  - 确保 `include/imgui-1.92.2b/` 包含 ImGui 库文件（可从 [ImGui GitHub](https://github.com/ocornut/imgui) 下载 1.92.2b 版本）。

## 未来计划
- 支持多级指针链扫描（动态解析）。
- 添加内存扫描功能（值搜索）。
- 主题自定义（dark/light）和字体调整。
- 扩展值类型（`int`, `float`, `byte` 等）。
- 错误日志和调试支持。

## 已知问题
- **字体支持**：优先加载 `msyh.ttc` 以支持中文，若无则回退到默认字体。
- **渲染**：使用 `ClearRenderTargetView` 避免渲染 artifact。

## 贡献
欢迎提交 Issue 或 Pull Request！请遵循项目编码风格（C++20，MinGW 兼容）。

## 作者
基于用户需求开发。

## 日期
2025-09-03