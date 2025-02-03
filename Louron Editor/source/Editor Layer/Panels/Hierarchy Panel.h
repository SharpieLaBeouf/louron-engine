#pragma once

#include "Louron.h"

class HierarchyPanel {

public:

	HierarchyPanel() = default;

	void OnImGuiRender(const std::shared_ptr<Louron::Scene>& scene_ref, Louron::Entity& selected_entity);

	Louron::Entity m_NewFocalEntity = {};

	friend class LouronEditorLayer;
};