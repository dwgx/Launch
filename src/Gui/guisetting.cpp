#include "guisetting.h"
#include "imgui.h"

void RegisterSettingWindow() {
    ImGui::Begin("设置界面");

    // 示例：修改外观（可扩展更多）
    if (ImGui::Button("切换暗黑主题")) {
        ImGui::StyleColorsDark();
    }
    if (ImGui::Button("切换亮白主题")) {
        ImGui::StyleColorsLight();
    }

    // 以后添加字体大小、颜色等

    ImGui::End();
}