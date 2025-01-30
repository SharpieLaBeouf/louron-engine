#include "Louron Editor Layer.h"

// Internal
#include "Utils/Editor Script Utils.h"

// C++ Headers

// External
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <imgui/imgui_internal.h>
#include <imguizmo/ImGuizmo.h>

using namespace Louron;

std::atomic_bool LouronEditorLayer::m_ScriptsNeedCompiling = false;
std::unique_ptr<filewatch::FileWatch<std::string>> LouronEditorLayer::m_ScriptFileWatcher = nullptr;

LouronEditorLayer::LouronEditorLayer() {
	L_APP_INFO("Opening Main Menu");
}

void LouronEditorLayer::OnAttach() {

	Engine::Get().GetWindow().SetVSync(false);
	
	Project::LoadProject("Sandbox Project/Sandbox Project.lproj");
	
	const EngineConfig& engine_config = Engine::Get().GetSpecification();

	// Parse arguments
	std::string lprojFilePath;
	for (int i = 0; i < engine_config.CommandLineArgs.Count; ++i) {
		std::string argument = engine_config.CommandLineArgs.Args[i];

		if (argument == "--project" && i + 1 < engine_config.CommandLineArgs.Count) {
			lprojFilePath = engine_config.CommandLineArgs.Args[++i];
		}
	}

	// 1. First we Load Project
	if(lprojFilePath.empty())
		Project::LoadProject("Sandbox Project/Sandbox Project.lproj");
	else
		Project::LoadProject(lprojFilePath);

	// 2. Then we have to load ScriptManager -> Script Manager needs a current project instance to be running
	ScriptManager::Init();

	// 3. Then we have to load startup scene -> ScriptComponent serialisation requires the ScriptManager to be initialised
	Project::GetActiveProject()->LoadStartupScene();

	for (const auto& entry : std::filesystem::directory_iterator(Project::GetActiveProject()->GetProjectDirectory() / "Scripts/")) {
		if (entry.path().extension() == ".csproj") {
			m_ScriptsCompiledSuccess = Utils::CompileAppAssembly(entry.path());
			if (m_ScriptsCompiledSuccess)
				ScriptManager::ReloadAssembly(); // Dont check if compilation succeeded because we need it to point the to updated assembly!
			break;
		}
	}

	auto scene = Project::GetActiveScene();
	//scene->SetDisplayOctree(true);

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

		{ "Asset Registry", false },

		{ "CreateNewProjectPopup", false },
		{ "CreateNewScenePopup", false },

		{ "ProjectProperties", false },
		{ "SceneProperties", false },
		{ "ScriptCompilationWarningMessage", false }

	};
	
	m_IconPlay = TextureImporter::LoadTexture2D("Resources/Icons/PlayButton.png");
	m_IconPause = TextureImporter::LoadTexture2D("Resources/Icons/PauseButton.png");
	m_IconSimulate = TextureImporter::LoadTexture2D("Resources/Icons/SimulateButton.png");
	m_IconStep = TextureImporter::LoadTexture2D("Resources/Icons/StepButton.png");
	m_IconStop = TextureImporter::LoadTexture2D("Resources/Icons/StopButton.png");

	m_ScriptsNeedCompiling.store(false, std::memory_order_relaxed);
	m_ScriptFileWatcher = std::make_unique<filewatch::FileWatch<std::string>>((Project::GetActiveProject()->GetProjectDirectory() / "Scripts").string(), ScriptsModifiedEvent);

	m_PropertiesPanel = {};
	m_HierarchyPanel = {};
	m_ContentBrowserPanel = {};
	m_ContentBrowserPanel.SetDirectory(Project::GetActiveProject()->GetProjectDirectory() / "Assets");

	m_EditorCamera = std::make_unique<EditorCamera>();
	m_EditorCamera->OnUpdate();
}

void LouronEditorLayer::OnDetach() {

	Project::GetActiveScene()->OnStop();
}

void LouronEditorLayer::OnUpdate() {
	
	if (m_SceneWindowFocused && m_SceneState == SceneState::Play) {
		glfwSetInputMode((GLFWwindow*)Engine::Get().GetWindow().GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}
	else {
		glfwSetInputMode((GLFWwindow*)Engine::Get().GetWindow().GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}


	if (auto scene_ref = Project::GetActiveScene(); scene_ref) {

		scene_ref->OnViewportResize(m_ViewportWindowSize);

		switch (m_SceneState) {

			case SceneState::Simulate:
			case SceneState::Edit: 
			{
				m_EditorCamera->SetViewportSize((float)m_ViewportWindowSize.x, (float)m_ViewportWindowSize.y);
				if (m_SceneWindowHovered) 
					m_EditorCamera->OnUpdate();
				scene_ref->OnUpdate(m_EditorCamera.get());
				break;
			}
			
			case SceneState::Play: scene_ref->OnUpdate(); break;

		}
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

static std::string s_NewProjectName = ""; // Buffer for the project name
static std::filesystem::path s_NewFolderPath = ""; // Buffer for the folder path

static std::string s_NewSceneName = ""; // Buffer for the scene name

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
	
	ImGuiID dockspace_id = 0;
	
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
			dockspace_id = ImGui::GetID("Louron Editor Dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar()) {

			if (ImGui::BeginMenu("File")) {

				if (ImGui::BeginMenu("New")) {

					if (ImGui::MenuItem("New Project")) {

						m_ActiveGUIWindows["CreateNewProjectPopup"] = true;
						s_NewProjectName.clear();
						s_NewFolderPath.clear();

					}

					if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					{
						NewScene();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Open")) {

					if (ImGui::MenuItem("Open Project")) {
						std::string filepath = FileUtils::OpenFile("Louron Project (*.lproj)\0*.lproj\0", Project::GetActiveProject()->GetProjectDirectory());
						if (!filepath.empty()) {

							m_SelectedEntity = {};
							m_SceneState = SceneState::Edit;

							OnSceneStop();
							Project::GetActiveScene()->OnStop();

							Project::SaveProject();

							// 1. same as before, load project
							Project::LoadProject(filepath);

							// 2. then ensure script manager has the correct assembly
							ScriptManager::SetAppAssemblyPath(Project::GetActiveProject()->GetProjectDirectory() / Project::GetActiveProject()->GetConfig().AppScriptAssemblyPath);

							for (const auto& entry : std::filesystem::directory_iterator(Project::GetActiveProject()->GetProjectDirectory() / "Scripts/")) {
								if (entry.path().extension() == ".csproj") {
									m_ScriptsCompiledSuccess = Utils::CompileAppAssembly(entry.path());
									ScriptManager::ReloadAssembly(); // Dont check if compilation succeeded because we need it to point the to updated assembly!
									break;
								}
							}

							// 3. then load startupscene 
							Project::GetActiveProject()->LoadStartupScene();

							m_ScriptFileWatcher.reset();
							m_ScriptFileWatcher = std::make_unique<filewatch::FileWatch<std::string>>((Project::GetActiveProject()->GetProjectDirectory() / "Scripts").string(), ScriptsModifiedEvent);

							auto scene = Project::GetActiveScene();

							FrameBufferConfig fbo_config;
							fbo_config.Width = m_ViewportWindowSize.x;
							fbo_config.Height = m_ViewportWindowSize.y;
							fbo_config.RenderToScreen = false;
							fbo_config.Samples = 1;

							scene->CreateSceneFrameBuffer(fbo_config);

							scene->OnStart();
						}
					}

					if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {

						OpenScene();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Save")) {

					if (ImGui::MenuItem("Save Project")) {

						OnSceneStop();

						Project::SaveProject();
					}

					if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
						SaveScene();
					}

					if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S"))
					{
						SaveScene(true);
					}

					ImGui::EndMenu();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Reload Shaders")) {
					Engine::Get().GetShaderLibrary().ReloadAllShaders();
				}

				if (ImGui::MenuItem("Reload Script Assembly")) {
					for (const auto& entry : std::filesystem::directory_iterator(Project::GetActiveProject()->GetProjectDirectory() / "Scripts/")) {
						if (entry.path().extension() == ".csproj") {
							m_ScriptsCompiledSuccess = Utils::CompileAppAssembly(entry.path());
							if(m_ScriptsCompiledSuccess)
								ScriptManager::ReloadAssembly();
							break;
						}
					}
				}

				ImGui::Separator();

				ImGui::MenuItem("Show Docking Options", NULL, &opt_show_options);
				ImGui::MenuItem("Show ImGui Demo", NULL, &opt_show_demo_window);

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
				Engine::Get().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit")) {

				if (ImGui::MenuItem("Project Properties")) 
					m_ActiveGUIWindows["ProjectProperties"] = true;

				if (ImGui::MenuItem("Scene Properties"))
					m_ActiveGUIWindows["SceneProperties"] = true;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {

				if (ImGui::BeginMenu("Windows")) {

					for (const auto& pair : m_ActiveGUIWindows) {

						if (pair.first == "CreateNewProjectPopup" || pair.first == "CreateNewScenePopup" || pair.first == "ProjectProperties" || pair.first == "SceneProperties") continue;

						if (ImGui::MenuItem(pair.first)) {
							m_ActiveGUIWindows[pair.first] = true;
						}
					}
					ImGui::EndMenu();
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

		// Render the modal dialog
		if (m_ActiveGUIWindows["CreateNewProjectPopup"]) {
			ImGui::OpenPopup("New Project");

			// Calculate window size: 25% of the screen width, minimum 300.0f
			float windowWidth = glm::max(300.0f, ImGui::GetContentRegionAvail().x * 0.25f);
			float windowHeight = 125.0f; // Fixed height
			ImVec2 windowSize(windowWidth, windowHeight);
			ImGui::SetNextWindowSize(windowSize);

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 viewportPos = viewport->Pos;
			ImVec2 viewportSize = viewport->Size;

			// Calculate centered position
			ImVec2 windowPos(
				viewportPos.x + (viewportSize.x - windowWidth) * 0.5f, // Center horizontally
				viewportPos.y + (viewportSize.y - windowHeight) * 0.5f // Center vertically
			);
			ImGui::SetNextWindowPos(windowPos);
		}

		if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
			// Grey out the background
			ImGui::Text("Create a New Project");

			// Input fields
			char projectNameBuffer[256];
			char folderPathBuffer[512];
			strncpy_s(projectNameBuffer, s_NewProjectName.c_str(), sizeof(projectNameBuffer));
			strncpy_s(folderPathBuffer, std::filesystem::path(s_NewFolderPath / s_NewProjectName).string().c_str(), sizeof(folderPathBuffer));

			float first_coloumn_width = ImGui::CalcTextSize("Project Name").x + 10.0f;

			ImGui::Columns(2, "New_Proj_Cols", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Project Name");

			ImGui::NextColumn();

			if (ImGui::InputText(".lproj", projectNameBuffer, sizeof(projectNameBuffer))) {
				s_NewProjectName = projectNameBuffer;
			}

			ImGui::NextColumn();

			ImGui::Text("Folder Path");

			ImGui::NextColumn();

			ImGui::InputText("##Folder Path", folderPathBuffer, sizeof(folderPathBuffer), ImGuiInputTextFlags_ReadOnly); 
			
			ImGui::SameLine();

			if (ImGui::Button("...")) {
				s_NewFolderPath = FileUtils::OpenDirectory();
			}

			ImGui::Columns(1);

			// Buttons for Create and Cancel
			if (ImGui::Button("Create") || Engine::Get().GetInput().GetKeyDown(GLFW_KEY_ENTER)) {
				if (!s_NewProjectName.empty()) {

					s_NewFolderPath /= s_NewProjectName;

					// Create and set the new project
					m_SelectedEntity = {};
					m_SceneState = SceneState::Edit;

					OnSceneStop();
					Project::GetActiveScene()->OnStop();

					Project::SaveProject();

					auto project = Project::NewProject(s_NewProjectName, s_NewFolderPath);

					// Generate C# Scripting MSVC Solution - TODO: idk use some form of project build tools?
					Utils::GenerateScriptingProject(project->GetConfig().Name, project->GetProjectDirectory() / "Scripts");

					ScriptManager::SetAppAssemblyPath(project->GetProjectDirectory() / project->GetConfig().AppScriptAssemblyPath);

					// Build initial DLL
					for (const auto& entry : std::filesystem::directory_iterator(project->GetProjectDirectory() / "Scripts")) {
						if (entry.path().extension() == ".csproj") {
							m_ScriptsCompiledSuccess = Utils::CompileAppAssembly(entry.path());
							ScriptManager::ReloadAssembly(); // Dont check if compilation succeeded because we need it to point the to updated assembly!
							break;
						}
					}

					m_ScriptFileWatcher.reset();
					m_ScriptFileWatcher = std::make_unique<filewatch::FileWatch<std::string>>((Project::GetActiveProject()->GetProjectDirectory() / "Scripts").string(), ScriptsModifiedEvent);

					auto scene = Project::GetActiveScene();

					FrameBufferConfig fbo_config;
					fbo_config.Width = m_ViewportWindowSize.x;
					fbo_config.Height = m_ViewportWindowSize.y;
					fbo_config.RenderToScreen = false;
					fbo_config.Samples = 1;

					scene->CreateSceneFrameBuffer(fbo_config);

					scene->OnStart();

					m_ActiveGUIWindows["CreateNewProjectPopup"] = false; // Close the dialog
					ImGui::CloseCurrentPopup();
				}

			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel")) {
				m_ActiveGUIWindows["CreateNewProjectPopup"] = false; // Close the dialog
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (m_ActiveGUIWindows["CreateNewScenePopup"]) {
			ImGui::OpenPopup("New Scene");

			// Calculate window size: 25% of the screen width, minimum 300.0f
			float windowWidth = glm::max(300.0f, ImGui::GetContentRegionAvail().x * 0.25f);
			float windowHeight = 100.0f; // Fixed height
			ImVec2 windowSize(windowWidth, windowHeight);
			ImGui::SetNextWindowSize(windowSize);

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 viewportPos = viewport->Pos;
			ImVec2 viewportSize = viewport->Size;

			// Calculate centered position
			ImVec2 windowPos(
				viewportPos.x + (viewportSize.x - windowWidth) * 0.5f, // Center horizontally
				viewportPos.y + (viewportSize.y - windowHeight) * 0.5f // Center vertically
			);
			ImGui::SetNextWindowPos(windowPos);
		}

		if (ImGui::BeginPopupModal("New Scene", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
			// Grey out the background
			ImGui::Text("Create a New Scene");

			// Input fields
			char sceneNameBuffer[256];
			strncpy_s(sceneNameBuffer, s_NewSceneName.c_str(), sizeof(sceneNameBuffer));

			float first_coloumn_width = ImGui::CalcTextSize("Scene Name").x + 10.0f;

			ImGui::Columns(2, "New_Scene_Cols", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Scene Name");

			ImGui::NextColumn();

			if (ImGui::InputText(".lscene", sceneNameBuffer, sizeof(sceneNameBuffer))) {
				s_NewSceneName = sceneNameBuffer;
			}

			ImGui::Columns(1);

			// Buttons for Create and Cancel
			if (ImGui::Button("Create") || Engine::Get().GetInput().GetKeyDown(GLFW_KEY_ENTER)) {
				if (!s_NewSceneName.empty())
				{
					std::filesystem::path file_path = Project::GetActiveProject()->GetProjectDirectory() / "Scenes" / (s_NewSceneName + ".lscene");
					if (!file_path.empty()) {

						m_SelectedEntity = {};
						m_SceneState = SceneState::Edit;

						auto project = Project::GetActiveProject();

						OnSceneStop();
						project->GetActiveScene()->OnStop();

						project->SaveScene();

						auto scene = project->NewScene(file_path);

						FrameBufferConfig fbo_config;
						fbo_config.Width = m_ViewportWindowSize.x;
						fbo_config.Height = m_ViewportWindowSize.y;
						fbo_config.RenderToScreen = false;
						fbo_config.Samples = 1;

						scene->CreateSceneFrameBuffer(fbo_config);

						scene->OnStart();

						m_ActiveGUIWindows["CreateNewScenePopup"] = false; // Close the dialog
						ImGui::CloseCurrentPopup();
					}
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel")) {
				m_ActiveGUIWindows["CreateNewScenePopup"] = false; // Close the dialog
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (m_ActiveGUIWindows["ScriptCompilationWarningMessage"]){
			ImGui::OpenPopup("Script Error");
			// Calculate window size: 25% of the screen width, minimum 300.0f
			float windowWidth = glm::max(300.0f, ImGui::GetContentRegionAvail().x * 0.25f);
			float windowHeight = 50.0f; // Fixed height
			ImVec2 windowSize(windowWidth, windowHeight);
			ImGui::SetNextWindowSize(windowSize);

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 viewportPos = viewport->Pos;
			ImVec2 viewportSize = viewport->Size;

			// Calculate centered position
			ImVec2 windowPos(
				viewportPos.x + (viewportSize.x - windowWidth) * 0.5f, // Center horizontally
				viewportPos.y + (viewportSize.y - windowHeight) * 0.5f // Center vertically
			);
			ImGui::SetNextWindowPos(windowPos);
		}

		if (ImGui::BeginPopupModal("Script Error", &m_ActiveGUIWindows["ScriptCompilationWarningMessage"], ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
			
			// Center text horizontally
			float textWidth = ImGui::CalcTextSize("Cannot Start Scene With Script Errors!").x;
			ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textWidth) * 0.5f);

			// Enable text wrapping
			ImGui::PushTextWrapPos(ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x);
			ImGui::TextWrapped("Cannot Start Scene With Script Errors!");
			ImGui::PopTextWrapPos();

			if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
				m_ActiveGUIWindows["ScriptCompilationWarningMessage"] = false;
			
			ImGui::EndPopup();
		}
		
		CheckInput();
				
		DisplaySceneViewportWindow();
		DisplayHierarchyWindow();
		DisplayPropertiesWindow();
		DisplayContentBrowserWindow();
		DisplayRenderStatsWindow();
		DisplayProfilerWindow();
		DisplayAssetRegistryWindow();

		DisplayProjectProperties();
		DisplaySceneProperties();

	}
	ImGui::End();
	
	if (!Project::GetActiveScene()->IsRunning() && glfwGetWindowAttrib((GLFWwindow*)Engine::Get().GetWindow().GetNativeWindow(), GLFW_FOCUSED)) {

		if (m_ScriptsNeedCompiling.load(std::memory_order_relaxed))
		{
			m_ScriptFileWatcher.reset();

			// Handle script recompilation logic
			for (const auto& entry : std::filesystem::directory_iterator(Project::GetActiveProject()->GetProjectDirectory() / "Scripts/")) {
				if (entry.path().extension() == ".csproj") {
					m_ScriptsCompiledSuccess = Utils::CompileAppAssembly(entry.path());
					
					if(m_ScriptsCompiledSuccess)
						ScriptManager::ReloadAssembly();

					break;
				}
			}

			m_ScriptFileWatcher = std::make_unique<filewatch::FileWatch<std::string>>((Project::GetActiveProject()->GetProjectDirectory() / "Scripts").string(), ScriptsModifiedEvent);
			m_ScriptsNeedCompiling.store(false, std::memory_order_relaxed); // Reset the flag
		}
	}
	else {
		m_WindowWasUnfocused = true;
	}
}

void LouronEditorLayer::OnScenePlay()
{
	if (!m_ScriptsCompiledSuccess)
	{
		m_ActiveGUIWindows["ScriptCompilationWarningMessage"] = true;
		return;
	}

	L_APP_INFO("Starting Scene");

	m_GizmoType = -1;
	m_SceneState = SceneState::Play;
	m_EditorScene = Scene::Copy(Project::GetActiveScene());
	m_SceneWindowFocused = false;

	Project::GetActiveScene()->OnRuntimeStart();
}

void LouronEditorLayer::OnSceneStop()
{
	L_APP_INFO("Stopping Scene");

	if(m_SceneState == SceneState::Play)
		Project::GetActiveScene()->OnRuntimeStop();
	else if (m_SceneState == SceneState::Simulate)
		Project::GetActiveScene()->OnSimulationStop();

	m_SceneState = SceneState::Edit;
	m_SceneWindowFocused = false;
	
	Louron::UUID selected_uuid = m_SelectedEntity ? m_SelectedEntity.GetUUID() : (Louron::UUID)NULL_UUID;
	m_SelectedEntity = {};

	if (m_EditorScene)
	{
		Project::GetActiveScene()->OnStop();

		Project::SetActiveScene(m_EditorScene);

		FrameBufferConfig fbo_config;
		fbo_config.Width = 1;
		fbo_config.Height = 1;
		fbo_config.RenderToScreen = false;
		fbo_config.Samples = 1;

		Project::GetActiveScene()->CreateSceneFrameBuffer(fbo_config);
		Project::GetActiveScene()->OnStart();

		if(selected_uuid != NULL_UUID)
			m_SelectedEntity = m_EditorScene->FindEntityByUUID(selected_uuid);

		m_EditorScene.reset();
		m_EditorScene = nullptr;
	}
}

void LouronEditorLayer::DisplaySceneViewportWindow() {

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

		m_SceneWindowHovered = ImGui::IsItemHovered();

		if (ImGui::IsItemClicked() && m_SceneState == SceneState::Play)
			m_SceneWindowFocused = true;

		if(m_SceneWindowFocused && m_SceneState == SceneState::Play && Engine::Get().GetInput().GetKeyDown(GLFW_KEY_ESCAPE))
			m_SceneWindowFocused = false;

		bool scene_image_hovered = !ImGui::IsItemHovered();

		// IM GUIZMO
		Entity selectedEntity = m_SelectedEntity;
		if (selectedEntity && m_GizmoType != -1 && m_SceneState != SceneState::Play)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
				m_ViewportBounds[1].x - m_ViewportBounds[0].x,
				m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Editor camera
			const glm::mat4& cameraProjection = m_EditorCamera->GetProjection();
			glm::mat4 cameraView = m_EditorCamera->GetViewMatrix();

			// Entity transform
			auto& transform_component = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = transform_component.GetGlobalTransform();

			// Snapping
			bool snap = Engine::Get().GetInput().GetKey(GLFW_KEY_LEFT_CONTROL);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? &snapValue : nullptr);

			static glm::mat4 start_transform{};
			static bool started_using = false;
			if (ImGuizmo::IsUsing())
			{
				if (!started_using) {
					start_transform = transform_component.GetGlobalTransform();
					started_using = true;
				}

				if (Engine::Get().GetInput().GetKeyDown(GLFW_KEY_ESCAPE)) {
					transform = start_transform;
					start_transform = {};
					started_using = false;
					ImGuizmo::Enable(false);
				}

				glm::vec3 position, rotation, scale;

				position = TransformComponent::GetPositionFromMatrix(transform);
				rotation = TransformComponent::GetRotationFromMatrix(transform);
				scale = TransformComponent::GetScaleFromMatrix(transform);

				transform_component.SetGlobalPosition(position);
				transform_component.SetGlobalRotation(rotation);
				transform_component.SetGlobalScale(scale);
			}
			else {
				started_using = false;
			}
			ImGuizmo::Enable(true); // Disable ImGuizmo
		}

		// ----- Draw Scene Control Buttons -----
		{
			ImGuiWindowFlags window_flags =
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoDocking |
				ImGuiWindowFlags_NoBackground;

			ImVec2 scene_control_window_pos = ImGui::GetWindowPos();
			ImVec2 scene_control_window_size = ImGui::GetWindowSize();
			ImVec2 scene_control_overlay_size = ImVec2(40.0f, 40.0f);
			ImVec2 scene_control_overlay_pos = ImVec2(
				scene_control_window_pos.x + scene_control_window_size.x - scene_control_overlay_size.x - 10.0f,  // 10.0f is a margin
				scene_control_window_pos.y + ImGui::GetCurrentWindow()->TitleBarHeight + 10.0f   // 10.0f is a margin
			);

			ImGui::SetNextWindowSize(scene_control_overlay_size, ImGuiCond_Always);
			ImGui::SetNextWindowPos(scene_control_overlay_pos, ImGuiCond_Always);
			if (ImGui::Begin("Scene Control", nullptr, window_flags))
			{
				// Center the button horizontally
				float buttonWidth = 30.0f; // Width of the play button
				ImVec2 available_region = ImGui::GetContentRegionAvail();
				ImGui::SetCursorPosX((available_region.x - buttonWidth) * 0.5f);

				bool isPlaying = m_SceneState == SceneState::Play;
				GLuint icon_texture_id = isPlaying ? m_IconStop->GetID() : m_IconPlay->GetID();

				ImGui::SetCursorPosX(0.0f);
				ImGui::SetCursorPosY(0.0f);
				if (ImGui::ImageButton((ImTextureID)(uintptr_t)icon_texture_id, ImVec2(30.0f, 30.0f))) {

					isPlaying = !isPlaying; // Toggle play state
					if (isPlaying)
					{
						OnScenePlay();
					}
					else
					{
						OnSceneStop();
					}
				}
				scene_image_hovered = ImGui::IsItemHovered();
			}
			ImGui::End();
		}

		// ----- Draw Simple FPS Counter -----
		{

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
			scene_image_hovered = ImGui::IsItemHovered();

		}

		auto [mx, my] = ImGui::GetMousePos();
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();

		m_ViewportBounds[0] = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;

		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];

		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y && m_SceneState != SceneState::Play && !ImGuizmo::IsUsing() && scene_image_hovered)
		{
			if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) {
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
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

	m_HierarchyPanel.OnImGuiRender(scene_ref, m_SelectedEntity);

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

	m_PropertiesPanel.OnImGuiRender(scene_ref, m_SelectedEntity);


	ImGui::End();
}

void LouronEditorLayer::DisplayContentBrowserWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Content Browser"]) {
		return;
	}

	ImGui::Begin("Content Browser", &m_ActiveGUIWindows["Content Browser"], 0);

	m_ContentBrowserPanel.OnImGuiRender(*this);

	ImGui::End();

}

void LouronEditorLayer::DisplayRenderStatsWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Render Stats"]) {
		return;
	}

	if (ImGui::Begin("Render", &m_ActiveGUIWindows["Render Stats"], 0)) {

		auto& stats = Renderer::GetFrameRenderStats();
		auto& FP_Data = std::static_pointer_cast<ForwardPlusPipeline>(Project::GetActiveScene()->GetConfig().ScenePipeline)->GetFPData();

		if (ImGui::TreeNodeEx("Rendering Options", ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::Checkbox("View Light Complexity", &FP_Data.ShowLightComplexity);
			ImGui::Checkbox("View Wireframe", &FP_Data.ShowWireframe);

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Rendering Stats", ImGuiTreeNodeFlags_DefaultOpen)) {

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

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Culling Stats", ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::Text("Entities Rendered: %i", stats.Entities_Rendered);
			ImGui::Text("Entities Culled: %i", stats.Entities_Culled);

			ImGui::Text("Visible Point Lights: %i", (int)FP_Data.PLEntities.size());
			ImGui::Text("Visible Spot Lights: %i", (int)FP_Data.SLEntities.size());

			if (ImGui::TreeNodeEx("Octree Info", ImGuiTreeNodeFlags_DefaultOpen)) {
				bool octree_display_toggle = Project::GetActiveScene()->GetDisplayOctree();
				if (ImGui::Checkbox("View Octree", &octree_display_toggle))
					Project::GetActiveScene()->SetDisplayOctree(octree_display_toggle);

				ImGui::Text("Octree Data Sources: %i", (int)Project::GetActiveScene()->GetOctree().lock()->GetAllOctreeDataSources().size());

				static OctreeBoundsConfig config = Project::GetActiveScene()->GetOctree().lock()->GetConfig();

				ImGui::DragFloat("Looseness", &config.Looseness, 0.001f, 1.0f, 2.0f);
				ImGui::DragInt("PreferredDataSourceLimit", &config.PreferredDataSourceLimit, 0.25f, 1);

				if (ImGui::Button("Rebuild Octree")) {
					L_PROFILE_SCOPE("Rebuild Octree");
					auto oct_ref = Project::GetActiveScene()->GetOctree().lock();
					oct_ref->RebuildOctree(config);

				}

				// Length of interval in seconds we gather new results
				static float timer_max = 0.5f;
				ImGui::SliderFloat("Octree Stat Timer", &timer_max, 0.1f, 1.0f);

				static float timer = timer_max;

				static float octreeTime = 0.0f;

				static const int data_size = 100;
				static float data[data_size] = { 0.0f };
				static int data_index = 0;

				if (timer > 0.0f) {
					timer -= Time::GetDeltaTime();
				}
				else {
					timer = timer_max;

					auto& results = Profiler::Get().GetResults();
					octreeTime = results["Forward Plus - Frustum Culling Octree Query"].Time;

					// Store the current octree time in the buffer
					data[data_index] = octreeTime * 1000.0f; // Convert to microseconds for easier reading
					data_index = (data_index + 1) % data_size; // Wrap around the buffer index
				}

				ImGui::Text("Octree Query Time: % .2fus", octreeTime * 1000.0f);

				// Plot the historical octree times
				ImVec2 plot_size = ImVec2(0, 80);
				ImGui::PlotLines("##Octree Query Time (us)", data, data_size, data_index, nullptr, 0.0f, 100.0f, plot_size); // Adjust the y-range as needed

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void LouronEditorLayer::DisplayProfilerWindow() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["Profiler"]) {
		return;
	}

	if (ImGui::Begin("Profiler", &m_ActiveGUIWindows["Profiler"], 0)) {

		static float timer = 1.0f;
		static bool IsResultsPerFrame = true;
		static std::map<const char*, ProfileResult> results;

		ImGui::Checkbox("Toggle Results Per Frame/Per Second", &IsResultsPerFrame);

		if (timer > 0.0f && !IsResultsPerFrame)
			timer -= Time::GetDeltaTime();
		
		
		if (timer <= 0.0f || IsResultsPerFrame) {
			timer = 1.0f;
			results = Profiler::Get().GetResults();
		}

		for (auto& result : results) {

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

void LouronEditorLayer::DisplayProjectProperties() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["ProjectProperties"]) {
		return;
	}

	if (ImGui::Begin("Project Properties", &m_ActiveGUIWindows["ProjectProperties"], 0)) {

		auto project = Project::GetActiveProject();

		project->GetConfig().Name;
		project->GetConfig().StartScene;

		float first_coloumn_width = ImGui::GetContentRegionAvail().x * 0.35f;

		ImGui::Columns(2, "Project_Props_Columns", false);
		ImGui::SetColumnWidth(-1, first_coloumn_width);

		ImGui::Text("Project Name");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1.0f);

		char buf[256];
		strcpy_s(buf, project->GetConfig().Name.c_str());
		ImGui::InputText("##ProjectName", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

		ImGui::NextColumn();

		ImGui::Text("Start Scene");
		ImGui::NextColumn();

		strcpy_s(buf, project->GetConfig().StartScene.stem().string().c_str());
		ImGui::InputText("##ProjectStartScene", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
		ImGui::SameLine();
		if (ImGui::Button("...")) {
			std::filesystem::path file_path = FileUtils::OpenFile("Louron Scene (*.lscene)\0*.lscene\0");

			// Check if scene is in scenes directory in this project
			auto scenes_directory = project->GetProjectDirectory() / "Scenes";
			if (file_path.lexically_normal().string().find(scenes_directory.lexically_normal().string()) != 0)
			{
				L_CORE_WARN("Scene is Not Located in Current Project ({0}): {1}", project->GetConfig().Name, file_path.string());
			}
			else {
				file_path = std::filesystem::relative(file_path, project->GetProjectDirectory());

				ProjectConfig config = project->GetConfig();
				config.StartScene = file_path;
				project->SetConfig(config);

				OnSceneStop();

				Project::SaveProject();
			}
		}

		ImGui::Columns(1);

	}

	ImGui::End();

}

void LouronEditorLayer::DisplaySceneProperties() {

	// Check if the window is open
	if (!m_ActiveGUIWindows["SceneProperties"]) {
		return;
	}

	if (ImGui::Begin("Scene Properties", &m_ActiveGUIWindows["SceneProperties"], 0)) {

		auto& scene_config = Project::GetActiveScene()->GetConfig();

		float first_coloumn_width = ImGui::GetContentRegionAvail().x * 0.35f;

		ImGui::Columns(2, "Scene_Props_Columns", false);
		ImGui::SetColumnWidth(-1, first_coloumn_width);

		ImGui::Text("Scene Name");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1.0f);

		char buf[256];
		strcpy_s(buf, scene_config.Name.c_str());
		ImGui::InputText("##SceneName", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

		ImGui::NextColumn();

		ImGui::Text("Scene Render Pipeline");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1.0f);

		std::string buf_string = (scene_config.ScenePipelineType == L_RENDER_PIPELINE::FORWARD) ? "Forward" : (scene_config.ScenePipelineType == L_RENDER_PIPELINE::FORWARD_PLUS) ? "Forward Plus" : "Deferred";

		strcpy_s(buf, buf_string.c_str());
		ImGui::InputText("##ScenePipeline", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

		ImGui::Columns(1);
	}
	ImGui::End();
}

void LouronEditorLayer::CheckInput() {

	if (Project::GetActiveScene()->IsRunning())
		return;

	InputManager& input = Engine::Get().GetInput();
	if (input.GetKey(GLFW_KEY_LEFT_CONTROL) || input.GetKey(GLFW_KEY_RIGHT_CONTROL)) {
		
		if (input.GetKeyDown(GLFW_KEY_N)) {
			NewScene();
		}

		if (input.GetKeyDown(GLFW_KEY_O)) {
			OpenScene();
		}

		if (input.GetKeyDown(GLFW_KEY_S)) {

			if ((input.GetKey(GLFW_KEY_LEFT_SHIFT) || input.GetKey(GLFW_KEY_RIGHT_SHIFT))) {
				SaveScene(true);
			}
			else {
				SaveScene();
			}
		}
	}
	else {

		if (input.GetKeyDown(GLFW_KEY_Q)) {

			if (!ImGuizmo::IsUsing())
				m_GizmoType = -1;
		}
		if (input.GetKeyDown(GLFW_KEY_G)) {

			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
		}
		if (input.GetKeyDown(GLFW_KEY_R)) {

			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
		}
		if (input.GetKeyDown(GLFW_KEY_S)) {

			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
		}
	}
}

void LouronEditorLayer::NewScene() {

	m_ActiveGUIWindows["CreateNewScenePopup"] = true;
	s_NewSceneName.clear();
}

void LouronEditorLayer::OpenScene(const std::filesystem::path& scene_file_path) {

	std::string filepath;
	
	if (scene_file_path.empty())
		filepath = FileUtils::OpenFile("Louron Scene (*.lscene)\0*.lscene\0", Project::GetActiveProject()->GetProjectDirectory() / "Scenes");
	else
		filepath = scene_file_path.string();

	if (!filepath.empty()) {

		m_SelectedEntity = {};
		m_SceneState = SceneState::Edit;

		auto project = Project::GetActiveProject();

		OnSceneStop();
		project->GetActiveScene()->OnStop();

		project->SaveScene();

		auto scene = project->LoadScene(filepath);

		if (scene) {

			FrameBufferConfig fbo_config;
			fbo_config.Width = m_ViewportWindowSize.x;
			fbo_config.Height = m_ViewportWindowSize.y;
			fbo_config.RenderToScreen = false;
			fbo_config.Samples = 1;

			scene->CreateSceneFrameBuffer(fbo_config);
		}

		project->GetActiveScene()->OnStart(); // New Scene may not load, so we call this to ensure old scene is loaded
	}
}

void LouronEditorLayer::SaveScene(bool save_as) {

	if (m_SceneWindowFocused)
		return;

	Entity selected = m_SelectedEntity;

	if (save_as) {

		std::string filepath = FileUtils::SaveFile("Louron Scene (*.lscene)\0*.lscene\0");
		if (!filepath.empty()) {

			OnSceneStop();

			Project::GetActiveScene()->SetSceneFilePath(filepath);
			Project::GetActiveProject()->SaveScene();
		}
	}
	else {
		OnSceneStop();

		Project::GetActiveProject()->SaveScene();
	}

	m_SelectedEntity = selected;
}
