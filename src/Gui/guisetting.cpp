#include "guisetting.h"
#include "imgui.h"

void RegisterSettingWindow() {
    ImGui::Begin("设置界面");

    if (ImGui::Button("切换暗黑主题")) {
        ImGui::StyleColorsDark();
    }
    if (ImGui::Button("切换亮白主题")) {
        ImGui::StyleColorsLight();
    }

    ImGui::End();
}