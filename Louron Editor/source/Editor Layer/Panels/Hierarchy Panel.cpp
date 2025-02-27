#include "Hierarchy Panel.h"

using namespace Louron;

void HierarchyPanel::OnImGuiRender(const std::shared_ptr<Louron::Scene>& scene_ref, Louron::Entity& selected_entity)
{
	// We do this so that we can accept a drag drop on 
	// the entirety of the window if we are looking to 
	// remove all parents to the entity.
	ImGui::BeginChild("##");

	bool isDraggingEntity = false;
	bool createChildOnCurrentEntity = false;
	Louron::UUID parent_UUID = -1;
	Entity entityToDelete = {};

	auto drop_target_content_file = [&](Louron::UUID parent_uuid) {

		// DROP A FILE FROM THE CONTENT BROWSER
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {
			// Convert the payload data (string) back into a filesystem path
			std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize - 1);
			std::filesystem::path dropped_path = dropped_path_str; // Convert to path

			if (AssetType asset_type = AssetManager::GetAssetTypeFromFileExtension(dropped_path.extension()); asset_type != AssetType::None) {

				AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_path, Project::GetActiveProject()->GetAssetDirectory());

				switch (asset_type) {

					case AssetType::ModelImport:
					case AssetType::Prefab:
					{
						auto prefab = AssetManager::GetAsset<Prefab>(dropped_asset_handle);

						if (!prefab) {
							L_APP_INFO("Attempting to Import New Asset.");

							AssetHandle asset_handle = Project::GetStaticEditorAssetManager()->ImportAsset(dropped_path, Project::GetActiveProject()->GetAssetDirectory());
							prefab = AssetManager::GetAsset<Prefab>(asset_handle);
						}

						if (prefab) {

							Entity entity = scene_ref->InstantiatePrefab(prefab);
							entity.GetComponent<TagComponent>().SetUniqueName(prefab->GetPrefabName());
							if (entity)
							{
								selected_entity = entity;

								if (parent_uuid != NULL_UUID)
									entity.GetComponent<HierarchyComponent>().AttachParent(parent_uuid);

								auto& transform = entity.GetTransform();
								transform.SetPosition({ 0.0f, 0.0f, 0.0f });
								transform.SetPosition({ 0.0f, 0.0f, 0.0f });
							}
							else
								L_APP_WARN("Could Not Instantiate Model Into Scene.");

						}

						break;
					}

				}

			}
			else {
				L_CORE_WARN("Cannot Instantiate '{}' Into Scene.", dropped_path.filename().string());
			}

		}
	};

	std::function<void(Entity)> DrawEntity = [&](Entity entity) {

		std::string tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;

		flags |= (selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0;

		if (entity.GetComponent<HierarchyComponent>().GetChildren().empty()) {
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen;
		}
		else {
			flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			tag += " (Children: " + std::to_string(entity.GetComponent<HierarchyComponent>().GetChildren().size()) + ")";
		}

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		static Entity clicked_entity = {};
		
		if (ImGui::IsItemClicked()) 
		{
			clicked_entity = entity;
		}

		// Check for double-click 
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) 
		{
			m_NewFocalEntity = entity;
		}

		if (ImGui::BeginPopupContextItem())
		{

			if (ImGui::MenuItem("Create Child Entity")) {
				createChildOnCurrentEntity = true;
				parent_UUID = entity.GetUUID();
			}

			if (ImGui::MenuItem("Delete Entity"))
				entityToDelete = entity;

			ImGui::EndPopup();
		}

		if (opened && !entity.GetComponent<HierarchyComponent>().GetChildren().empty()) 
		{

			// Drag source
			if (!isDraggingEntity && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) 
			{

				isDraggingEntity = true;
				clicked_entity = {};

				// Set the payload to carry the entity UUID
				auto uuid = entity.GetUUID();

				ImGui::SetDragDropPayload("ENTITY_UUID", &uuid, sizeof(uuid));
				ImGui::EndDragDropSource();
			}
			else if(!ImGui::IsMouseDown(ImGuiMouseButton_Left) && clicked_entity)
			{
				selected_entity = clicked_entity;
				clicked_entity = {};
			}

			// Drag target
			if (ImGui::BeginDragDropTarget()) 
			{
				
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_UUID")) 
				{
					Louron::UUID droppedEntityUUID = *(const Louron::UUID*)payload->Data;
					Entity droppedEntity = scene_ref->FindEntityByUUID(droppedEntityUUID);

					if (droppedEntity) {
						// Attach the dropped entity as a child to the current entity
						droppedEntity.GetComponent<HierarchyComponent>().AttachParent(entity.GetUUID());
					}
					isDraggingEntity = false;
				}

				drop_target_content_file(entity.GetUUID());

				ImGui::EndDragDropTarget();
			}

			for (const auto& child : entity.GetComponent<HierarchyComponent>().GetChildren())
				DrawEntity(scene_ref->FindEntityByUUID(child));

			ImGui::TreePop();
		}

		// Drag source
		if (!isDraggingEntity && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) 
		{

			isDraggingEntity = true;
			clicked_entity = {};

			// Set the payload to carry the entity UUID
			auto uuid = entity.GetUUID();

			ImGui::SetDragDropPayload("ENTITY_UUID", &uuid, sizeof(uuid));
			ImGui::EndDragDropSource();
		}
		else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && clicked_entity)
		{
			selected_entity = clicked_entity;
			clicked_entity = {};
		}

		// Drag target
		if (ImGui::BeginDragDropTarget()) 
		{
			
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_UUID")) 
			{
				Louron::UUID droppedEntityUUID = *(const Louron::UUID*)payload->Data;
				Entity droppedEntity = scene_ref->FindEntityByUUID(droppedEntityUUID);

				if (droppedEntity) {
					// Attach the dropped entity as a child to the current entity
					droppedEntity.GetComponent<HierarchyComponent>().AttachParent(entity.GetUUID());
				}
				isDraggingEntity = false;
			}

			drop_target_content_file(entity.GetUUID());

			ImGui::EndDragDropTarget();
		}
	};

	// Right-click on blank space
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem("Create Empty Entity"))
			selected_entity = scene_ref->CreateEntity();

		ImGui::Separator();

		if (ImGui::BeginMenu("Meshes"))
		{
			if (ImGui::MenuItem("Create Cube"))
			{
				if (auto prefab = AssetManager::GetInbuiltAsset<Prefab>("Cube", AssetType::ModelImport); prefab)
				{
					selected_entity = scene_ref->InstantiatePrefab(prefab);
				}
			}

			if (ImGui::MenuItem("Create Sphere"))
			{
				if (auto prefab = AssetManager::GetInbuiltAsset<Prefab>("Sphere", AssetType::ModelImport); prefab)
				{
					selected_entity = scene_ref->InstantiatePrefab(prefab);
				}
			}

			if (ImGui::MenuItem("Create Plane"))
			{
				if (auto prefab = AssetManager::GetInbuiltAsset<Prefab>("Plane", AssetType::ModelImport); prefab)
				{
					selected_entity = scene_ref->InstantiatePrefab(prefab);
				}
			}

			if (ImGui::MenuItem("Create Capsule"))
			{
				if (auto prefab = AssetManager::GetInbuiltAsset<Prefab>("Capsule", AssetType::ModelImport); prefab)
				{
					selected_entity = scene_ref->InstantiatePrefab(prefab);
				}
			}

			if (ImGui::MenuItem("Create Suzanne"))
			{
				if (auto prefab = AssetManager::GetInbuiltAsset<Prefab>("Suzanne", AssetType::ModelImport); prefab)
				{
					selected_entity = scene_ref->InstantiatePrefab(prefab);
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Lights"))
		{

			if (ImGui::MenuItem("Create Directional Light"))
			{
				Entity entity = scene_ref->CreateEntity("Directional Light");
				entity.AddComponent<DirectionalLightComponent>();
			}

			if (ImGui::MenuItem("Create Point Light"))
			{
				Entity entity = scene_ref->CreateEntity("Point Light");
				entity.AddComponent<PointLightComponent>();
			}

			if (ImGui::MenuItem("Create Spot Light"))
			{
				Entity entity = scene_ref->CreateEntity("Spot Light");
				entity.AddComponent<SpotLightComponent>();
			}

			ImGui::EndMenu();
		}


		ImGui::EndPopup();
	}

	scene_ref->GetRegistry()->each([&](auto entityID) 
	{
		Entity root_entity = { entityID , scene_ref.get() };

		if (root_entity.HasComponent<HierarchyComponent>()) {

			if (!root_entity.GetComponent<HierarchyComponent>().HasParent()) {
				DrawEntity(root_entity);
			}
		}
	});

	if (createChildOnCurrentEntity) 
	{
		selected_entity = scene_ref->CreateEntity();
		selected_entity.GetComponent<HierarchyComponent>().AttachParent(parent_UUID);

		parent_UUID = false;
		createChildOnCurrentEntity = false;
	}

	if (entityToDelete) 
	{
		scene_ref->DestroyEntity(entityToDelete);
		entityToDelete = {};
	}

	ImGui::EndChild();

	// Check if no item was clicked and we clicked in the child window
	if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered()) 
	{
		// Clicked on blank space
		selected_entity = {}; // Deselect entity or handle click as needed
	}

	if (ImGui::IsKeyDown(ImGuiKey_Delete) && selected_entity) 
	{
		scene_ref->DestroyEntity(selected_entity);
		selected_entity = {};
	}

	// Detect dropping on empty space
	if (ImGui::BeginDragDropTarget()) 
	{

		// Temporarily change the highlight color
		//ImVec4 prevColor = ImGui::GetStyle().Colors[ImGuiCol_DragDropTarget];
		ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(0, 0, 0, 0));

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_UUID")) {
			Louron::UUID droppedEntityUUID = *(const Louron::UUID*)payload->Data;
			Entity droppedEntity = scene_ref->FindEntityByUUID(droppedEntityUUID);

			if (droppedEntity) {
				// Remove parent to make the entity a root entity again
				droppedEntity.GetComponent<HierarchyComponent>().DetachParent();
			}
		}

		// Restore the previous highlight color
		ImGui::PopStyleColor();

		drop_target_content_file(NULL_UUID);

		ImGui::EndDragDropTarget();
	}

}
