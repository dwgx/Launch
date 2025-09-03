cmake_minimum_required(VERSION 3.10)

project(Launch)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 新增：针对 GCC/MinGW 的 C++20 UTF-8 兼容旗
add_compile_options(-fno-char8_t)

# 添加 ImGui 的头文件目录
include_directories(include)
include_directories(D:/Project/Launch/include/imgui-1.92.2b)
include_directories(src)

# 添加 Windows SDK 的头文件目录
include_directories(D:/Software/Windows Kits/10/Include/10.0.19041.0/um)

# 设置 DirectX 11 和 Windows SDK 的库文件路径
link_directories(D:/Software/Windows Kits/10/Lib/10.0.19041.0/um/x64)

# 设置 ImGui 的源文件
set(IMGUI_SOURCES
        D:/Project/Launch/include/imgui-1.92.2b/imgui.cpp
        D:/Project/Launch/include/imgui-1.92.2b/imgui_draw.cpp
        D:/Project/Launch/include/imgui-1.92.2b/imgui_widgets.cpp
        D:/Project/Launch/include/imgui-1.92.2b/imgui_tables.cpp
        D:/Project/Launch/include/imgui-1.92.2b/imgui_demo.cpp
        D:/Project/Launch/include/imgui-1.92.2b/backends/imgui_impl_dx11.cpp
        D:/Project/Launch/include/imgui-1.92.2b/backends/imgui_impl_win32.cpp
)

# 添加源文件
set(SOURCES
        src/ProcessHandler/ProcessHandler.cpp
        src/MemoryManager/MemoryManager.cpp
        src/main.cpp
        src/Gui/gui.cpp
        src/Gui/gui.h
        src/Gui/guimain.cpp
        src/Gui/guimain.h
        src/Gui/guisetting.cpp
        src/Gui/guisetting.h
)

# 添加可执行文件
add_executable(Launch ${IMGUI_SOURCES} ${SOURCES})

# 链接 DirectX 和 Windows SDK 库
target_link_libraries(Launch PRIVATE d3d11 dxguid dwmapi d3dcompiler kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32)