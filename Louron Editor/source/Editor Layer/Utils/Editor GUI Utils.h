#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Utils::GUI {

    static void MultiRangeLODSliderFloat(const char* label, float* values, int count, float min = 0.0f, float max = 1.0f, float minSpacing = 0.01f) {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 slider_pos = ImGui::GetCursorScreenPos();
        float slider_width = ImGui::GetContentRegionAvail().x, slider_height = 20.0f;
        minSpacing = ImClamp(minSpacing, 0.0f, max - min);

        // Adjust cursor position and create interaction area
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + slider_height);
        ImGui::InvisibleButton(label, { slider_width, slider_height });
        ImGui::SetItemAllowOverlap();

        static int active_point = -1;

        auto DrawSegment = [&](float x1, float x2, ImU32 color, const std::string& text) {
            draw_list->AddRectFilled({ x1, slider_pos.y }, { x2, slider_pos.y + slider_height }, color);
            ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
            if (text_size.x < x2 - x1)
                draw_list->AddText({ x1 + (x2 - x1 - text_size.x) / 2, slider_pos.y + slider_height / 2 - text_size.y / 2 }, IM_COL32(255, 255, 255, 255), text.c_str());
            };

        // Draw first segment (LOD 0)
        DrawSegment(slider_pos.x, slider_pos.x + ((values[0] - min) / (max - min)) * slider_width, IM_COL32(100, 100, 100, 255), "LOD 1");

        // Draw remaining segments & handle interaction
        for (int i = 0; i < count; ++i) {
            float x1 = slider_pos.x + ((values[i] - min) / (max - min)) * slider_width;
            float x2 = (i < count - 1) ? slider_pos.x + ((values[i + 1] - min) / (max - min)) * slider_width : slider_pos.x + slider_width;
            bool is_last = (i == count - 1);

            DrawSegment(x1, x2, is_last ? IM_COL32(255, 114, 114, 255) : IM_COL32(100, 100, 100, 255), is_last ? "Culled" : "LOD " + std::to_string(i + 2));

            ImVec2 marker_pos = { x1, slider_pos.y + slider_height / 2 };
            draw_list->AddCircleFilled(marker_pos, 6.0f, IM_COL32(255, 255, 255, 255));

            // Handle marker interaction
            if (fabs(io.MousePos.x - x1) < 8.0f && fabs(io.MousePos.y - marker_pos.y) < 10.0f && ImGui::IsMouseClicked(0))
                active_point = i;

            if (active_point == i) {
                if (ImGui::IsMouseDown(0)) {
                    float new_value = min + ((io.MousePos.x - slider_pos.x) / slider_width) * (max - min);
                    values[i] = ImClamp(new_value, (i == 0) ? min : values[i - 1] + minSpacing, (i == count - 1) ? max : values[i + 1] - minSpacing);
                }
                else active_point = -1;
            }
        }
    }

	static void MultiRangeLODSliderFloat(const char* label, std::vector<float>& data, float min = 0.0f, float max = 1.0f, float minSpacing = 0.01f)
	{
		MultiRangeLODSliderFloat(label, data.data(), (int)data.size(), min, max, minSpacing);
	}


}