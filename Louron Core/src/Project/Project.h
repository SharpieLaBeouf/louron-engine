#pragma once

// Louron Core Headers
#include "../Debug/Assert.h"

#include "../Scene/Scene.h"
#include "../Scene/Scene Serializer.h"

#include "../Asset/Asset Manager.h"

// C++ Standard Library Headers
#include <string>
#include <memory>
#include <filesystem>

// External Vendor Library Headers

namespace Louron {

	struct ProjectConfig {

		std::string Name = "Untitled Project";

		// Needs to be absolute path!
		std::filesystem::path StartScene;

		// Needs to be absolute path!
		// This path is where all Project and Scene Assets are stored
		std::filesystem::path AssetDirectory;
		
		// This path should be Relative to the ProjectConfig::AssetDirectory
		std::filesystem::path AssetRegistry;

		// This is an absolute path of where the CoreScriptAssembly is
		std::filesystem::path CoreScriptAssemblyPath;

		// Needs to be absolute path!
		// This is a relative path of where the AppScriptAssembly is
		std::filesystem::path AppScriptAssemblyPath;
	};

	class Project {

	public: // Project Functions

		static std::shared_ptr<Project> GetActiveProject() { return (s_ActiveProject) ? s_ActiveProject : nullptr; }
		static std::shared_ptr<Scene> GetActiveScene() { return (s_ActiveProject) ? (s_ActiveProject->m_ActiveScene) ? s_ActiveProject->m_ActiveScene : nullptr : nullptr; }

		static void SetActiveScene(std::shared_ptr<Scene> scene);

		static std::shared_ptr<Project> NewProject(const std::string& project_name, const std::filesystem::path& project_folder_path);
		static std::shared_ptr<Project> LoadProject(const std::filesystem::path& projectFilePath);
		static bool SaveProject();

		const ProjectConfig& GetConfig() { return m_Config; }
		void SetConfig(const ProjectConfig& config) { m_Config = config; }

		const std::filesystem::path& GetProjectDirectory() const { return m_ProjectDirectory; }

	public: // Scene Functions

		std::shared_ptr<Scene> NewScene(const std::filesystem::path& sceneFilePath);
		std::shared_ptr<Scene> LoadScene(const std::filesystem::path& sceneFilePath);
		std::shared_ptr<Scene> LoadStartupScene();
		bool SaveScene();
		void SetScene(std::shared_ptr<Scene> scene);

	public: // Asset Manager Functions

		static std::shared_ptr<EditorAssetManager> GetStaticEditorAssetManager();
		std::shared_ptr<EditorAssetManager> GetEditorAssetManager() const;

	private:

		inline static std::shared_ptr<Project> s_ActiveProject;

		ProjectConfig m_Config;
		std::shared_ptr<Scene> m_ActiveScene;
		std::filesystem::path m_ProjectFilePath;
		std::filesystem::path m_ProjectDirectory;

		std::shared_ptr<AssetManagerBase> m_AssetManager;
	};

}