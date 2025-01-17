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

	std::function<void(Entity)> DrawEntity = [&](Entity entity) {

		std::string tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;

		flags |= ((uint32_t)selected_entity == (uint32_t)entity) ? ImGuiTreeNodeFlags_Selected : 0;

		if (entity.GetComponent<HierarchyComponent>().GetChildren().empty()) {
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen;
		}
		else {
			flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			tag += " (Children: " + std::to_string(entity.GetComponent<HierarchyComponent>().GetChildren().size()) + ")";
		}

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked()) {
			selected_entity = entity;
		}

		// Check for double-click
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			// Call LookAtGlobalPosition with the entity's global position

			glm::vec3 position_to_look_at{};

			// If it is a mesh I'd like to look at the mesh, some meshes have AABBs that are offset from their true transform
			if (entity.HasComponent<AssetMeshFilter>() && entity.HasComponent<AssetMeshRenderer>()) {
				position_to_look_at = entity.GetComponent<AssetMeshFilter>().TransformedAABB.Center();
				scene_ref->GetPrimaryCameraEntity().GetComponent<CameraComponent>().CameraInstance->LookAtGlobalPosition(position_to_look_at);
			}
			else if (entity.GetUUID() != scene_ref->GetPrimaryCameraEntity().GetUUID()) {
				// Make sure we don't try to look at ourselves lol
				position_to_look_at = entity.GetComponent<TransformComponent>().GetGlobalPosition();
				scene_ref->GetPrimaryCameraEntity().GetComponent<CameraComponent>().CameraInstance->LookAtGlobalPosition(position_to_look_at);
			}

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

		if (opened && !entity.GetComponent<HierarchyComponent>().GetChildren().empty()) {

			// Drag source
			if (!isDraggingEntity && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

				isDraggingEntity = true;

				// Set the payload to carry the entity UUID
				auto uuid = entity.GetUUID();

				ImGui::SetDragDropPayload("ENTITY_UUID", &uuid, sizeof(uuid));
				ImGui::EndDragDropSource();
			}

			// Drag target
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_UUID")) {
					Louron::UUID droppedEntityUUID = *(const Louron::UUID*)payload->Data;
					Entity droppedEntity = scene_ref->FindEntityByUUID(droppedEntityUUID);

					if (droppedEntity) {
						// Attach the dropped entity as a child to the current entity
						droppedEntity.GetComponent<HierarchyComponent>().AttachParent(entity.GetUUID());
					}
					isDraggingEntity = false;
				}
				ImGui::EndDragDropTarget();
			}

			for (const auto& child : entity.GetComponent<HierarchyComponent>().GetChildren())
				DrawEntity(scene_ref->FindEntityByUUID(child));

			ImGui::TreePop();
		}

		// Drag source
		if (!isDraggingEntity && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

			isDraggingEntity = true;

			// Set the payload to carry the entity UUID
			auto uuid = entity.GetUUID();

			ImGui::SetDragDropPayload("ENTITY_UUID", &uuid, sizeof(uuid));
			ImGui::EndDragDropSource();
		}

		// Drag target
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_UUID")) {
				Louron::UUID droppedEntityUUID = *(const Louron::UUID*)payload->Data;
				Entity droppedEntity = scene_ref->FindEntityByUUID(droppedEntityUUID);

				if (droppedEntity) {
					// Attach the dropped entity as a child to the current entity
					droppedEntity.GetComponent<HierarchyComponent>().AttachParent(entity.GetUUID());
				}
				isDraggingEntity = false;
			}
			ImGui::EndDragDropTarget();
		}
		};

	// Right-click on blank space
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem("Create Empty Entity"))
			selected_entity = scene_ref->CreateEntity();

		if (ImGui::MenuItem("Create Cube"))
		{
			if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(3119409521))
			{
				auto prefab = Project::GetStaticEditorAssetManager()->GetAsset<Prefab>(3119409521);
				selected_entity = scene_ref->InstantiatePrefab(prefab);
			}
		}

		if (ImGui::MenuItem("Create Sphere"))
		{
			if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(1835682225))
			{
				auto prefab = Project::GetStaticEditorAssetManager()->GetAsset<Prefab>(1835682225);
				selected_entity = scene_ref->InstantiatePrefab(prefab);
			}
		}

		ImGui::EndPopup();
	}

	scene_ref->GetRegistry()->each([&](auto entityID) {
		Entity root_entity = { entityID , scene_ref.get() };

		if (root_entity.HasComponent<HierarchyComponent>()) {

			if (!root_entity.GetComponent<HierarchyComponent>().HasParent()) {
				DrawEntity(root_entity);
			}
		}
		});

	if (createChildOnCurrentEntity) {
		selected_entity = scene_ref->CreateEntity();
		selected_entity.GetComponent<HierarchyComponent>().AttachParent(parent_UUID);

		parent_UUID = false;
		createChildOnCurrentEntity = false;
	}

	if (entityToDelete) {
		scene_ref->DestroyEntity(entityToDelete);
		entityToDelete = {};
	}

	ImGui::EndChild();

	// Check if no item was clicked and we clicked in the child window
	if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered()) {
		// Clicked on blank space
		selected_entity = {}; // Deselect entity or handle click as needed
	}

	if (ImGui::IsKeyDown(ImGuiKey_Delete) && selected_entity) {
		scene_ref->DestroyEntity(selected_entity);
		selected_entity = {};
	}

	// Detect dropping on empty space
	if (ImGui::BeginDragDropTarget()) {

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

		ImGui::EndDragDropTarget();
	}

}
