#pragma once

#include "Louron.h"

class PropertiesPanel {

public:

	PropertiesPanel() = default;

	void OnImGuiRender(const std::shared_ptr<Louron::Scene>& scene_ref, Louron::Entity selected_entity);

	void DisplayScriptFields(const std::string& script_name, Louron::Entity selected_entity);

	void DisplayEntitySelectionModal(Louron::Entity& selected_entity);

	template <typename ComponentType>
	void ShowComponentContextPopup(const char* context_window_name, Louron::Entity selected_entity) {
		if (ImGui::BeginPopupContextItem(context_window_name)) {
			if (ImGui::MenuItem("Remove Component")) {
				selected_entity.RemoveComponent<ComponentType>();
			}
			ImGui::EndPopup();
		}
	}
};