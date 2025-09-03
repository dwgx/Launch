cmake_minimum_required(VERSION 3.10)

project(Launch)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 针对 MinGW 的 C++20 UTF-8 兼容旗
if (MINGW)
    add_compile_options(-fno-char8_t)
endif()

# 添加 ImGui 的头文件目录（相对路径）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/imgui-1.92.2b)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Windows SDK 头文件（使用环境变量，逃逸括号）
if (NOT DEFINED WIN_SDK_INCLUDE_DIR)
    set(WIN_SDK_INCLUDE_DIR "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/Include/10.0.19041.0/um")
endif()
include_directories(${WIN_SDK_INCLUDE_DIR})

# Windows SDK 库路径
if (NOT DEFINED WIN_SDK_LIB_DIR)
    set(WIN_SDK_LIB_DIR "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/Lib/10.0.19041.0/um/x64")
endif()
link_directories(${WIN_SDK_LIB_DIR})

# 使用 GLOB 收集 ImGui 源文件
file(GLOB IMGUI_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui-1.92.2b/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui-1.92.2b/backends/imgui_impl_dx11.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui-1.92.2b/backends/imgui_impl_win32.cpp
)

# 收集项目源文件（明确指定 GUI 大写）
file(GLOB SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ProcessHandler/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp  # main.cpp
)

# 调试：打印收集的文件
message("IMGUI_SOURCES: ${IMGUI_SOURCES}")
message("SOURCES: ${SOURCES}")

# 添加可执行文件
add_executable(Launch ${IMGUI_SOURCES} ${SOURCES})

# 链接 DirectX 和 Windows 库
target_link_libraries(Launch PRIVATE d3d11 dxguid dwmapi d3dcompiler kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32)