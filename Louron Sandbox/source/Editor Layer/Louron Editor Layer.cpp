#include "Louron Editor Layer.h"

#include <glm/gtx/string_cast.hpp>

#include <imgui/imgui_internal.h>

using namespace Louron;

LouronEditorLayer::LouronEditorLayer() {
	L_APP_INFO("Opening Main Menu");
}

void LouronEditorLayer::OnAttach() {
	Engine::Get().GetWindow().SetVSync(false);
	
	Project::LoadProject("Sandbox Project/Sandbox Project.lproj");
	auto scene = Project::GetActiveScene();

	FrameBufferConfig fbo_config;
	fbo_config.Width = m_ViewportWindowSize.x;
	fbo_config.Height = m_ViewportWindowSize.y;
	fbo_config.RenderToScreen = false;
	fbo_config.Samples = 1;

	scene->CreateSceneFrameBuffer(fbo_config);

	scene->OnStart();

	m_ActiveGUIWindows = {

		{ "Scene", true },
		{ "Scene Stats", true },

		{ "Scene Hierarchy", true },
		{ "Properties Panel", true },

		{ "Content Browser", true },

		{ "Render Stats", true },
		{ "Profiler", true },

		{ "Asset Registry", false }

	};

}

void LouronEditorLayer::OnDetach() {

	Project::GetActiveScene()->OnStop();
}

void LouronEditorLayer::OnUpdate() {


	if (auto scene_ref = Project::GetActiveScene(); scene_ref) {

		scene_ref->OnViewportResize(m_ViewportWindowSize);

		if(m_SceneWindowFocused) {

			// Update Camera Component
			Entity camera_entity = scene_ref->GetPrimaryCameraEntity();

			if (camera_entity) {
				camera_entity.GetComponent<CameraComponent>().CameraInstance->Update(Time::Get().GetDeltaTime());
				camera_entity.GetComponent<Transform>().SetPosition(camera_entity.GetComponent<CameraComponent>().CameraInstance->GetGlobalPosition());
				camera_entity.GetComponent<Transform>().SetGlobalForwardDirection(camera_entity.GetComponent<CameraComponent>().CameraInstance->GetCameraDirection());
			}

			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		}
		else {
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		scene_ref->OnUpdate();
	}
	else {

		glm::vec4 colour(1.0f);

		colour.r = (std::sin(static_cast<float>(Time::Get().GetCurrTime()) * 0.1f + 0.0f) + 1.0f) * 0.5f;
		colour.g = (std::sin(static_cast<float>(Time::Get().GetCurrTime()) * 0.1f + 2.0f) + 1.0f) * 0.5f;
		colour.b = (std::sin(static_cast<float>(Time::Get().GetCurrTime()) * 0.1f + 4.0f) + 1.0f) * 0.5f;

		Renderer::ClearColour(colour);
		Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void LouronEditorLayer::OnFixedUpdate() {

	if (auto scene_ref = Project::GetActiveScene(); scene_ref) {
		scene_ref->OnFixedUpdate();
	}
}

void LouronEditorLayer::OnGuiRender() {
	static bool opt_show_demo_window = false;
	if(opt_show_demo_window)
		ImGui::ShowDemoWindow(&opt_show_demo_window);

	// This is pretty much just the code from ImGui::ShowExampleAppDockSpace and 
	// ImGui::DockSpaceOverViewport demo example to setup the dockspace
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static bool opt_show_options = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// Setup dockspace central window
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else {
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if(ImGui::Begin("Louron Editor Dockspace Window", NULL, window_flags))
	{
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("Louron Editor Dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar()) {

			if (ImGui::BeginMenu("File")) {

				if (ImGui::MenuItem("New Project")) {
					Project::SetActiveProject(Project::NewProject());
					m_SelectedEntity = {};
				}

				if (ImGui::MenuItem("Open Project")) {
					std::string filepath = FileUtils::OpenFile("Louron Project (*.lproj)\0*.lproj\0");
					if (!filepath.empty()) {
						Project::LoadProject(filepath);
						m_SelectedEntity = {};
					}
				}

				if (ImGui::MenuItem("Save Project")) {
					Project::SaveProject();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene"))
				{
					std::string filepath = FileUtils::SaveFile("Louron Scene (*.lscene)\0*.lscene\0");
					if (!filepath.empty()) {

						m_SelectedEntity = {};

						Project::GetActiveScene()->OnStop();

						auto scene = Project::GetActiveProject()->NewScene(std::filesystem::relative(filepath, Project::GetActiveProject()->GetConfig().AssetDirectory));
						
						FrameBufferConfig fbo_config;
						fbo_config.Width = m_ViewportWindowSize.x;
						fbo_config.Height = m_ViewportWindowSize.y;
						fbo_config.RenderToScreen = false;
						fbo_config.Samples = 1;

						scene->CreateSceneFrameBuffer(fbo_config);

						scene->OnStart();

						Project::SetActiveScene(scene);
					}
				}

				if (ImGui::MenuItem("Open Scene")) {

					std::string filepath = FileUtils::OpenFile("Louron Scene (*.lscene)\0*.lscene\0");
					if (!filepath.empty()) {

						auto scene = Project::GetActiveProject()->LoadScene(std::filesystem::relative(filepath, Project::GetActiveProject()->GetConfig().AssetDirectory));

						if (scene) {

							m_SelectedEntity = {};

							Project::GetActiveScene()->OnStop();

							Project::SetActiveScene(scene);

							FrameBufferConfig fbo_config;
							fbo_config.Width = 1;
							fbo_config.Height = 1;
							fbo_config.RenderToScreen = false;
							fbo_config.Samples = 1;

							scene->CreateSceneFrameBuffer(fbo_config);

							scene->OnStart();

						}
						else {
							L_APP_ERROR("Could Not Load Scene.");
						}
					}
				}

				if (ImGui::MenuItem("Save Scene")) {
					Project::GetActiveProject()->SaveScene();
				}

				if (ImGui::MenuItem("Save Scene As"))
				{
					std::string filepath = FileUtils::SaveFile("Louron Scene (*.lscene)\0*.lscene\0");
					if (!filepath.empty()) {
						Project::GetActiveScene()->SetSceneFilePath(filepath);
						Project::GetActiveProject()->SaveScene();
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Reload Shaders")) {
					Engine::Get().GetShaderLibrary().ReloadAllShaders();
				}

				ImGui::Separator();

				ImGui::MenuItem("Show Docking Options", NULL, &opt_show_options);
				ImGui::MenuItem("Show ImGui Demo", NULL, &opt_show_demo_window);

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
					Engine::Get().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window")) {

				for (const auto& pair : m_ActiveGUIWindows) {

					if (ImGui::MenuItem(pair.first)) {
						m_ActiveGUIWindows[pair.first] = true;
					}
				}

				ImGui::EndMenu();
			}

			if (opt_show_options) {
				if (ImGui::BeginMenu("Options")) {
					ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
					ImGui::MenuItem("Padding", NULL, &opt_padding);
					ImGui::Separator();

					if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
					if (ImGui::MenuItem("Flag: NoDockingSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
					if (ImGui::MenuItem("Flag: NoUndocking", "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
					if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
					if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
					if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }

					ImGui::Separator();

					ImGui::EndMenu();
				}
			}


			ImGui::EndMenuBar();
		}

		DisplayViewportWindow();
		DisplayHierarchyWindow();
		DisplayPropertiesWindow();
		DisplayContentBrowserWindow();
		DisplayRenderStatsWindow();
		DisplayProfilerWindow();
		DisplayAssetRegistryWindow();

	}
	ImGui::End();
}

void LouronEditorLayer::DisplayViewportWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Scene"]) {
		return;
	}

	// Render Our ViewPort
	if (ImGui::Begin("Scene", &m_ActiveGUIWindows["Scene"], 0)) {

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportWindowSize = { viewportPanelSize.x, viewportPanelSize.y };

		auto scene_ref = Project::GetActiveScene();

		if (scene_ref) {
			ImGui::Image((ImTextureID)(uintptr_t)scene_ref->GetSceneFrameBuffer()->GetTexture(FrameBufferTexture::ColourTexture), ImGui::GetContentRegionAvail(), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		}
		else {
			ImGui::Image(0, ImGui::GetContentRegionAvail());
			ImGui::End();
			return;
		}

		if (m_SceneWindowFocused != ImGui::IsWindowFocused()) {
			m_SceneWindowFocused = ImGui::IsWindowFocused();
			scene_ref->GetPrimaryCameraEntity().GetComponent<CameraComponent>().CameraInstance->MouseToggledOff = m_SceneWindowFocused;
		}

		// ----- Draw Simple FPS Counter -----
		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoDocking;

		ImVec2 scene_window_pos = ImGui::GetWindowPos();
		ImVec2 scene_window_size = ImGui::GetWindowSize();
		ImVec2 fps_overlay_size = ImVec2(100.0f, 50.0f);
		ImVec2 fps_overlay_pos = ImVec2(
			scene_window_pos.x + scene_window_size.x - fps_overlay_size.x - 10.0f,  // 10.0f is a margin
			scene_window_pos.y + scene_window_size.y - fps_overlay_size.y - 10.0f   // 10.0f is a margin
		);

		ImGui::SetNextWindowSize(fps_overlay_size, ImGuiCond_Always);
		ImGui::SetNextWindowPos(fps_overlay_pos, ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.35f);
		if (ImGui::Begin("Scene Stats", &m_ActiveGUIWindows["Scene Stats"], window_flags))
		{
			ImGui::Text("FPS Counter");
			ImGui::Separator();
			ImGui::Text("%.0f", ImGui::GetIO().Framerate);

		}
		ImGui::End();

		auto [mx, my] = ImGui::GetMousePos();
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();

		glm::vec2 viewportBounds[2]; 
		viewportBounds[0] = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
		viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		mx -= viewportBounds[0].x;
		my -= viewportBounds[0].y;

		glm::vec2 viewportSize = viewportBounds[1] - viewportBounds[0];

		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{

			if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
				uint32_t pixelData = Project::GetActiveScene()->GetSceneFrameBuffer()->ReadEntityPixelData({ mouseX, mouseY });
				m_SelectedEntity = pixelData == NULL_UUID ? Entity() : Project::GetActiveScene()->FindEntityByUUID(pixelData);
			}
		}
	}
	ImGui::End();
}

void LouronEditorLayer::DisplayHierarchyWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Scene Hierarchy"]) {
		return;
	}

	ImGui::Begin("Scene Hierarchy", &m_ActiveGUIWindows["Scene Hierarchy"], 0);

	auto scene_ref = Project::GetActiveScene();

	if (!scene_ref) {
		ImGui::End();
		return;
	}

	// We do this so that we can accept a drag drop on 
	// the entirety of the window if we are looking to 
	// remove all parents to the entity.
	ImGui::BeginChild("##"); 

	bool isDraggingEntity = false;
	Entity entityToDelete = {};

	std::function<void(Entity)> DrawEntity = [&](Entity entity) {

		std::string tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags =  ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;

		flags |= ((uint32_t)m_SelectedEntity == (uint32_t)entity) ? ImGuiTreeNodeFlags_Selected : 0;

		if (entity.GetComponent<HierarchyComponent>().GetChildren().empty()) {
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}
		else {
			flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			tag += " (Children: " + std::to_string(entity.GetComponent<HierarchyComponent>().GetChildren().size()) + ")";
		}

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_SelectedEntity = entity;
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
			else if(entity.GetUUID() != scene_ref->GetPrimaryCameraEntity().GetUUID()) { 
				// Make sure we don't try to look at ourselves lol
				position_to_look_at = entity.GetComponent<Transform>().GetGlobalPosition();
				scene_ref->GetPrimaryCameraEntity().GetComponent<CameraComponent>().CameraInstance->LookAtGlobalPosition(position_to_look_at);
			}

		}

		if (ImGui::BeginPopupContextItem())
		{

			if (ImGui::MenuItem("Create Child Entity")) {
				m_SelectedEntity = scene_ref->CreateEntity();
				m_SelectedEntity.GetComponent<HierarchyComponent>().AttachParent(entity.GetUUID());
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
					UUID droppedEntityUUID = *(const UUID*)payload->Data;
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
				UUID droppedEntityUUID = *(const UUID*)payload->Data;
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
			m_SelectedEntity = scene_ref->CreateEntity();

		ImGui::EndPopup();
	}

	scene_ref->GetRegistry()->each([&](auto entityID) {
		Entity root_entity = { entityID , scene_ref.get() };

		if(root_entity.HasComponent<HierarchyComponent>()) {

			if (!root_entity.GetComponent<HierarchyComponent>().HasParent()) {
				DrawEntity(root_entity);
			}
		}
	});

	if (entityToDelete) {
		scene_ref->DestroyEntity(entityToDelete);
		entityToDelete = {};
	}
		
	ImGui::EndChild();

	// Check if no item was clicked and we clicked in the child window
	if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered()) {
		// Clicked on blank space
		m_SelectedEntity = {}; // Deselect entity or handle click as needed
	}

	if (ImGui::IsKeyDown(ImGuiKey_Delete) && m_SelectedEntity) {
		scene_ref->DestroyEntity(m_SelectedEntity);
		m_SelectedEntity = {};
	}

	// Detect dropping on empty space
	if (ImGui::BeginDragDropTarget()) {

		// Temporarily change the highlight color
		//ImVec4 prevColor = ImGui::GetStyle().Colors[ImGuiCol_DragDropTarget];
		ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(0, 0, 0, 0));

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_UUID")) {
			UUID droppedEntityUUID = *(const UUID*)payload->Data;
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

	ImGui::End();
}

void LouronEditorLayer::DisplayPropertiesWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Properties Panel"]) {
		return;
	}

	ImGui::Begin("Properties Panel", &m_ActiveGUIWindows["Properties Panel"], 0);

	auto scene_ref = Project::GetActiveScene();
	if (!m_SelectedEntity || !scene_ref) {
		ImGui::End();
		return;
	}

	std::vector<AssetHandle> material_list;

	ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen;

	// Right-click on blank space
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem("Add Camera Component")) {
			if (!m_SelectedEntity.HasComponent<CameraComponent>()) {
				auto& component = m_SelectedEntity.AddComponent<CameraComponent>();
				component.CameraInstance = std::make_shared<Camera>(m_SelectedEntity.GetComponent<Transform>().GetGlobalPosition());
				component.CameraInstance->UpdateProjMatrix({ scene_ref->GetSceneFrameBuffer()->GetConfig().Width, scene_ref->GetSceneFrameBuffer()->GetConfig().Height });
			}
		}

		if (ImGui::MenuItem("Add Skybox Component")) {
			if (!m_SelectedEntity.HasComponent<SkyboxComponent>())
				m_SelectedEntity.AddComponent<SkyboxComponent>();
		}

		if (ImGui::MenuItem("Add MeshFilter")) {
			if (!m_SelectedEntity.HasComponent<AssetMeshFilter>())
				m_SelectedEntity.AddComponent<AssetMeshFilter>();
		}

		if (ImGui::MenuItem("Add MeshRenderer")) {
			if (!m_SelectedEntity.HasComponent<AssetMeshRenderer>())
				m_SelectedEntity.AddComponent<AssetMeshRenderer>();
		}

		if (ImGui::MenuItem("Add Rigidbody")) {
			if (!m_SelectedEntity.HasComponent<Rigidbody>())
				m_SelectedEntity.AddComponent<Rigidbody>();
		}

		if (ImGui::MenuItem("Add Sphere Collider")) {
			if (!m_SelectedEntity.HasComponent<SphereCollider>())
				m_SelectedEntity.AddComponent<SphereCollider>();
		}

		if (ImGui::MenuItem("Add Box Collider")) {
			if (!m_SelectedEntity.HasComponent<BoxCollider>())
				m_SelectedEntity.AddComponent<BoxCollider>();
		}

		if (ImGui::MenuItem("Add Point Light Component")) {
			if (!m_SelectedEntity.HasComponent<PointLightComponent>())
				m_SelectedEntity.AddComponent<PointLightComponent>();
		}

		if (ImGui::MenuItem("Add Spot Light Component")) {
			if (!m_SelectedEntity.HasComponent<SpotLightComponent>())
				m_SelectedEntity.AddComponent<SpotLightComponent>();
		}

		if (ImGui::MenuItem("Add Directional Light Component")) {
			if (!m_SelectedEntity.HasComponent<DirectionalLightComponent>())
				m_SelectedEntity.AddComponent<DirectionalLightComponent>();
		}

		ImGui::EndPopup();
	}

	ImGui::BeginChild("##Immutable Components", {}, ImGuiChildFlags_AutoResizeY);

	float first_coloumn_width = ImGui::GetContentRegionAvail().x * 0.35f;

	if (m_SelectedEntity.HasComponent<TagComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::Text("Tag");
		auto& component = m_SelectedEntity.GetComponent<TagComponent>();

		char tag_buffer[256];
		strncpy_s(tag_buffer, component.Tag.c_str(), sizeof(tag_buffer));
		tag_buffer[sizeof(tag_buffer) - 1] = '\0'; // Ensure null-termination

		ImGui::SameLine();
		ImGui::InputText("##", tag_buffer, sizeof(tag_buffer), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::IsItemDeactivatedAfterEdit()) {
			// Update the tag if the input box is deactivated (Enter pressed or box loses focus)
			component.Tag = std::string(tag_buffer);
		}

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<Transform>()){

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::Text("Transform Component\n ");
		Transform& entity_transform = m_SelectedEntity.GetComponent<Transform>();

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
		if (ImGui::DragFloat("##Local PositionX", &value.x, 0.0f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local PositionY", &value.y, 0.0f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local PositionZ", &value.z, 0.0f, 0, 0, "%.2f")) updated = true;

		if (updated) entity_transform.SetPosition(value);

		ImGui::NextColumn();
		ImGui::Text("Rotation");
		ImGui::NextColumn();

		value = entity_transform.GetLocalRotation();
		updated = false;

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local RotationX", &value.x, 0.0f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local RotationY", &value.y, 0.0f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local RotationZ", &value.z, 0.0f, 0, 0, "%.2f")) updated = true;

		if (updated) entity_transform.SetRotation(value);

		ImGui::NextColumn();
		ImGui::Text("Scale");
		ImGui::NextColumn();

		value = entity_transform.GetLocalScale();
		updated = false;

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local ScaleX", &value.x, 0.0f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local ScaleY", &value.y, 0.0f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		if (ImGui::DragFloat("##Local ScaleZ", &value.z, 0.0f, 0, 0, "%.2f")) updated = true;

		if (updated) entity_transform.SetScale(value);

		ImGui::NextColumn();

		ImGui::Columns(1);

		ImGui::Dummy({ 0.0f, 5.0f });
	}

	ImGui::EndChild();

	ImGui::Separator();

	ImGui::BeginChild("##Mutable Components", {}, ImGuiChildFlags_AutoResizeY);

	if (m_SelectedEntity.HasComponent<CameraComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		bool selected = false;

		ImGui::BeginChild("##CamerComponentChild", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Camera Component", tree_node_flags)) {

			auto& component = m_SelectedEntity.GetComponent<CameraComponent>();

			ImGui::Columns(2, "camera_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("FOV");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::DragFloat("##CameraFOV", &component.CameraInstance->FOV, 0.05f, 0.1f, std::numeric_limits<float>::max(), "%.2f"))
				component.CameraInstance->UpdateProjMatrix({ scene_ref->GetSceneFrameBuffer()->GetConfig().Width, scene_ref->GetSceneFrameBuffer()->GetConfig().Height });
			ImGui::NextColumn();

			ImGui::Text("Near");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::DragFloat("##CameraNear", &component.CameraInstance->NearDistance, 0.05f, 0.1f, std::numeric_limits<float>::max(), "%.2f"))
				component.CameraInstance->UpdateProjMatrix({ scene_ref->GetSceneFrameBuffer()->GetConfig().Width, scene_ref->GetSceneFrameBuffer()->GetConfig().Height });
			ImGui::NextColumn();

			ImGui::Text("Far");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::DragFloat("##CameraFar", &component.CameraInstance->FarDistance, 0.05f, 0.1f, std::numeric_limits<float>::max(), "%.2f"))
				component.CameraInstance->UpdateProjMatrix({ scene_ref->GetSceneFrameBuffer()->GetConfig().Width, scene_ref->GetSceneFrameBuffer()->GetConfig().Height });
			ImGui::NextColumn();

			ImGui::Text("Primary");
			ImGui::NextColumn();
			ImGui::Checkbox("##PrimaryCheckBox", &component.Primary);
			ImGui::NextColumn();

			std::array<const char*, 2> camera_clear_types = { "Colour Only", "Skybox" };
			uint8_t item_current = static_cast<uint8_t>(component.ClearFlags);
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

		ShowComponentContextPopup<CameraComponent>("Camera Component Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<SkyboxComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Skybox Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Skybox Component", tree_node_flags)) {

			auto& component = m_SelectedEntity.GetComponent<SkyboxComponent>();

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

					if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Skybox) {
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

		ShowComponentContextPopup<SkyboxComponent>("Skybox Component Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<AssetMeshFilter>()) {

		ImGui::Dummy({ 0.0f, 5.0f });
		
		ImGui::BeginChild("##Mesh Filter Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Mesh Filter Component", tree_node_flags)) {
			auto& component = m_SelectedEntity.GetComponent<AssetMeshFilter>();

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

		ShowComponentContextPopup<AssetMeshFilter>("Mesh Filter Component Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<AssetMeshRenderer>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Mesh Renderer Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Mesh Renderer Component", tree_node_flags)) {

			auto& component = m_SelectedEntity.GetComponent<AssetMeshRenderer>();

			if (component.MeshRendererMaterialHandles.empty()) {
				component.MeshRendererMaterialHandles.push_back(NULL_UUID);
			}

			for (const auto& asset_handle : component.MeshRendererMaterialHandles) {
				if (asset_handle != NULL_UUID) {
					material_list.push_back(asset_handle);
				}
			}

			if (ImGui::TreeNodeEx("Materials", tree_node_flags)) {

				ImGui::Dummy({ 0.0f, 5.0f });

				int i = 0;
				ImGui::Columns(2, "mesh_renderer_material_columns", false);
				ImGui::SetColumnWidth(-1, first_coloumn_width);
				for (const auto& asset_handle : component.MeshRendererMaterialHandles) {

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

						ImGui::InputText("##MeshFilterName", asset_name_buf, sizeof(asset_name_buf), ImGuiInputTextFlags_ReadOnly);

						ImGui::PopItemWidth();
					}

					ImGui::EndDisabled();

					ImGui::PopStyleVar();
					ImGui::PopStyleColor();

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

		ShowComponentContextPopup<AssetMeshRenderer>("Mesh Renderer Component Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<Rigidbody>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Rigidbody Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Rigidbody Component", tree_node_flags)) {

			auto& component = m_SelectedEntity.GetComponent<Rigidbody>();

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

		ShowComponentContextPopup<Rigidbody>("Rigidbody Component Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<SphereCollider>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Sphere Collider Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Sphere Collider Component", tree_node_flags)) {

			auto& component = m_SelectedEntity.GetComponent<SphereCollider>();

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

		ShowComponentContextPopup<SphereCollider>("Sphere Collider Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<BoxCollider>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Box Collider Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Box Collider Component", tree_node_flags)) {

			auto& component = m_SelectedEntity.GetComponent<BoxCollider>();

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

		ShowComponentContextPopup<BoxCollider>("Box Collider Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<PointLightComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Point Light Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Point Light Component", tree_node_flags)) {

			auto& component = m_SelectedEntity.GetComponent<PointLightComponent>();

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

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<PointLightComponent>("Point Light Component Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<SpotLightComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Spot Light Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Spot Light Component", tree_node_flags)) {

			auto& component = m_SelectedEntity.GetComponent<SpotLightComponent>();

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

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<SpotLightComponent>("Spot Light Component Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (m_SelectedEntity.HasComponent<DirectionalLightComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Directional Light Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Directional Light Component", tree_node_flags)) {

			auto& component = m_SelectedEntity.GetComponent<DirectionalLightComponent>();

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

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<DirectionalLightComponent>("Directional Light Component Options");

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	// Material Inspector
	if (!material_list.empty()) {

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Text("Materials Inspector");
		ImGui::Dummy({ 0.0f, 5.0f });

		for (const auto& asset_handle : material_list) {

			auto asset_material = Project::GetStaticEditorAssetManager()->GetAsset<PBRMaterial>(asset_handle);
			auto& metadata_material = Project::GetStaticEditorAssetManager()->GetMetadata(asset_handle);

			if (!asset_material)
				continue;

			// Toggle the != to == if you want to edit materials from an imported asset that are not saved to their own file
			bool immutable_material = metadata_material.FilePath.extension() == ".lmaterial";

			if (ImGui::TreeNode(std::string("Material: " + metadata_material.AssetName).c_str())) {

				ImGui::Dummy({ 0.0f, 5.0f });

				if (immutable_material) {
					ImGui::BeginDisabled();
				}

				GLuint texture_id = asset_material->GetAlbedoTextureAssetHandle() != NULL_UUID ? Project::GetStaticEditorAssetManager()->GetAsset<Texture>(asset_material->GetAlbedoTextureAssetHandle())->GetID() : 0;

				// Texture and text alignment
				ImGui::ImageButton("##Albedo Texture", (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });

				ImGui::SameLine();

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize("Albedo Texture").y / 2.0f));
				ImGui::Text("Albedo Texture");

				// ColorEdit4 button size adjustment
				glm::vec4 colour = asset_material->GetAlbedoTintColour();

				// Size of font impacts size of ColorEdit4 button, and we want this to be uniform to the ImageButton, so we add FramePadding similar to how ImageButton does this internally
				ImGuiContext& context = *ImGui::GetCurrentContext();
				float padding = (32.0f - context.FontSize) / 2.0f + context.Style.FramePadding.y; 

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, padding));
				ImGui::ColorEdit4("##Albedo Colour", glm::value_ptr(colour), ImGuiColorEditFlags_NoInputs);
				ImGui::PopStyleVar();

				ImGui::SameLine();

				ImGui::Text("Albedo Colour");

				asset_material->SetAlbedoTintColour(colour);

				texture_id = asset_material->GetMetallicTextureAssetHandle() != NULL_UUID ? Project::GetStaticEditorAssetManager()->GetAsset<Texture>(asset_material->GetMetallicTextureAssetHandle())->GetID() : 0;

				ImGui::ImageButton("##Metallic Texture", (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize("Metallic Texture").y / 2.0f));
				ImGui::Text("Metallic Texture");

				if (texture_id == 0) {
					ImGui::Text("Metallic");
					ImGui::SameLine();
					float metallic_temp = asset_material->GetMetallic();
					ImGui::SliderFloat("##Metallic", &metallic_temp, 0.0f, 1.0f, "%.2f");
					asset_material->SetMetallic(metallic_temp);
				}

				ImGui::Text("Roughness");
				ImGui::SameLine();
				float roughness_temp = asset_material->GetRoughness();
				ImGui::SliderFloat("##Roughness", &roughness_temp, 0.0f, 1.0f, "%.2f");
				asset_material->SetRoughness(roughness_temp);

				texture_id = asset_material->GetNormalTextureAssetHandle() != NULL_UUID ? Project::GetStaticEditorAssetManager()->GetAsset<Texture>(asset_material->GetNormalTextureAssetHandle())->GetID() : 0;

				ImGui::ImageButton("##Normal Texture", (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize("Normal Texture").y / 2.0f));
				ImGui::Text("Normal Texture");

				if (immutable_material) {
					ImGui::EndDisabled();
				}

				ImGui::TreePop();
			}

			ImGui::Dummy({ 0.0f, 5.0f });
		}
	}

	ImGui::EndChild();


	ImGui::End();
	
}

void LouronEditorLayer::DisplayContentBrowserWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Content Browser"]) {
		return;
	}
}

void LouronEditorLayer::DisplayRenderStatsWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Render Stats"]) {
		return;
	}

	if (ImGui::Begin("Render Stats", &m_ActiveGUIWindows["Render Stats"], 0)) {

		auto& stats = Renderer::GetFrameRenderStats();

		ImGui::SeparatorText("Low Level API Calls");
		ImGui::Text("Draw Calls: %i", stats.DrawCalls);

		ImGui::SeparatorText("Primitives");
		ImGui::Text("Total Vertice Count: %i", stats.Primitives_VerticeCount);
		ImGui::Text("Triangle Count: %i", stats.Primitives_TriangleCount);
		ImGui::Text("Line Count: %i", stats.Primitives_LineCount);

		ImGui::SeparatorText("Sub Meshes");
		ImGui::Text("Sub Meshes Renderered: %i", stats.SubMeshes_Rendered);
		ImGui::Text("Sub Meshes Instanced: %i", stats.SubMeshes_Instanced);
		ImGui::Text("Skyboxes Renderered: %i", stats.Skybox_Rendered);

		ImGui::SeparatorText("Frustum Culling");
		ImGui::Text("Entities Rendered: %i", stats.Entities_Rendered);
		ImGui::Text("Entities Culled: %i", stats.Entities_Culled);
	}
	ImGui::End();
}

void LouronEditorLayer::DisplayProfilerWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Profiler"]) {
		return;
	}

	if (ImGui::Begin("Profiler", &m_ActiveGUIWindows["Profiler"], 0)) {

		for (auto& result : Profiler::Get().GetResults()) {

			char label[128];
			strcpy_s(label, result.second.Name);
			strcat_s(label, " %.3fms");

			ImGui::Text(label, result.second.Time);
		}
	}
	ImGui::End();

}

void LouronEditorLayer::DisplayAssetRegistryWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Asset Registry"]) {
		return;
	}

	L_PROFILE_FUNCTION();

	if (ImGui::Begin("Asset Registry", &m_ActiveGUIWindows["Asset Registry"], 0)) {
		static char filter[128] = "";  // Buffer for the search filter
		static int currentPage = 0;    // Current page for pagination
		const int assetsPerPage = 20;  // Number of assets per page
		static AssetHandle selectedHandle = 0;  // Handle of the selected asset

		auto asset_manager = Project::GetStaticEditorAssetManager();

		// Input box for filtering assets
		ImGui::InputTextWithHint("##Filter", "Search Assets...", filter, IM_ARRAYSIZE(filter));
		ImGui::Text("Total Assets: %i", asset_manager->GetAssetRegistry().size());
		ImGui::SameLine();
		if(ImGui::Button("Refresh Asset Registry")) {
			Project::GetStaticEditorAssetManager()->RefreshAssetRegistry();
		}

		// Filter and collect assets that match the filter
		std::unordered_map<AssetHandle, AssetMetaData> filteredAssets;
		for (const auto& [handle, meta_data] : asset_manager->GetAssetRegistry()) {
			if (meta_data.AssetName.find(filter) != std::string::npos) {
				filteredAssets.emplace(handle, meta_data);
			}
		}

		// Calculate the total number of pages
		int totalPages = ((int)filteredAssets.size() + assetsPerPage - 1) / assetsPerPage;

		// Pagination controls
		if (ImGui::Button("Previous") && currentPage > 0) {
			--currentPage;
		}
		ImGui::SameLine();
		ImGui::Text("Page %i of %i", currentPage + 1, totalPages);
		ImGui::SameLine();
		if (ImGui::Button("Next") && currentPage < totalPages - 1) {
			++currentPage;
		}

		// Ensure current page is within valid range
		if (currentPage >= totalPages) {
			currentPage = totalPages - 1;
		}
		if (currentPage < 0) {
			currentPage = 0;
		}

		// Split the window
		float availableHeight = ImGui::GetContentRegionAvail().y;
		float listHeight = availableHeight * 0.75f;
		float detailsHeight = availableHeight - listHeight;

		ImGui::BeginChild("Asset List", ImVec2(0, listHeight), true);

		// Display assets for the current page
		int startIdx = currentPage * assetsPerPage;
		int endIdx = std::min(startIdx + assetsPerPage, static_cast<int>(filteredAssets.size()));

		int i = 0;
		for (const auto& [handle, metadata] : filteredAssets) {

			if (i < startIdx || i > endIdx) {
				i++;
				continue;
			}

			static char buf[64];

			std::memset(buf, 0, sizeof(buf));
			(metadata.AssetName + "##" + std::to_string(handle)).copy(buf, sizeof(buf), 0);

			if (ImGui::Selectable(buf, selectedHandle == handle)) {
				selectedHandle = handle;
			}
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

				ImGui::SetDragDropPayload("ASSET_HANDLE", &handle, sizeof(handle));
				ImGui::EndDragDropSource();

			}
			i++;
		}

		ImGui::EndChild();

		ImGui::Separator();

		ImGui::BeginChild("Asset Details", ImVec2(0, detailsHeight), true);

		// Display details of the selected asset
		if (selectedHandle != 0 && asset_manager->GetAssetRegistry().find(selectedHandle) != asset_manager->GetAssetRegistry().end()) {
			const auto& meta_data = asset_manager->GetAssetRegistry().at(selectedHandle);

			static char buf[256];

			std::memset(buf, 0, sizeof(buf));
			std::to_string(selectedHandle).copy(buf, sizeof(buf), 0);

			ImGui::Text("Asset UUID:");
			ImGui::SameLine();
			ImGui::InputText("##AssetUUID", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

				ImGui::SetDragDropPayload("ASSET_HANDLE", &selectedHandle, sizeof(selectedHandle));
				ImGui::EndDragDropSource();

			}

			std::memset(buf, 0, sizeof(buf));
			meta_data.AssetName.copy(buf, sizeof(buf), 0);

			ImGui::Text("Asset Name:");
			ImGui::SameLine();
			ImGui::InputText("##AssetName", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

			std::memset(buf, 0, sizeof(buf));
			AssetTypeToString(meta_data.Type).copy(buf, sizeof(buf), 0);

			ImGui::Text("Asset Type:");
			ImGui::SameLine();
			ImGui::InputText("##AssetType", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

			std::memset(buf, 0, sizeof(buf));
			if (meta_data.ParentAssetHandle != NULL_UUID)
				std::to_string(meta_data.ParentAssetHandle).copy(buf, sizeof(buf), 0);
			else
				std::string("None").copy(buf, sizeof(buf), 0);

			ImGui::Text("Asset Parent:");
			ImGui::SameLine();
			ImGui::InputText("##AssetParent", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

			std::memset(buf, 0, sizeof(buf));
			meta_data.FilePath.string().copy(buf, sizeof(buf), 0);

			ImGui::Text("Asset FilePath:");
			ImGui::SameLine();
			ImGui::InputText("##AssetFilePath", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
		}
		else {
			ImGui::Text("No asset selected.");
		}

		ImGui::EndChild();
	}
	ImGui::End();
	

}
