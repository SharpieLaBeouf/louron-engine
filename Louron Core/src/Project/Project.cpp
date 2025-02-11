#include "Project.h"

// Louron Core Headers
#include "Project Serializer.h"

#include "../Core/Logging.h"

#include "../Renderer/Camera.h"
#include "../Renderer/RendererPipeline.h"

#include "../Scene/OctreeBounds.h"
#include "../Scene/Entity.h"
#include "../Scripting/Script Manager.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	// PROJECT

	/// <summary>
	/// Create a New Project Instance.
	/// Please Note: this will create a new project, and it will replace the current project. Please 
	/// ensure that any changes made to the existing project are saved before creating a new project.
	/// </summary>
	/// <param name="projectPath">
	/// Path of where project will be saved. Please ensure the 
	/// path includes the file of the project. The filename of the project will become
	/// the name of the project. If a project already exists in this path
	/// this will call Project::LoadProject and return an instance the saved project.
	/// </param>
	std::shared_ptr<Project> Project::NewProject(const std::string& project_name, const std::filesystem::path& project_folder_path) {

		std::filesystem::path outFilePath = std::filesystem::absolute(project_folder_path) / (project_name + ".lproj");

		// 1. Validate ProjectFilePath
		{
			// Check if Project File Path is Empty.
			if (project_name.empty()) {

				L_CORE_WARN("Cannot Create New Project - No Project Name Provided!");
				return nullptr;
			}

			// Check if Project File Path is Empty.
			if (project_folder_path.empty()) {

				L_CORE_WARN("Cannot Create New Project - No Project Path Provided!");
				return nullptr;
			}

			// Checks if Project Already Exists and Returns Deserialized Project Instance.
			if (std::filesystem::exists(outFilePath)) {
				L_CORE_INFO("Project Already Exists");
				return Project::LoadProject(outFilePath);
			}

			// Check if Project Has a Dedicated Parent Project Directory.
			// If not we will create a directory in the name of the ProjectFilePath parsed to this function.
			std::filesystem::create_directories(outFilePath.parent_path());

		}
		
		// 2. Initialise Project and Scene Instances.
		std::shared_ptr<Project> project = std::make_shared<Project>();
		std::shared_ptr<Project> old_project_just_in_case = s_ActiveProject;

		// Stop any scenes running at the moment
		if (s_ActiveProject && s_ActiveProject->m_ActiveScene)
			s_ActiveProject->m_ActiveScene->OnStop();

		// Set Active Project to this new instance
		s_ActiveProject.reset();
		s_ActiveProject = nullptr;
		s_ActiveProject = project;

		// Create Project Directories
		std::filesystem::path projectDirectory = outFilePath.parent_path();
		std::filesystem::create_directories(projectDirectory / "Scenes");

		std::filesystem::create_directories(projectDirectory / "Scripts/Binaries");

		std::filesystem::create_directories(projectDirectory / "Assets/Audio");
		std::filesystem::create_directories(projectDirectory / "Assets/Materials");
		std::filesystem::create_directories(projectDirectory / "Assets/Models");
		std::filesystem::create_directories(projectDirectory / "Assets/Shaders");
		std::filesystem::create_directories(projectDirectory / "Assets/Textures");

		// Set Paths
		project->m_ProjectFilePath = outFilePath;
		project->m_ProjectDirectory = projectDirectory;

		// Set Project Config
		project->m_Config.Name = project_name;
		project->m_Config.StartScene = "Scenes/Untitled Scene.lscene";
		project->m_Config.AssetDirectory = "Assets/";
		project->m_Config.AssetRegistry = "AssetRegistry.lassetreg";
		project->m_Config.CoreScriptAssemblyPath = "Scripts/Binaries/Louron Script Core.dll"; 
		
		std::string tempName = project->m_Config.Name; // Create a temporary copy
		tempName.erase(std::remove(tempName.begin(), tempName.end(), ' '), tempName.end()); // Remove spaces
		project->m_Config.AppScriptAssemblyPath = "Scripts/Binaries/" + tempName + ".dll";

		// This is a new project. New projects can only be created in the editor environment.
		// Additionally, as this is a new project, we will serialise the empty AssetManager to
		// ensure the file is created in the AssetDirectory.
		project->m_AssetManager = std::make_shared<EditorAssetManager>();
		project->GetEditorAssetManager()->InitDefaultResources();
		project->GetEditorAssetManager()->RefreshAssetRegistry(project->GetAssetDirectory());

		// We now want to create a new empty scene that is attached to this Project automatically.
		project->m_ActiveScene = project->NewScene(project->m_ProjectDirectory / project->m_Config.StartScene);

		// 3. Serialise Project Data
		ProjectSerializer serializer(project);
		if (!serializer.Serialize(outFilePath)) {
			L_CORE_ERROR("Project Could Not Be Saved");
			s_ActiveProject = old_project_just_in_case;
			s_ActiveProject->m_ActiveScene->OnStart();
			return s_ActiveProject;
		}

		// 4. Serialise Empty Scene Data
		L_CORE_INFO("Project Created: {0}", project->m_Config.Name);

		return project;
	}

	/// <summary>
	/// Loads a Project File.
	/// Please Note: This will load a project and replace any current project 
	/// running. Ensure project is saved before Loading a new project!
	/// 
	/// This will not load the startup scene, you must call LoadProject, then initialise ScriptManager, then LoadStartUpScene!
	/// </summary>
	std::shared_ptr<Project> Project::LoadProject(const std::filesystem::path& projectFilePath) {

		std::filesystem::path abs_project_file_path = std::filesystem::absolute(projectFilePath);

		if (abs_project_file_path.empty()) {
			L_CORE_ERROR("No Project File Path Provided");
			return nullptr;
		}

		if (abs_project_file_path.extension() != ".lproj") {
			L_CORE_ERROR("Incompatible Project File Extension! Extension Used: {0}, Extension Required : .lproj", abs_project_file_path.extension().string());
			return nullptr;
		}

		if (!std::filesystem::exists(abs_project_file_path)) {
			L_CORE_ERROR("Project File Does Not Exist");
			return nullptr;
		}

		// Create Project
		std::shared_ptr<Project> project = std::make_shared<Project>();
		std::shared_ptr<Project> old_project_just_in_case = s_ActiveProject;

		// Stop any scenes running at the moment
		if (s_ActiveProject && s_ActiveProject->m_ActiveScene)
			s_ActiveProject->m_ActiveScene->OnStop();

		// Set Active Project to this new instance
		s_ActiveProject.reset();
		s_ActiveProject = nullptr;
		s_ActiveProject = project;

		ProjectSerializer serializer(project);
		if (!serializer.Deserialize(abs_project_file_path)) {
			L_CORE_ERROR("Project Could Not Be Loaded.");
			return old_project_just_in_case;
		}

		project->m_ProjectFilePath = abs_project_file_path;
		project->m_ProjectDirectory = abs_project_file_path.parent_path();

		project->m_AssetManager = std::make_shared<EditorAssetManager>();
		project->GetEditorAssetManager()->InitDefaultResources();
		project->GetEditorAssetManager()->RefreshAssetRegistry(project->GetAssetDirectory());

		L_CORE_INFO("Project Loaded: {0}", project->m_Config.Name);
		return project;
	}

	/// <summary>
	/// Saves a Project File and Active Scene File.
	/// </summary>
	bool Project::SaveProject() {

		// First Save Active Scene
		s_ActiveProject->SaveScene();

		// Second Save Project Data
		ProjectSerializer serializer(s_ActiveProject);

		if (!serializer.Serialize(s_ActiveProject->m_ProjectFilePath)) {
			L_CORE_ERROR("Project Could Not Be Saved");
			return false;
		}

		L_CORE_INFO("Project ({0}) Saved At: {1}", s_ActiveProject->m_Config.Name, s_ActiveProject->m_ProjectFilePath.string());
		return true;
	}

	// SCENE

	/// <summary>
	/// Creates a New Scene.
	/// This does not save previous scene, please 
	/// ensure you use SaveScene to save progress.
	/// 
	/// You must pass an absolute path to the new scene file.
	/// </summary>
	std::shared_ptr<Scene> Project::NewScene(const std::filesystem::path& sceneFilePath)
	{
		std::filesystem::path scene_file_path;
		auto scenes_directory = m_ProjectDirectory / "Scenes";

		// Resolve the output file path
		if (sceneFilePath.lexically_normal().string().find(scenes_directory.lexically_normal().string()) != 0) {
			L_CORE_ERROR("Scene is Not Located in Current Project ({0}): {1}", s_ActiveProject->m_Config.Name, scene_file_path.string());
			return nullptr;
		}
		else 
			scene_file_path = sceneFilePath; // Use the provided path as is

		if (m_ActiveScene)
			m_ActiveScene->OnStop();

		m_ActiveScene.reset();
		m_ActiveScene = nullptr;
		m_ActiveScene = std::make_shared<Scene>();

		// See if Scene file already exists and create unique filename
		if (std::filesystem::exists(scene_file_path)) {

			auto directory = scene_file_path.parent_path();
			auto stem = scene_file_path.stem().string();  // File name without extension
			auto extension = scene_file_path.extension(); // File extension

			int counter = 1;
			std::filesystem::path newFilePath = scene_file_path;
			while (std::filesystem::exists(newFilePath))
			{
				newFilePath = directory / (stem + "_" + std::to_string(counter++) + extension.string());
			}
			scene_file_path = newFilePath;
		}
		
		Entity camera_entity = m_ActiveScene->CreateEntity("Main Camera");
		auto& camera_component = camera_entity.AddComponent<CameraComponent>();
		camera_entity.GetComponent<TransformComponent>().SetGlobalPosition({ 0.0f, 5.0f, 20.0f });
		camera_entity.GetComponent<TransformComponent>().SetGlobalRotation({ -10.0f, 0.0f, 0.0f });
		camera_component.CameraInstance = std::make_shared<SceneCamera>();
		camera_component.Primary = true;

		Entity directional_light = m_ActiveScene->CreateEntity("Directional Light");
		directional_light.GetComponent<TransformComponent>().SetGlobalRotation({-30.0f, 150.0f, 0.0f});
		directional_light.AddComponent<DirectionalLightComponent>();

		SceneConfig scnConfig = m_ActiveScene->GetConfig();
		scnConfig.Name = scene_file_path.stem().string();
		scnConfig.SceneFilePath = scene_file_path;
		scnConfig.AssetDirectory = m_ProjectDirectory / "Assets";
		m_ActiveScene->SetConfig(scnConfig);

		m_ActiveScene->m_Octree = std::make_shared<OctreeBounds<Entity>>();

		SaveScene();

		L_CORE_INFO("Scene Created: {0}", scene_file_path.filename().string());

		return m_ActiveScene;
	}

	/// <summary>
	/// Loads a Scene File.
	/// Please Note: this only loads the required scene. Please use Project::SetActiveScene
	/// to make this scene the new active scene.
	/// </summary>
	std::shared_ptr<Scene> Project::LoadScene(const std::filesystem::path& sceneFilePath) {
		
		std::filesystem::path scene_file_path;
		auto scenes_directory = m_ProjectDirectory / "Scenes";

		// Resolve the output file path
		if (sceneFilePath.lexically_normal().string().find(scenes_directory.lexically_normal().string()) != 0)
		{
			L_CORE_ERROR("Scene is Not Located in Current Project ({0}): {1}", s_ActiveProject->m_Config.Name, scene_file_path.string());
			return nullptr;
		}
		else
			scene_file_path = sceneFilePath; // Use the provided path as is

		if (!std::filesystem::exists(scene_file_path))
		{
			L_CORE_ERROR("Could Not Find Scene at: {}", scene_file_path.string());
			return nullptr;
		}

		if (!std::filesystem::exists(scene_file_path)) {
			return m_ActiveScene;
		}

		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		std::shared_ptr<Scene> old_scene_just_in_case;

		if (m_ActiveScene)
			m_ActiveScene->OnStop();

		old_scene_just_in_case = m_ActiveScene;

		m_ActiveScene.reset();
		m_ActiveScene = nullptr;
		m_ActiveScene = scene;

		if (!scene->LoadSceneFile(scene_file_path)) {
			m_ActiveScene = old_scene_just_in_case;
			m_ActiveScene->OnStart();
			return m_ActiveScene;
		}

		SceneConfig scnConfig = m_ActiveScene->GetConfig();
		scnConfig.Name = scene_file_path.stem().string();
		scnConfig.SceneFilePath = scene_file_path;
		scnConfig.AssetDirectory = m_ProjectDirectory / "Assets";
		m_ActiveScene->SetConfig(scnConfig);

		if(!m_ActiveScene->m_Octree)
			m_ActiveScene->m_Octree = std::make_shared<OctreeBounds<Entity>>();
		
		L_CORE_INFO("Scene Loaded: {0}", scene_file_path.filename().string());
		return scene;
	}

	std::shared_ptr<Scene> Project::LoadStartupScene()
	{
		s_ActiveProject->m_ActiveScene = s_ActiveProject->LoadScene(s_ActiveProject->m_ProjectDirectory / s_ActiveProject->m_Config.StartScene);
		L_CORE_INFO("Scene Loaded: {0}", s_ActiveProject->m_ActiveScene->GetConfig().Name);
		return s_ActiveProject->m_ActiveScene;
	}

	/// <summary>
	/// Saves a Scene File relative to the Project Directory.
	/// </summary>
	bool Project::SaveScene() {

		if (!m_ActiveScene)
			return false;

		SceneSerializer sceneSerializer(m_ActiveScene);
		const std::filesystem::path& path = std::filesystem::absolute(m_ActiveScene->m_SceneConfig.SceneFilePath);

		if (path.empty() || !std::filesystem::exists(path.parent_path())) {
			L_CORE_ERROR("Cannot Save Scene, File Path Invalid : \'{0}\'", path.string());
			return false;
		}

		sceneSerializer.Serialize(path);
		L_CORE_INFO("Scene ({0}) Saved At: {1}", m_Config.Name, path.string());
		return true;
	}

	/// <summary>
	/// Updates the Active Scene of the Active Project.
	/// </summary>
	void Project::SetActiveScene(std::shared_ptr<Scene> scene) {
		s_ActiveProject->SetScene(scene);
	}

	/// <summary>
	/// Updates the Scene of the Project.
	/// </summary>
	/// <param name="scene"></param>
	void Project::SetScene(std::shared_ptr<Scene> scene) {
		m_ActiveScene.reset();
		m_ActiveScene = scene;
	}

	/// <summary>
	/// Static function to get retrieve the Static Pointer Cast of AssetManagerBase -> EditorAssetManager.
	/// 
	/// THIS IS TO BE CALLED ONLY IN EDITOR ENVIRONMENTS.
	/// </summary>
	std::shared_ptr<EditorAssetManager> Project::GetStaticEditorAssetManager() {
		if (auto project_ref = Project::GetActiveProject(); project_ref)
			return project_ref->GetEditorAssetManager();
		return nullptr;
	}

	/// <summary>
	/// Static Pointer Cast of AssetManagerBase -> EditorAssetManager.
	/// 
	/// THIS IS TO BE CALLED ONLY IN EDITOR ENVIRONMENTS.
	/// </summary>
	std::shared_ptr<EditorAssetManager> Project::GetEditorAssetManager() const {
		return std::static_pointer_cast<EditorAssetManager>(m_AssetManager);
	}

	std::shared_ptr<AssetManagerBase> Project::GetStaticAssetManager()
	{
		if (auto project_ref = Project::GetActiveProject(); project_ref)
			return project_ref->GetAssetManager();
		return nullptr;
	}

	std::shared_ptr<AssetManagerBase> Project::GetAssetManager() const
	{
		return m_AssetManager;
	}
}