#include "../Louron Editor Layer.h"
#include "Properties Panel.h"

#include "../Utils/Editor Script Utils.h"

#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

using namespace Louron;

void PropertiesPanel::OnImGuiRender(const std::shared_ptr<Scene>& scene_ref, Entity selected_entity)
{
	std::vector<AssetHandle> material_list;

	ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen;

	// Right-click on blank space
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem("Add Camera Component")) {
			if (!selected_entity.HasComponent<CameraComponent>()) {
				auto& component = selected_entity.AddComponent<CameraComponent>();
				component.CameraInstance = std::make_shared<SceneCamera>();

				auto& frame_buffer_config = Project::GetActiveScene()->GetSceneFrameBuffer()->GetConfig();
				component.CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);
			}
		}

		// Add Component Section
		{

			if (ImGui::MenuItem("Add Script Component")) {
				if (!selected_entity.HasComponent<ScriptComponent>())
					selected_entity.AddComponent<ScriptComponent>();
			}

			if (ImGui::MenuItem("Add Skybox Component")) {
				if (!selected_entity.HasComponent<SkyboxComponent>())
					selected_entity.AddComponent<SkyboxComponent>();
			}

			if (ImGui::MenuItem("Add MeshFilter")) {
				if (!selected_entity.HasComponent<AssetMeshFilter>())
					selected_entity.AddComponent<AssetMeshFilter>();
			}

			if (ImGui::MenuItem("Add MeshRenderer")) {
				if (!selected_entity.HasComponent<AssetMeshRenderer>())
					selected_entity.AddComponent<AssetMeshRenderer>();
			}

			if (ImGui::MenuItem("Add Rigidbody")) {
				if (!selected_entity.HasComponent<RigidbodyComponent>())
					selected_entity.AddComponent<RigidbodyComponent>();
			}

			if (ImGui::MenuItem("Add Sphere Collider")) {
				if (!selected_entity.HasComponent<SphereColliderComponent>())
					selected_entity.AddComponent<SphereColliderComponent>();
			}

			if (ImGui::MenuItem("Add Box Collider")) {
				if (!selected_entity.HasComponent<BoxColliderComponent>())
					selected_entity.AddComponent<BoxColliderComponent>();
			}

			if (ImGui::MenuItem("Add Point Light Component")) {
				if (!selected_entity.HasComponent<PointLightComponent>())
					selected_entity.AddComponent<PointLightComponent>();
			}

			if (ImGui::MenuItem("Add Spot Light Component")) {
				if (!selected_entity.HasComponent<SpotLightComponent>())
					selected_entity.AddComponent<SpotLightComponent>();
			}

			if (ImGui::MenuItem("Add Directional Light Component")) {
				if (!selected_entity.HasComponent<DirectionalLightComponent>())
					selected_entity.AddComponent<DirectionalLightComponent>();
			}
		}

		ImGui::EndPopup();
	}

	ImGui::BeginChild("##Immutable Components", {}, ImGuiChildFlags_AutoResizeY);

	float first_coloumn_width = ImGui::GetContentRegionAvail().x * 0.35f;

	if (selected_entity.HasComponent<TagComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::Columns(2, "entity_properties_cols", false);
		ImGui::SetColumnWidth(-1, ImGui::CalcTextSize("Name").x + 10.0f);

		ImGui::Text("Name");

		ImGui::NextColumn();

		auto& component = selected_entity.GetComponent<TagComponent>();

		char tag_buffer[256];
		strncpy_s(tag_buffer, component.Tag.c_str(), sizeof(tag_buffer));
		tag_buffer[sizeof(tag_buffer) - 1] = '\0'; // Ensure null-termination

		ImGui::InputText("##TagDisplay", tag_buffer, sizeof(tag_buffer), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::IsItemDeactivatedAfterEdit()) {
			// Update the tag if the input box is deactivated (Enter pressed or box loses focus)
			component.Tag = std::string(tag_buffer);
		}

		ImGui::NextColumn();

		ImGui::Text("ID");

		ImGui::NextColumn();

		char id_buffer[11];
		strncpy_s(id_buffer, std::to_string(selected_entity.GetUUID()).c_str(), sizeof(id_buffer));
		id_buffer[sizeof(id_buffer) - 1] = '\0'; // Ensure null-termination

		ImGui::InputText("##IDDisplay", id_buffer, sizeof(id_buffer), ImGuiInputTextFlags_ReadOnly);

		ImGui::Columns(1);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<TransformComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::Text("Transform Component\n ");
		TransformComponent& entity_transform = selected_entity.GetComponent<TransformComponent>();

		ImGui::Columns(2, "transform_columns", false);

		ImGui::SetColumnWidth(-1, glm::min(ImGui::CalcTextSize("Position").x + ImGui::GetStyle().ItemSpacing.x * 2, first_coloumn_width));

		ImGui::Text("Position");
		ImGui::NextColumn();

		// Get the width of the current column
		float columnWidth = ImGui::GetColumnWidth() - (ImGui::GetStyle().ItemSpacing.x * 6 + ImGui::CalcTextSize("X").x * 3); // Account for spacing

		glm::vec3 value = entity_transform.GetLocalPosition();
		bool updated = false;

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local PositionX", &value.x, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local PositionY", &value.y, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local PositionZ", &value.z, 0.1f, 0, 0, "%.2f")) updated = true;

		if (updated) entity_transform.SetPosition(value);

		ImGui::NextColumn();
		ImGui::Text("Rotation");
		ImGui::NextColumn();

		value = entity_transform.GetLocalRotation();
		updated = false;

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local RotationX", &value.x, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local RotationY", &value.y, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local RotationZ", &value.z, 0.1f, 0, 0, "%.2f")) updated = true;

		if (updated) entity_transform.SetRotation(value);

		ImGui::NextColumn();
		ImGui::Text("Scale");
		ImGui::NextColumn();

		value = entity_transform.GetLocalScale();
		updated = false;

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local ScaleX", &value.x, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local ScaleY", &value.y, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local ScaleZ", &value.z, 0.1f, 0, 0, "%.2f")) updated = true;

		if (updated) entity_transform.SetScale(value);

		ImGui::NextColumn();

		ImGui::Columns(1);

		ImGui::Dummy({ 0.0f, 5.0f });
	}

	ImGui::EndChild();

	ImGui::Separator();

	ImGui::BeginChild("##Mutable Components", {}, ImGuiChildFlags_AutoResizeY);

	if (selected_entity.HasComponent<ScriptComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		bool selected = false;

		ImGui::BeginChild("##ScriptComponentChild", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Script Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<ScriptComponent>();
			std::vector<std::pair<std::string, bool>>& component_script_vector = component.Scripts;

			int index = 0;
			for (auto it = component_script_vector.begin(); it != component_script_vector.end(); ) {

				bool& active = it->second;
				std::string& script_name = it->first;

				std::string label = "Script##" + std::to_string(index);

				if (ImGui::TreeNodeEx(label.c_str(), tree_node_flags | ImGuiTreeNodeFlags_OpenOnDoubleClick)) {

					ImGui::Columns(2, "script_columns", false);
					ImGui::SetColumnWidth(-1, ImGui::CalcTextSize("Script").x + 10.0f);

					ImGui::Text("Active");
					ImGui::NextColumn();
					ImGui::Checkbox("##ActiveCheckBox", &active);
					ImGui::NextColumn();

					ImGui::Text("Script");
					ImGui::NextColumn();

					const auto& script_classes = ScriptManager::GetEntityClasses();
					std::vector<const char*> available_scripts;
					available_scripts.reserve(script_classes.size() + 1);
					available_scripts.push_back(" ");

					int i = 0;
					int current_item = -1;
					for (const auto& script : script_classes) {

						available_scripts.push_back(script.first.c_str());

						if (script.first == script_name)
							current_item = i + 1;

						i++;
					}

					std::string combo_label = "##ScriptCombo_" + script_name;
					if (ImGui::BeginCombo(combo_label.c_str(), current_item != -1 ? available_scripts.data()[current_item] : "")) {

						for (int n = 0; n < available_scripts.size(); n++)
						{
							const bool is_selected = (current_item == n);
							if (ImGui::Selectable(available_scripts[n], is_selected))
							{
								current_item = n;
								script_name = available_scripts[current_item];
							}

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					ImGui::Columns(1);

					// Ensure that the script class is valid
					// Check if has fields, if no fields, we won't display anything for fields
					auto script_class = script_classes.find(script_name);
					if (script_class != script_classes.end() && script_class->second && !script_class->second->GetFields().empty()) {

						ImGui::Dummy({ 0.0f, 5.0f });
						ImGui::SeparatorText("Fields");
						ImGui::Dummy({ 0.0f, 5.0f });

						// SCRIPT FIELDS
						DisplayScriptFields(script_name, selected_entity);

						ImGui::Dummy({ 0.0f, 5.0f });
						ImGui::SeparatorText("");
					}

					ImGui::Dummy({ 0.0f, 2.5f });

					ImVec2 available = ImGui::GetContentRegionAvail();
					float button_width = ImGui::CalcTextSize("Create New Script").x + ImGui::GetStyle().FramePadding.x * 2.0f;
					if (available.x > button_width) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (available.x - button_width) * 0.5f);

					if (ImGui::Button("Remove Script")) {
						it = component_script_vector.erase(it);
						ImGui::TreePop();
						continue;
					}

					ImGui::Dummy({ 0.0f, 2.5f });

					ImGui::TreePop();
				}

				++it;
				index++;
			}

			ImGui::Dummy({ 0.0f, 10.0f });

			ImGui::Columns(2, "script_columns_buttons", false);

			static bool script_create = false;

			if (scene_ref->IsRunning()) ImGui::BeginDisabled();
			if (ImGui::Button("Create New Script") || script_create) {
				script_create = true;
			}
			if (scene_ref->IsRunning()) ImGui::EndDisabled();

			ImGui::NextColumn();

			if (ImGui::Button("Add Existing Script")) {
				component_script_vector.push_back({ "", true });
			}

			ImGui::Columns(1);

			auto script_full_name = Utils::OnCreateNewScriptGUI(&script_create);
			if (!script_full_name.empty())
				component_script_vector.push_back({ script_full_name, true });

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<ScriptComponent>("Script Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<CameraComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		bool selected = false;

		ImGui::BeginChild("##CamerComponentChild", {}, ImGuiChildFlags_AutoResizeY);


		if (ImGui::TreeNodeEx("Camera Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<CameraComponent>();

			ImGui::Columns(2, "camera_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);


			std::array<const char*, 2> camera_projection_types = { "Perspective", "Orthographic" };
			uint8_t item_current = static_cast<uint8_t>(component.CameraInstance->GetProjectionType());
			ImGui::Text("Projection Type");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##CameraProjectTypes", camera_projection_types[item_current])) {

				for (int n = 0; n < camera_projection_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(camera_projection_types[n], is_selected))
					{
						item_current = n;
						component.CameraInstance->SetProjectionType(static_cast<SceneCamera::ProjectionType>(item_current));
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			ImGui::Text("FOV");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			auto& frame_buffer_config = scene_ref->GetSceneFrameBuffer()->GetConfig();
			float data = glm::degrees(component.CameraInstance->GetPerspectiveVerticalFOV());
			if (ImGui::DragFloat("##CameraFOV", &data, 0.05f, 0.1f, std::numeric_limits<float>::max(), "%.2f"))
			{
				component.CameraInstance->SetPerspectiveVerticalFOV(glm::radians(data));
				component.CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);
			}
			ImGui::NextColumn();

			ImGui::Text("Near");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			data = component.CameraInstance->GetPerspectiveNearClip();
			if (ImGui::DragFloat("##CameraNear", &data, 0.05f, 0.1f, std::numeric_limits<float>::max(), "%.2f"))
			{
				component.CameraInstance->SetPerspectiveNearClip(data);
				component.CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);
			}
			ImGui::NextColumn();

			ImGui::Text("Far");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			data = component.CameraInstance->GetPerspectiveFarClip();
			if (ImGui::DragFloat("##CameraFar", &data, 0.05f, 0.1f, std::numeric_limits<float>::max(), "%.2f"))
			{
				component.CameraInstance->SetPerspectiveFarClip(data);
				component.CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);
			}
			ImGui::NextColumn();

			ImGui::Text("Primary");
			ImGui::NextColumn();
			ImGui::Checkbox("##PrimaryCheckBox", &component.Primary);
			ImGui::NextColumn();

			std::array<const char*, 2> camera_clear_types = { "Colour Only", "Skybox" };
			item_current = static_cast<uint8_t>(component.ClearFlags);
			ImGui::Text("Clear Flag");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##CameraClearColour", camera_clear_types[item_current])) {

				for (int n = 0; n < camera_clear_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(camera_clear_types[n], is_selected))
					{
						item_current = n;
						component.ClearFlags = static_cast<CameraClearFlags>(item_current);
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			ImGui::Text("Clear Colour");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::ColorEdit4("##SpotLightColour", glm::value_ptr(component.ClearColour));
			ImGui::NextColumn();

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<CameraComponent>("Camera Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<SkyboxComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Skybox Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Skybox Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<SkyboxComponent>();
			material_list.push_back(component.SkyboxMaterialAssetHandle);

			std::string skybox_material_name;
			ImVec4 text_colour = ImGui::GetStyleColorVec4(ImGuiCol_Text);

			if (component.SkyboxMaterialAssetHandle != NULL_UUID && Project::GetStaticEditorAssetManager()->IsAssetHandleValid(component.SkyboxMaterialAssetHandle)) {
				skybox_material_name = Project::GetStaticEditorAssetManager()->GetMetadata(component.SkyboxMaterialAssetHandle).AssetName;
			}
			else if (component.SkyboxMaterialAssetHandle != NULL_UUID) {
				skybox_material_name = "Asset Handle Invalid: " + std::to_string(component.SkyboxMaterialAssetHandle);
				text_colour = { 1.0f, 0.35f, 0.35f, 1.0f };
			}
			else {
				skybox_material_name = "None";
			}

			char asset_name_buf[256];
			strncpy_s(asset_name_buf, skybox_material_name.c_str(), sizeof(asset_name_buf));
			asset_name_buf[sizeof(asset_name_buf) - 1] = '\0'; // Ensure null-termination

			ImGui::Columns(2, "skybox_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);
			ImGui::Text("Material:");
			ImGui::NextColumn();

			// Calculate the width of the text labels and buttons
			float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
			float closeButtonWidth = ImGui::CalcTextSize(" X ").x + ImGui::GetStyle().FramePadding.x * 2;
			float spacing = ImGui::GetStyle().ItemSpacing.x;

			// Calculate the available width for the InputText
			float availableWidth = ImGui::GetColumnWidth() - buttonWidth - closeButtonWidth - spacing * 4;

			ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
			ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

			ImGui::BeginDisabled(true);

			if (availableWidth > ImGui::CalcTextSize(skybox_material_name.c_str()).x)
				ImGui::SetNextItemWidth(availableWidth);
			else
				ImGui::SetNextItemWidth(-1);

			ImGui::InputText("##SkyboxAssetMaterial", asset_name_buf, sizeof(asset_name_buf), ImGuiInputTextFlags_ReadOnly);
			ImGui::EndDisabled();

			// Drag target
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
					AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

					if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Skybox) {
						component.SkyboxMaterialAssetHandle = dropped_asset_handle;
					}
					else {
						L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			if (availableWidth > ImGui::CalcTextSize(skybox_material_name.c_str()).x)
				ImGui::SameLine();

			if (ImGui::Button("...")) {
				L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR SKYBOX MATERIAL!");
			}

			ImGui::SameLine();
			if (ImGui::Button(" X ")) {
				component.SkyboxMaterialAssetHandle = NULL_UUID;
			}

			ImGui::NextColumn();
			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<SkyboxComponent>("Skybox Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<AssetMeshFilter>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Mesh Filter Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Mesh Filter Component", tree_node_flags)) {
			auto& component = selected_entity.GetComponent<AssetMeshFilter>();

			std::string mesh_filter_name;
			ImVec4 text_colour = ImGui::GetStyleColorVec4(ImGuiCol_Text);

			if (component.MeshFilterAssetHandle != NULL_UUID && Project::GetStaticEditorAssetManager()->IsAssetHandleValid(component.MeshFilterAssetHandle)) {
				mesh_filter_name = Project::GetStaticEditorAssetManager()->GetMetadata(component.MeshFilterAssetHandle).AssetName;
			}
			else if (component.MeshFilterAssetHandle != NULL_UUID) {
				mesh_filter_name = "Asset Handle Invalid: " + std::to_string(component.MeshFilterAssetHandle);
				text_colour = { 1.0f, 0.35f, 0.35f, 1.0f };
			}
			else {
				mesh_filter_name = "None";
			}

			char asset_name_buf[256];
			strncpy_s(asset_name_buf, mesh_filter_name.c_str(), sizeof(asset_name_buf));
			asset_name_buf[sizeof(asset_name_buf) - 1] = '\0'; // Ensure null-termination

			ImGui::Columns(2, "mesh_filter_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);
			ImGui::Text("Mesh Filter");
			ImGui::NextColumn();

			ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
			ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

			ImGui::BeginDisabled(true);
			{

				float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
				float availableWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
				ImGui::PushItemWidth(availableWidth);

				ImGui::InputText("##MeshFilterName", asset_name_buf, sizeof(asset_name_buf), ImGuiInputTextFlags_ReadOnly);

				ImGui::PopItemWidth();
			}
			ImGui::EndDisabled();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			// Drag target
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
					AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

					if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Mesh) {
						component.MeshFilterAssetHandle = dropped_asset_handle;
						component.AABBNeedsUpdate = true;
						component.OctreeNeedsUpdate = true;
						Project::GetStaticEditorAssetManager()->GetAsset<AssetMesh>(component.MeshFilterAssetHandle); // Force load the Asset on the main thread/GL context
					}
					else {
						L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
			if (ImGui::Button("...")) {
				L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR MESHES!");
			}

			ImGui::NextColumn();

			ImGui::Text("Show Debug AABB");
			ImGui::NextColumn();
			bool show_debug = component.GetShouldDisplayDebugLines();
			if (ImGui::Checkbox("##DebugAABBCheckBox", &show_debug))
				component.SetShouldDisplayDebugLines(show_debug);

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<AssetMeshFilter>("Mesh Filter Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<AssetMeshRenderer>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Mesh Renderer Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Mesh Renderer Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<AssetMeshRenderer>();

			if (component.MeshRendererMaterialHandles.empty()) {
				component.MeshRendererMaterialHandles.push_back(NULL_UUID);
			}

			for (const auto& asset_handle : component.MeshRendererMaterialHandles) {
				if (asset_handle != NULL_UUID) {
					material_list.push_back(asset_handle);
				}
			}

			ImGui::Columns(2, "mesh_renderer_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Active");

			ImGui::NextColumn();

			ImGui::Checkbox("##MeshRendererActive", &component.Active);

			ImGui::NextColumn();

			ImGui::Text("Cast Shadows");

			ImGui::NextColumn();

			ImGui::Checkbox("##MeshRendererCastShadows", &component.CastShadows);

			ImGui::NextColumn();

			ImGui::Columns(1);

			if (ImGui::TreeNodeEx("Materials", tree_node_flags)) {

				ImGui::Dummy({ 0.0f, 5.0f });

				int i = 0;
				ImGui::Columns(2, "mesh_renderer_material_columns", false);
				ImGui::SetColumnWidth(-1, first_coloumn_width);
				for (auto& asset_handle : component.MeshRendererMaterialHandles) {

					std::string material_name;
					ImVec4 text_colour = ImGui::GetStyleColorVec4(ImGuiCol_Text);

					if (asset_handle != NULL_UUID && Project::GetStaticEditorAssetManager()->IsAssetHandleValid(asset_handle)) {
						material_name = Project::GetStaticEditorAssetManager()->GetMetadata(asset_handle).AssetName;
					}
					else if (asset_handle != NULL_UUID) {
						material_name = "Invalid Asset: " + std::to_string(asset_handle);
						text_colour = { 1.0f, 0.35f, 0.35f, 1.0f };
					}
					else {
						material_name = "None";
					}

					char asset_name_buf[256];
					strncpy_s(asset_name_buf, material_name.c_str(), sizeof(asset_name_buf));
					asset_name_buf[sizeof(asset_name_buf) - 1] = '\0'; // Ensure null-termination

					ImGui::Text("Element %i: ", i);
					ImGui::NextColumn();

					ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
					ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

					ImGui::BeginDisabled(true);
					{
						float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
						float availableWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
						ImGui::PushItemWidth(availableWidth);

						ImGui::InputText("##MaterialName", asset_name_buf, sizeof(asset_name_buf), ImGuiInputTextFlags_ReadOnly);

						ImGui::PopItemWidth();
					}

					ImGui::EndDisabled();

					ImGui::PopStyleVar();
					ImGui::PopStyleColor();

					// Drag target
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Standard) {
								asset_handle = dropped_asset_handle;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Mesh Renderer Material Target.");
							}
						}

						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {

							std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
							std::filesystem::path dropped_asset_path = dropped_asset_path_string;

							if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {

								AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path);

								if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Standard) {
									asset_handle = dropped_asset_handle;
								}
								else {
									L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
								}
							}
							else {
								L_APP_WARN("Invalid File Path Dropped on Skybox Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					if (ImGui::Button("...")) {
						L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR MATERIALS!");
					}

					ImGui::NextColumn();
					i++;
				}

				ImGui::Columns(1);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<AssetMeshRenderer>("Mesh Renderer Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<RigidbodyComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Rigidbody Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Rigidbody Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<RigidbodyComponent>();

			ImGui::Columns(2, "rb_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Mass");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			float value = component.GetMass();
			if (ImGui::DragFloat("##RB_Mass", &value, 0.05f, 0.0f, std::numeric_limits<float>::max(), "%.2f"))
				component.SetMass(value);
			ImGui::NextColumn();

			ImGui::Text("Drag");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			value = component.GetDrag();
			if (ImGui::DragFloat("##RB_Drag", &value, 0.05f, 0.0f, std::numeric_limits<float>::max(), "%.2f"))
				component.SetDrag(value);
			ImGui::NextColumn();

			ImGui::Text("Angular Drag");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			value = component.GetAngularDrag();
			if (ImGui::DragFloat("##RB_Angular Drag", &value, 0.05f, 0.0f, std::numeric_limits<float>::max(), "%.2f"))
				component.SetAngularDrag(value);
			ImGui::NextColumn();

			ImGui::Text("Automatic Centre of Mass");
			ImGui::NextColumn();
			bool value_2 = component.IsAutomaticCentreOfMassEnabled();
			if (ImGui::Checkbox("##RB_Automatic Centre of Mass", &value_2))
				component.SetAutomaticCentreOfMass(value_2);
			ImGui::NextColumn();

			ImGui::Text("Use Gravity");
			ImGui::NextColumn();
			value_2 = component.IsGravityEnabled();
			if (ImGui::Checkbox("##RB_Gravity", &value_2))
				component.SetGravity(value_2);
			ImGui::NextColumn();

			ImGui::Text("Is Kinematic");
			ImGui::NextColumn();
			value_2 = component.IsKinematicEnabled();
			if (ImGui::Checkbox("##RB_Kinematic", &value_2))
				component.SetKinematic(value_2);
			ImGui::NextColumn();

			ImGui::Columns(1);

			if (ImGui::TreeNodeEx("Constraints", tree_node_flags)) {

				ImGui::Columns(2, "rb_constraints", false);
				ImGui::Text("Freeze Position");
				ImGui::NextColumn();

				glm::bvec3 value_3 = component.GetPositionConstraint();
				if (ImGui::Checkbox("X", &value_3.x))
					component.SetPositionConstraint(value_3);

				ImGui::SameLine();
				if (ImGui::Checkbox("Y", &value_3.y))
					component.SetPositionConstraint(value_3);

				ImGui::SameLine();
				if (ImGui::Checkbox("Z", &value_3.z))
					component.SetPositionConstraint(value_3);

				ImGui::NextColumn();
				ImGui::Text("Freeze Rotation");
				ImGui::NextColumn();

				value_3 = component.GetRotationConstraint();
				if (ImGui::Checkbox("X", &value_3.x))
					component.SetRotationConstraint(value_3);

				ImGui::SameLine();
				if (ImGui::Checkbox("Y", &value_3.y))
					component.SetRotationConstraint(value_3);

				ImGui::SameLine();
				if (ImGui::Checkbox("Z", &value_3.z))
					component.SetRotationConstraint(value_3);

				ImGui::NextColumn();

				ImGui::Columns(1);

				ImGui::TreePop();
			}


			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<RigidbodyComponent>("Rigidbody Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<SphereColliderComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Sphere Collider Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Sphere Collider Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<SphereColliderComponent>();

			ImGui::Columns(2, "sc_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Radius");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			float value = component.GetRadius();
			if (ImGui::DragFloat("##SC_Radius", &value, 0.05f, 0.0f, std::numeric_limits<float>::max(), "%.2f"))
				component.SetRadius(value);
			ImGui::NextColumn();

			ImGui::Text("Is Trigger");
			ImGui::NextColumn();
			bool value_2 = component.IsTrigger();
			if (ImGui::Checkbox("##SC_Is Trigger", &value_2))
				component.SetIsTrigger(value_2);
			ImGui::NextColumn();

			ImGui::Text("Centre");
			ImGui::NextColumn();

			// Get the width of the current column
			float columnWidth = ImGui::GetColumnWidth() - (ImGui::GetStyle().ItemSpacing.x * 6 + ImGui::CalcTextSize("X").x * 3); // Account for spacing

			glm::vec3 value_3 = component.GetCentre();
			bool updated = false;

			ImGui::Text("X");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionX", &value_3.x, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Y");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionY", &value_3.y, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Z");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionZ", &value_3.z, 0.01f, 0, 0, "%.2f")) updated = true;

			if (updated) component.SetCentre(value_3);

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<SphereColliderComponent>("Sphere Collider Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<BoxColliderComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Box Collider Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Box Collider Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<BoxColliderComponent>();

			ImGui::Columns(2, "bc_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Is Trigger");
			ImGui::NextColumn();
			bool value_2 = component.IsTrigger();
			if (ImGui::Checkbox("##SC_Is Trigger", &value_2))
				component.SetIsTrigger(value_2);
			ImGui::NextColumn();

			ImGui::Text("Centre");
			ImGui::NextColumn();

			// Get the width of the current column
			float columnWidth = ImGui::GetColumnWidth() - (ImGui::GetStyle().ItemSpacing.x * 6 + ImGui::CalcTextSize("X").x * 3); // Account for spacing

			glm::vec3 value_3 = component.GetCentre();
			bool updated = false;

			ImGui::Text("X");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionX", &value_3.x, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Y");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionY", &value_3.y, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Z");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionZ", &value_3.z, 0.01f, 0, 0, "%.2f")) updated = true;

			if (updated) component.SetCentre(value_3);

			ImGui::NextColumn();
			ImGui::Text("Size");
			ImGui::NextColumn();

			value_3 = component.GetSize();
			updated = false;

			ImGui::Text("X");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local SizeX", &value_3.x, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Y");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local SizeY", &value_3.y, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Z");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local SizeZ", &value_3.z, 0.01f, 0, 0, "%.2f")) updated = true;

			if (updated) component.SetSize(value_3);

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<BoxColliderComponent>("Box Collider Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<PointLightComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Point Light Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Point Light Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<PointLightComponent>();

			ImGui::Columns(2, "point_light_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Active");
			ImGui::NextColumn();
			ImGui::Checkbox("##ActiveCheckBox", &component.Active);
			ImGui::NextColumn();

			ImGui::Text("Radius");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##PointLightRadius", &component.Radius, 0.05f, 0.0f, std::numeric_limits<float>::max(), "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Intensity");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##PointLightIntensity", &component.Intensity, 0.05f, 0.0f, std::numeric_limits<float>::max(), "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Colour");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::ColorEdit4("##PointLightColour", glm::value_ptr(component.Colour));
			ImGui::NextColumn();

			static std::array<const char*, 3> shadow_types = { "No Shadows", "Hard Shadows", "Soft Shadows" };
			uint8_t item_current = static_cast<uint8_t>(component.ShadowFlag);
			ImGui::Text("Shadow Type");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##PointShadowType", shadow_types[item_current])) {

				for (int n = 0; n < shadow_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(shadow_types[n], is_selected))
					{
						item_current = n;
						component.ShadowFlag = static_cast<ShadowTypeFlag>(item_current);
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<PointLightComponent>("Point Light Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<SpotLightComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Spot Light Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Spot Light Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<SpotLightComponent>();

			ImGui::Columns(2, "spot_light_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Active");
			ImGui::NextColumn();
			ImGui::Checkbox("##ActiveCheckBox", &component.Active);
			ImGui::NextColumn();

			ImGui::Text("Angle");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::SliderFloat("##SpotLightAngle", &component.Angle, 1.0f, 179.0f, "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Range");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##SpotLightRange", &component.Range, 0.5f, 0.0f, std::numeric_limits<float>::max(), "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Intensity");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##SpotLightIntensity", &component.Intensity, 0.5f, 0.0f, std::numeric_limits<float>::max(), "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Colour");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::ColorEdit4("##SpotLightColour", glm::value_ptr(component.Colour));
			ImGui::NextColumn();

			static std::array<const char*, 3> shadow_types = { "No Shadows", "Hard Shadows", "Soft Shadows" };
			uint8_t item_current = static_cast<uint8_t>(component.ShadowFlag);
			ImGui::Text("Shadow Type");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##SpotShadowType", shadow_types[item_current])) {

				for (int n = 0; n < shadow_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(shadow_types[n], is_selected))
					{
						item_current = n;
						component.ShadowFlag = static_cast<ShadowTypeFlag>(item_current);
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<SpotLightComponent>("Spot Light Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<DirectionalLightComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Directional Light Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Directional Light Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<DirectionalLightComponent>();

			ImGui::Columns(2, "directional_light_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Active");
			ImGui::NextColumn();
			ImGui::Checkbox("##ActiveCheckBox", &component.Active);
			ImGui::NextColumn();

			ImGui::Text("Intensity");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##DirectionalLightIntensity", &component.Intensity, 0.5f, 0.0f, std::numeric_limits<float>::max(), "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Colour");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::ColorEdit4("##DirectionalLightColour", glm::value_ptr(component.Colour));
			ImGui::NextColumn();

			static std::array<const char*, 3> shadow_types = { "No Shadows", "Hard Shadows", "Soft Shadows" };
			uint8_t item_current = static_cast<uint8_t>(component.ShadowFlag);
			ImGui::Text("Shadow Type");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##DirectionalShadowType", shadow_types[item_current])) {

				for (int n = 0; n < shadow_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(shadow_types[n], is_selected))
					{
						item_current = n;
						component.ShadowFlag = static_cast<ShadowTypeFlag>(item_current);
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<DirectionalLightComponent>("Directional Light Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	// Material Inspector
	if (!material_list.empty()) {

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Text("Materials Inspector");
		ImGui::Dummy({ 0.0f, 5.0f });

		for (const auto& asset_handle : material_list) {

			auto& metadata_material = Project::GetStaticEditorAssetManager()->GetMetadata(asset_handle);

			switch (metadata_material.Type) {

			case AssetType::Material_Standard:
			{
				if (!Project::GetStaticEditorAssetManager()->IsAssetHandleValid(asset_handle))
					continue;

				auto asset_material = Project::GetStaticEditorAssetManager()->GetAsset<PBRMaterial>(asset_handle);
				if (!asset_material)
					continue;

				// TODO: implement serialisation for custom materials, probably need content browser first though lol
				// Toggle the != to == if you want to edit materials from an imported asset that are not saved to their own file
				bool immutable_material = metadata_material.FilePath.extension() != ".lmat";
				bool material_modified = false;

				if (ImGui::TreeNode(std::string("Material: " + metadata_material.AssetName).c_str())) {

					ImGui::Dummy({ 0.0f, 5.0f });

					if (immutable_material) {
						ImGui::BeginDisabled();
					}

					GLuint texture_id = Project::GetStaticEditorAssetManager()->IsAssetHandleValid(asset_material->GetAlbedoTextureAssetHandle()) ? Project::GetStaticEditorAssetManager()->GetAsset<Texture>(asset_material->GetAlbedoTextureAssetHandle())->GetID() : 0;

					// Texture and text alignment
					ImGui::ImageButton("##Albedo Texture", (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });

					if (texture_id == 0 && ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Text("Invalid Asset");
						ImGui::EndTooltip();
					}

					// Drag target
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
								asset_material->SetAlbedoTexture(dropped_asset_handle);
								material_modified = true;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();

					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize("Albedo Texture").y / 2.0f));
					ImGui::Text("Albedo Texture");

					// ColorEdit4 button size adjustment
					glm::vec4 colour = asset_material->GetAlbedoTintColour();

					// Size of font impacts size of ColorEdit4 button, and we want this to be uniform to the ImageButton, so we add FramePadding similar to how ImageButton does this internally
					ImGuiContext& context = *ImGui::GetCurrentContext();
					float padding = (32.0f - context.FontSize) / 2.0f + context.Style.FramePadding.y;

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, padding));
					if (ImGui::ColorEdit4("##Albedo Colour", glm::value_ptr(colour), ImGuiColorEditFlags_NoInputs))
					{
						asset_material->SetAlbedoTintColour(colour);
						material_modified = true;
					}
					ImGui::PopStyleVar();

					ImGui::SameLine();

					ImGui::Text("Albedo Colour");

					texture_id = Project::GetStaticEditorAssetManager()->IsAssetHandleValid(asset_material->GetMetallicTextureAssetHandle()) ? Project::GetStaticEditorAssetManager()->GetAsset<Texture>(asset_material->GetMetallicTextureAssetHandle())->GetID() : 0;

					ImGui::ImageButton("##Metallic Texture", (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });

					if (texture_id == 0 && ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Text("Invalid Asset");
						ImGui::EndTooltip();
					}

					// Drag target
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
								asset_material->SetMetallicTexture(dropped_asset_handle);
								material_modified = true;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize("Metallic Texture").y / 2.0f));
					ImGui::Text("Metallic Texture");

					if (texture_id == 0) {
						ImGui::Text("Metallic");
						ImGui::SameLine();
						float metallic_temp = asset_material->GetMetallic();
						if (ImGui::SliderFloat("##Metallic", &metallic_temp, 0.0f, 1.0f, "%.2f"))
						{
							asset_material->SetMetallic(metallic_temp);
							material_modified = true;
						}
					}

					ImGui::Text("Roughness");
					ImGui::SameLine();
					float roughness_temp = asset_material->GetRoughness();
					if (ImGui::SliderFloat("##Roughness", &roughness_temp, 0.0f, 1.0f, "%.2f")) 
					{
						asset_material->SetRoughness(roughness_temp);
						material_modified = true;
					}

					texture_id = Project::GetStaticEditorAssetManager()->IsAssetHandleValid(asset_material->GetNormalTextureAssetHandle()) ? Project::GetStaticEditorAssetManager()->GetAsset<Texture>(asset_material->GetNormalTextureAssetHandle())->GetID() : 0;

					ImGui::ImageButton("##Normal Texture", (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });

					if (texture_id == 0 && ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Text("Invalid Asset");
						ImGui::EndTooltip();
					}

					// Drag target
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
								asset_material->SetNormalTexture(dropped_asset_handle);
								material_modified = true;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize("Normal Texture").y / 2.0f));
					ImGui::Text("Normal Texture");

					if (immutable_material) {
						ImGui::EndDisabled();
					}

					if (material_modified)
					{
						YAML::Emitter out;
						out << YAML::BeginMap;
						asset_material->Serialize(out);
						out << YAML::EndMap;

						std::ofstream fout(Project::GetActiveProject()->GetProjectDirectory() / "Assets" / metadata_material.FilePath); // Create the file
						fout << out.c_str();
					}

					ImGui::TreePop();
				}

				break;
			}

			case AssetType::Material_Skybox:
			{
				if (!Project::GetStaticEditorAssetManager()->IsAssetHandleValid(asset_handle))
					continue;

				auto asset_material = Project::GetStaticEditorAssetManager()->GetAsset<SkyboxMaterial>(asset_handle);

				if (ImGui::TreeNode(std::string("Skybox Material: " + metadata_material.AssetName).c_str())) {

					ImGui::Dummy({ 0.0f, 5.0f });

					auto& sb_texture_asset_handles = asset_material->GetTextureAssetHandles();

					for (int i = 0; i < sb_texture_asset_handles.size(); i++) {


						GLuint texture_id = Project::GetStaticEditorAssetManager()->IsAssetHandleValid(sb_texture_asset_handles[i]) ? Project::GetStaticEditorAssetManager()->GetAsset<Texture>(sb_texture_asset_handles[i])->GetID() : 0;

						static std::array<std::string, 6> skybox_binding_names = {
							"Right",
							"Left",
							"Top",
							"Bottom",
							"Back",
							"Front"
						};

						// Texture and text alignment
						std::string label = "##" + skybox_binding_names[i] + "_Texture";
						ImGui::ImageButton(label.c_str(), (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });

						if (texture_id == 0 && ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("Invalid Asset");
							ImGui::EndTooltip();
						}

						// Drag target
						if (ImGui::BeginDragDropTarget()) {
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
								AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

								if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
									asset_material->SetSkyboxFaceTexture(static_cast<L_SKYBOX_BINDING>(i), dropped_asset_handle);
									asset_material->Serialize();
								}
								else {
									L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
								}
							}

							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {

								std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
								std::filesystem::path dropped_asset_path = dropped_asset_path_string;

								if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {

									AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path);

									if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
										asset_material->SetSkyboxFaceTexture(static_cast<L_SKYBOX_BINDING>(i), dropped_asset_handle);
										asset_material->Serialize();
									}
									else {
										L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
									}
								}
								else {
									L_APP_WARN("Invalid File Path Dropped on Skybox Material Target.");
								}
							}
							ImGui::EndDragDropTarget();
						}

						ImGui::SameLine();

						std::string label_text = skybox_binding_names[i] + " Texture";
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize(label_text.c_str()).y / 2.0f));
						ImGui::Text(label_text.c_str());
					}


					ImGui::TreePop();
				}

				break;
			}

			}


			ImGui::Dummy({ 0.0f, 5.0f });
		}
	}

	ImGui::EndChild();

	DisplayEntitySelectionModal(selected_entity);
}

static bool modal_box_open = false;
static ScriptFieldType modal_box_field_type = ScriptFieldType::None;
static std::string modal_box_script_name = "";
static std::string modal_box_field_name = "";
static Entity modal_box_selected_entity{};

void PropertiesPanel::DisplayScriptFields(const std::string& script_name, Entity selected_entity)
{
	auto scene_ref = Project::GetActiveScene();
	bool scriptClassExists = ScriptManager::EntityClassExists(script_name);

#pragma region Display Script Fields

	if (scene_ref->IsRunning()) {

		if (auto instance = ScriptManager::GetEntityScriptInstance(selected_entity.GetUUID(), script_name); instance) {

			const auto& fields = instance->GetScriptClass()->GetFields();

			float first_coloumn_width = ImGui::GetContentRegionAvail().x * 0.35f;
			ImGui::Columns(2, "script_field_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			for (const auto& [name, field] : fields)
			{
				switch (field.Type) {

				case ScriptFieldType::Prefab:
				{

					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					AssetHandle data = instance->GetFieldPrefabValue(name);
					std::string label = "##" + name + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type));

					AssetMetaData meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(data);
					std::string text = (meta_data.AssetName != "") ? meta_data.AssetName + (" (Prefab)") : ("None (Prefab)");

					char buffer[256];
					strcpy_s(buffer, sizeof(buffer), text.c_str());
					ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					if (ImGui::BeginDragDropTarget()) {

						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {
							// Convert the payload data (string) back into a filesystem path
							std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize - 1);
							std::filesystem::path dropped_path = dropped_path_str; // Convert to path

							if (AssetType asset_type = AssetManager::GetAssetTypeFromFileExtension(dropped_path.extension()); asset_type != AssetType::None) {

								AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_path);

								switch (asset_type) {

								case AssetType::Prefab:
								case AssetType::ModelImport:
								{
									instance->SetFieldPrefabValue(ScriptManager::GetScriptFieldMap(selected_entity.GetUUID(), script_name).at(name), dropped_asset_handle);

									break;
								}
								default: {

									L_APP_WARN("Cannot Set Asset Type {} to Script Prefab Field.", dropped_path.extension().string());
									break;
								}

								}

							}
							else {
								L_APP_WARN("Cannot Set Prefab {} to Script.", dropped_path.filename().string());
							}

						}
						ImGui::EndDragDropTarget();
					}

					ImGui::NextColumn();


					break;
				}
				case ScriptFieldType::Entity:
				case ScriptFieldType::TransformComponent:
				case ScriptFieldType::TagComponent:
				case ScriptFieldType::ScriptComponent:
				case ScriptFieldType::PointLightComponent:
				case ScriptFieldType::SpotLightComponent:
				case ScriptFieldType::DirectionalLightComponent:
				case ScriptFieldType::RigidbodyComponent:
				case ScriptFieldType::BoxColliderComponent:
				case ScriptFieldType::SphereColliderComponent:
				case ScriptFieldType::Component:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					Louron::UUID data = (field.Type == ScriptFieldType::Entity) ? instance->GetFieldEntityValue(name) : instance->GetComponentPropertyValue(name);
					std::string label = "##" + name + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type));
					std::string text = (scene_ref->HasEntityByUUID(data)) ? scene_ref->FindEntityByUUID(data).GetName().c_str() : ("None (" + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type)) + ")");

					char buffer[256];
					strcpy_s(buffer, sizeof(buffer), text.c_str());
					ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
					{
						modal_box_open = true;
						modal_box_field_name = name;
						modal_box_field_type = field.Type;
						modal_box_script_name = script_name;
					}

					ImGui::SameLine();

					if (ImGui::Button(std::string("...##" + name + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type))).c_str()))
					{
						modal_box_open = true;
						modal_box_field_name = name;
						modal_box_field_type = field.Type;
						modal_box_script_name = script_name;
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Bool: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					bool data = instance->GetFieldValue<bool>(name);
					if (ImGui::Checkbox(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &data)) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Byte: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint8_t data = instance->GetFieldValue<uint8_t>(name);
					int temp = static_cast<int>(data);
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &temp, 1, 0, 255)) {
						data = static_cast<uint8_t>(temp);
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Sbyte: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int8_t data = instance->GetFieldValue<int8_t>(name);
					int temp = static_cast<int>(data);
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &temp, 1, -128, 127)) {
						data = static_cast<int8_t>(temp);
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Char: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					char data = instance->GetFieldValue<char>(name);
					char buffer[2] = { data, '\0' };
					if (ImGui::InputText(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), buffer, sizeof(buffer))) {
						data = buffer[0];
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Decimal:
				case ScriptFieldType::Double: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					double data = instance->GetFieldValue<double>(name);
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_Double, &data, 0.02f, (const void*)0, (const void*)0, "%.2f")) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Float: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					float data = instance->GetFieldValue<float>(name);
					if (ImGui::DragFloat(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &data, 0.02f, 0.0f, 0.0f, "%.2f")) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Int: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int data = instance->GetFieldValue<int>(name);
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &data)) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Uint: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint32_t data = instance->GetFieldValue<uint32_t>(name);
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_U32, &data)) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Long: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int64_t data = instance->GetFieldValue<int64_t>(name);
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_S64, &data)) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Ulong: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint64_t data = instance->GetFieldValue<uint64_t>(name);
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_U64, &data)) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Short: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int16_t data = instance->GetFieldValue<int16_t>(name);
					int temp = static_cast<int>(data);
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &temp, 1, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max())) {
						data = static_cast<int16_t>(temp);
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Ushort: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint16_t data = instance->GetFieldValue<uint16_t>(name);
					int temp = static_cast<int>(data);
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &temp, 1, 0, std::numeric_limits<uint16_t>::max())) {
						data = static_cast<uint16_t>(temp);
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector2: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					glm::vec2 data = instance->GetFieldValue<glm::vec2>(name);
					if (ImGui::DragFloat2(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), glm::value_ptr(data), 0.02f, 0.0f, 0.0f, "%.2f")) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector3: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					glm::vec3 data = instance->GetFieldValue<glm::vec3>(name);
					if (ImGui::DragFloat3(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), glm::value_ptr(data), 0.02f, 0.0f, 0.0f, "%.2f")) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector4: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					glm::vec4 data = instance->GetFieldValue<glm::vec4>(name);
					if (ImGui::DragFloat4(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), glm::value_ptr(data), 0.02f, 0.0f, 0.0f, "%.2f")) {
						instance->SetFieldValue(name, data);
					}

					ImGui::NextColumn();
					break;
				}
				default:
					// Placeholder for unsupported or custom types
					break;
				}

			}

			ImGui::Columns(1);

		}
	}
	else if (scriptClassExists) {

		std::shared_ptr<ScriptClass> entityClass = ScriptManager::GetEntityClass(script_name);
		const auto& fields = entityClass->GetFields();

		auto& entityFields = ScriptManager::GetScriptFieldMap(selected_entity.GetUUID(), script_name);

		float first_coloumn_width = ImGui::GetContentRegionAvail().x * 0.35f;
		ImGui::Columns(2, "script_field_columns", false);
		ImGui::SetColumnWidth(-1, first_coloumn_width);

		for (const auto& [name, field] : fields)
		{
			// Field has been set in editor
			if (entityFields.find(name) != entityFields.end())
			{
				ScriptFieldInstance& scriptField = entityFields.at(name);
				switch (field.Type) {

				case ScriptFieldType::Prefab:
				{

					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					AssetHandle data = scriptField.GetValue<AssetHandle>();
					std::string label = "##" + name + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type));

					AssetMetaData meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(data);

					std::string text = (meta_data.AssetName != "") ? meta_data.AssetName + (" (Prefab)") : ("None (Prefab)");

					char buffer[256];
					strcpy_s(buffer, sizeof(buffer), text.c_str());
					ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					if (ImGui::BeginDragDropTarget()) {

						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {
							// Convert the payload data (string) back into a filesystem path
							std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize - 1);
							std::filesystem::path dropped_path = dropped_path_str; // Convert to path

							if (AssetType asset_type = AssetManager::GetAssetTypeFromFileExtension(dropped_path.extension()); asset_type != AssetType::None) {

								AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_path);

								switch (asset_type) {

									case AssetType::Prefab:
									case AssetType::ModelImport:
									{
										ScriptFieldInstance& scriptField = entityFields.at(name);
										scriptField.SetValue<AssetHandle>(dropped_asset_handle);

										break;
									}
									default: {

										L_APP_WARN("Cannot Set Asset Type {} to Script Prefab Field.", dropped_path.extension().string());
										break;
									}

								}

							}
							else {
								L_APP_WARN("Cannot Set Prefab {} to Script.", dropped_path.filename().string());
							}

						}
						ImGui::EndDragDropTarget();
					}

					ImGui::NextColumn();


					break;
				}
				case ScriptFieldType::Entity:
				case ScriptFieldType::TransformComponent:
				case ScriptFieldType::TagComponent:
				case ScriptFieldType::ScriptComponent:
				case ScriptFieldType::PointLightComponent:
				case ScriptFieldType::SpotLightComponent:
				case ScriptFieldType::DirectionalLightComponent:
				case ScriptFieldType::RigidbodyComponent:
				case ScriptFieldType::BoxColliderComponent:
				case ScriptFieldType::SphereColliderComponent:
				case ScriptFieldType::Component:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					Louron::UUID data = scriptField.GetValue<Louron::UUID>();
					std::string label = "##" + name + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type));
					std::string text = (scene_ref->HasEntityByUUID(data)) ? scene_ref->FindEntityByUUID(data).GetName().c_str() : ("None (" + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type)) + ")");

					char buffer[256];
					strcpy_s(buffer, sizeof(buffer), text.c_str());
					ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
					{
						modal_box_open = true;
						modal_box_field_name = name;
						modal_box_field_type = field.Type;
						modal_box_script_name = script_name;
					}

					ImGui::SameLine();

					if (ImGui::Button(std::string("...##" + name + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type))).c_str()))
					{
						modal_box_open = true;
						modal_box_field_name = name;
						modal_box_field_type = field.Type;
						modal_box_script_name = script_name;
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Bool: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					bool data = scriptField.GetValue<bool>();
					if (ImGui::Checkbox(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &data)) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Byte: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint8_t data = scriptField.GetValue<uint8_t>();
					int temp = static_cast<int>(data);
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &temp, 1, 0, 255)) {
						data = static_cast<uint8_t>(temp);
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Sbyte: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int8_t data = scriptField.GetValue<int8_t>();
					int temp = static_cast<int>(data);
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &temp, 1, -128, 127)) {
						data = static_cast<int8_t>(temp);
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Char: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					char data = scriptField.GetValue<char>();
					char buffer[2] = { data, '\0' };
					if (ImGui::InputText(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), buffer, sizeof(buffer))) {
						scriptField.SetValue(buffer[0]);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Decimal:
				case ScriptFieldType::Double: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					double data = scriptField.GetValue<double>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_Double, &data, 0.02f, (const void*)0, (const void*)0, "%.2f")) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Float: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					float data = scriptField.GetValue<float>();
					if (ImGui::DragFloat(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &data, 0.02f, 0.0f, 0.0f, "%.2f")) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Int: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int data = scriptField.GetValue<int>();
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &data)) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Uint: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint32_t data = scriptField.GetValue<uint32_t>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_U32, &data)) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Long: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int64_t data = scriptField.GetValue<int64_t>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_S64, &data)) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Ulong: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint64_t data = scriptField.GetValue<uint64_t>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_U64, &data)) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Short: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int16_t data = scriptField.GetValue<int16_t>();
					int temp = static_cast<int>(data);
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &temp, 1, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max())) {
						data = static_cast<int16_t>(temp);
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Ushort: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint16_t data = scriptField.GetValue<uint16_t>();
					int temp = static_cast<int>(data);
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &temp, 1, 0, std::numeric_limits<uint16_t>::max())) {
						data = static_cast<uint16_t>(temp);
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector2: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					glm::vec2 data = scriptField.GetValue<glm::vec2>();
					if (ImGui::DragFloat2(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), glm::value_ptr(data), 0.02f, 0.0f, 0.0f, "%.2f")) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector3: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					glm::vec3 data = scriptField.GetValue<glm::vec3>();
					if (ImGui::DragFloat3(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), glm::value_ptr(data), 0.02f, 0.0f, 0.0f, "%.2f")) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector4: {
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					glm::vec4 data = scriptField.GetValue<glm::vec4>();
					if (ImGui::DragFloat4(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), glm::value_ptr(data), 0.02f, 0.0f, 0.0f, "%.2f")) {
						scriptField.SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}

				default:
					// Placeholder for unsupported or custom types
					break;
				}

			}
			// Field has not been edited, get default value from the ScriptFieldType
			else
			{
				switch (field.Type)
				{

				case ScriptFieldType::Prefab:
				{

					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					AssetHandle data = field.GetInitialValue<AssetHandle>();
					std::string label = "##" + name + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type));

					AssetMetaData meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(data);

					std::string text = (meta_data.AssetName != "") ? meta_data.AssetName + (" (Prefab)") : ("None (Prefab)");

					char buffer[256];
					strcpy_s(buffer, sizeof(buffer), text.c_str());
					ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					if (ImGui::BeginDragDropTarget()) {
						
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {
							// Convert the payload data (string) back into a filesystem path
							std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize - 1);
							std::filesystem::path dropped_path = dropped_path_str; // Convert to path

							if (AssetType asset_type = AssetManager::GetAssetTypeFromFileExtension(dropped_path.extension()); asset_type != AssetType::None) {

								AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_path);

								switch (asset_type) {

									case AssetType::Prefab:
									case AssetType::ModelImport:
									{

										ScriptFieldInstance& scriptField = entityFields[name];
										scriptField.Field = fields.at(name);
										scriptField.SetValue<AssetHandle>(dropped_asset_handle);

										break;
									}
									default: {

										L_APP_WARN("Cannot Set Asset Type {} to Script Prefab Field.", dropped_path.extension().string());
										break;
									}

								}

							}
							else {
								L_APP_WARN("Cannot Set Prefab {} to Script.", dropped_path.filename().string());
							}

						}
						ImGui::EndDragDropTarget();
					}

					ImGui::NextColumn();


					break;
				}
				case ScriptFieldType::Entity:
				case ScriptFieldType::TransformComponent:
				case ScriptFieldType::TagComponent:
				case ScriptFieldType::ScriptComponent:
				case ScriptFieldType::PointLightComponent:
				case ScriptFieldType::SpotLightComponent:
				case ScriptFieldType::DirectionalLightComponent:
				case ScriptFieldType::RigidbodyComponent:
				case ScriptFieldType::BoxColliderComponent:
				case ScriptFieldType::SphereColliderComponent:
				case ScriptFieldType::Component:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					Louron::UUID data = field.GetInitialValue<Louron::UUID>();
					std::string label = "##" + name + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type));
					std::string text = (scene_ref->HasEntityByUUID(data)) ? scene_ref->FindEntityByUUID(data).GetName().c_str() : ("None (" + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type)) + ")");

					char buffer[256];
					strcpy_s(buffer, sizeof(buffer), text.c_str());
					ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
					{
						modal_box_open = true;
						modal_box_field_name = name;
						modal_box_field_type = field.Type;
						modal_box_script_name = script_name;
					}

					ImGui::SameLine();

					if (ImGui::Button(std::string("...##" + name + std::string(ScriptingUtils::ScriptFieldTypeToString(field.Type))).c_str()))
					{
						modal_box_open = true;
						modal_box_field_name = name;
						modal_box_field_type = field.Type;
						modal_box_script_name = script_name;
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Bool:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					bool data = field.GetInitialValue<bool>();
					if (ImGui::Checkbox(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &data))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<bool>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Byte:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint8_t data = field.GetInitialValue<uint8_t>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_U8, &data))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<uint8_t>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Sbyte:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int8_t data = field.GetInitialValue<int8_t>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_S8, &data))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<int8_t>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Char:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					char data = field.GetInitialValue<char>();
					char buffer[2] = { data, '\0' };
					if (ImGui::InputText(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), buffer, sizeof(buffer)))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<char>(buffer[0]);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Decimal:
				case ScriptFieldType::Double:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					double data = field.GetInitialValue<double>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_Double, &data, 0.02f, (const void*)0, (const void*)0, "%.2f"))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<double>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Float:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					float data = field.GetInitialValue<float>();
					if (ImGui::DragFloat(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &data, 0.02f, 0.0f, 0.0f, "%.2f"))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<float>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Int:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int data = field.GetInitialValue<int>();
					if (ImGui::DragInt(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), &data))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<int>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Uint:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					unsigned int data = field.GetInitialValue<unsigned int>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_U32, &data))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<unsigned int>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Long:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int64_t data = field.GetInitialValue<int64_t>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_S64, &data))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<int64_t>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Ulong:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint64_t data = field.GetInitialValue<uint64_t>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_U64, &data))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<uint64_t>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Short:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					int16_t data = field.GetInitialValue<int16_t>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_S16, &data))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<int16_t>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Ushort:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					uint16_t data = field.GetInitialValue<uint16_t>();
					if (ImGui::DragScalar(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), ImGuiDataType_U16, &data))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<uint16_t>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector2:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					glm::vec2 data = field.GetInitialValue<glm::vec2>();
					if (ImGui::DragFloat2(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), glm::value_ptr(data), 0.02f, 0.0f, 0.0f, "%.2f"))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<glm::vec2>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector3:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					glm::vec3 data = field.GetInitialValue<glm::vec3>();
					if (ImGui::DragFloat3(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), glm::value_ptr(data), 0.02f, 0.0f, 0.0f, "%.2f"))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<glm::vec3>(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector4:
				{
					ImGui::Text(name.c_str());
					ImGui::NextColumn();

					glm::vec4 data = field.GetInitialValue<glm::vec4>();
					if (ImGui::DragFloat4(std::string("##" + name + ScriptingUtils::ScriptFieldTypeToString(field.Type)).c_str(), glm::value_ptr(data), 0.02f, 0.0f, 0.0f, "%.2f"))
					{
						ScriptFieldInstance& fieldInstance = entityFields[name];
						fieldInstance.Field = field;
						fieldInstance.SetValue<glm::vec4>(data);
					}

					ImGui::NextColumn();
					break;
				}
				default:
					break;
				}

			}
		}

		ImGui::Columns(1);
	}

#pragma endregion

}

void PropertiesPanel::DisplayEntitySelectionModal(Entity& selected_entity)
{
	auto scene_ref = Project::GetActiveScene();
	bool scriptClassExists = ScriptManager::EntityClassExists(modal_box_script_name);

#pragma region Entity Selection w/ Component Type

	if (modal_box_open) {
		ImGui::OpenPopup("Select Entity");

		// Calculate window size: 25% of the screen width, minimum 300.0f
		float windowWidth = glm::max(600.0f, ImGui::GetContentRegionAvail().x * 0.25f);
		float windowHeight = glm::max(600.0f, ImGui::GetContentRegionAvail().y * 0.25f); // Fixed height
		ImVec2 windowSize(windowWidth, windowHeight);
		ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	}

	auto set_entity_value = [&](Entity& entity) {

		if (scene_ref->IsRunning()) {

			if (auto instance = ScriptManager::GetEntityScriptInstance(selected_entity.GetUUID(), modal_box_script_name); instance) {

				instance->SetFieldEntityValue(ScriptManager::GetScriptFieldMap(selected_entity.GetUUID(), modal_box_script_name).at(modal_box_field_name), entity.GetUUID());

				modal_box_open = false;
				modal_box_field_type = ScriptFieldType::None;
				modal_box_script_name = "";
				modal_box_field_name = "";
				modal_box_selected_entity = {};
				return;
			}

		}
		else if (scriptClassExists) {

			const auto& fields = ScriptManager::GetEntityClass(modal_box_script_name)->GetFields();
			auto& entityFields = ScriptManager::GetScriptFieldMap(selected_entity.GetUUID(), modal_box_script_name);

			// Field has been set in editor
			if (entityFields.find(modal_box_field_name) != entityFields.end())
			{
				ScriptFieldInstance& scriptField = entityFields.at(modal_box_field_name);
				scriptField.SetValue(entity.GetUUID());

				modal_box_open = false;
				modal_box_field_type = ScriptFieldType::None;
				modal_box_script_name = "";
				modal_box_field_name = "";
				modal_box_selected_entity = {};
				return;
			}
			// Field has not been edited, get default value from the ScriptFieldType
			else {
				ScriptFieldInstance& scriptField = entityFields[modal_box_field_name];
				scriptField.Field = fields.at(modal_box_field_name);
				scriptField.SetValue<Louron::UUID>(entity.GetUUID());

				modal_box_open = false;
				modal_box_field_type = ScriptFieldType::None;
				modal_box_script_name = "";
				modal_box_field_name = "";
				modal_box_selected_entity = {};
				return;
			}

		}
	};

	if (ImGui::BeginPopupModal("Select Entity", &modal_box_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {

		std::vector<Entity> available_entities{};
		switch (modal_box_field_type) {

		case ScriptFieldType::TagComponent:
		case ScriptFieldType::TransformComponent:
		case ScriptFieldType::Entity: {

			auto view = scene_ref->GetAllEntitiesWith<TransformComponent, TagComponent>();

			for (auto entity_handle : view) {
				available_entities.push_back({ entity_handle, scene_ref.get() });
			}

			break;
		}
		case ScriptFieldType::ScriptComponent: {

			auto view = scene_ref->GetAllEntitiesWith<ScriptComponent>();

			for (auto entity_handle : view) {
				available_entities.push_back({ entity_handle, scene_ref.get() });
			}

			break;
		}
		case ScriptFieldType::PointLightComponent: {

			auto view = scene_ref->GetAllEntitiesWith<PointLightComponent>();

			for (auto entity_handle : view) {
				available_entities.push_back({ entity_handle, scene_ref.get() });
			}

			break;
		}
		case ScriptFieldType::SpotLightComponent: {

			auto view = scene_ref->GetAllEntitiesWith<SpotLightComponent>();

			for (auto entity_handle : view) {
				available_entities.push_back({ entity_handle, scene_ref.get() });
			}

			break;
		}
		case ScriptFieldType::DirectionalLightComponent: {

			auto view = scene_ref->GetAllEntitiesWith<DirectionalLightComponent>();

			for (auto entity_handle : view) {
				available_entities.push_back({ entity_handle, scene_ref.get() });
			}

			break;
		}
		case ScriptFieldType::RigidbodyComponent: {

			auto view = scene_ref->GetAllEntitiesWith<RigidbodyComponent>();

			for (auto entity_handle : view) {
				available_entities.push_back({ entity_handle, scene_ref.get() });
			}

			break;
		}
		case ScriptFieldType::BoxColliderComponent: {

			auto view = scene_ref->GetAllEntitiesWith<BoxColliderComponent>();

			for (auto entity_handle : view) {
				available_entities.push_back({ entity_handle, scene_ref.get() });
			}

			break;
		}
		case ScriptFieldType::SphereColliderComponent: {

			auto view = scene_ref->GetAllEntitiesWith<SphereColliderComponent>();

			for (auto entity_handle : view) {
				available_entities.push_back({ entity_handle, scene_ref.get() });
			}

			break;
		}

		}

		const float footer_height = 40.0f;
		const ImVec2 child_size(0, ImGui::GetContentRegionAvail().y - footer_height); // Adjust the height based on your preference
		ImGui::BeginChild("EntitySelectableList", child_size, true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		{
			// ImGui selectable list
			for (auto& entity : available_entities) {
				const std::string& entity_name = entity.GetName();

				// Check if this entity is the selected one
				bool isSelected = modal_box_selected_entity == entity;

				if (ImGui::Selectable(std::string(" - " + entity_name).c_str(), isSelected)) {
					modal_box_selected_entity = entity;
				}

				// Handle double click
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
					set_entity_value(entity);
					break;
				}
			}
		}
		ImGui::EndChild();

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - footer_height);
		ImGui::BeginChild("EntitySelectableFooterBar", ImVec2(0, footer_height), false);
		{

			// Calculate the position for the button to be in the middle-right of the FooterBar
			static ImVec2 button_size = ImGui::CalcTextSize(" Select Entity ");
			float footer_middle_y = button_size.y * 0.5f;

			ImGui::SetCursorPosY(footer_middle_y);  // Center the button vertically within the footer

			// Calculate the position for the button to be in the middle-right of the FooterBar
			float content_region_width = ImGui::GetContentRegionAvail().x;
			ImGui::SetCursorPosX(content_region_width - button_size.x - 10.0f);  // 10.0f for some padding from the right edge

			if (!modal_box_selected_entity)
				ImGui::BeginDisabled();

			if (ImGui::Button("Select Entity", ImVec2(button_size.x, 0)))
			{
				set_entity_value(modal_box_selected_entity);
			}
			else if (!modal_box_selected_entity)
					ImGui::EndDisabled();
		}

		ImGui::EndChild();

		ImGui::EndPopup();
	}


#pragma endregion

}
