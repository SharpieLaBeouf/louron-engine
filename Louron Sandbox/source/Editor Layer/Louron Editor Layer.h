#pragma once

#include <iostream>
#include <string>

#include "Louron.h"
#include "Core/Layer.h"

class LouronEditorLayer : public Louron::Layer {
public:

	LouronEditorLayer();
	virtual ~LouronEditorLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate() override;
	virtual void OnFixedUpdate() override;
	virtual void OnGuiRender() override;

private:

	void DisplayViewportWindow();

	void DisplayHierarchyWindow();
	void DisplayPropertiesWindow();

	void DisplayContentBrowserWindow();

	void DisplayRenderStatsWindow();
	void DisplayProfilerWindow();
	void DisplayAssetRegistryWindow();

	std::unordered_map<const char*, bool> m_ActiveGUIWindows;

	bool m_SceneWindowFocused = false;
	glm::ivec2 m_ViewportWindowSize = { 800, 600 };

	Louron::Entity m_SelectedEntity;

	template <typename ComponentType>
	void ShowComponentContextPopup(const char* context_window_name) {
		if (ImGui::BeginPopupContextItem(context_window_name)) {
			if (ImGui::MenuItem("Remove Component")) {
				m_SelectedEntity.RemoveComponent<ComponentType>();
			}
			ImGui::EndPopup();
		}
	}
};