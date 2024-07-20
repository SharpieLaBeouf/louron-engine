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

		std::filesystem::path StartScene;

		// This path is where all Project and Scene Assets are stored
		std::filesystem::path AssetDirectory;
		
		// This path should be Relative to the ProjectConfig::AssetDirectory
		std::filesystem::path AssetRegistry;
	};

	class Project {

	public: // Project Functions

		static std::shared_ptr<Project> GetActiveProject() { return s_ActiveProject; }
		static std::shared_ptr<Scene> GetActiveScene() { return s_ActiveProject->m_ActiveScene; }

		static void SetActiveProject(std::shared_ptr<Project> project);
		static void SetActiveScene(std::shared_ptr<Scene> scene);

		static std::shared_ptr<Project> NewProject(const std::filesystem::path& projectFilePath = "Untitled Project/Untitled Project.lproj");
		static std::shared_ptr<Project> LoadProject(const std::filesystem::path& projectFilePath, const std::filesystem::path& startUpScene = "");
		static bool SaveProject(const std::filesystem::path& projectFilePath = "");

		const ProjectConfig& GetConfig() { return m_Config; }
		void SetConfig(const ProjectConfig& config) { m_Config = config; }

	public: // Scene Functions

		std::shared_ptr<Scene> NewScene(const std::filesystem::path& sceneFilePath);
		std::shared_ptr<Scene> LoadScene(const std::filesystem::path& sceneFilePath);
		bool SaveScene(const std::filesystem::path& sceneFilePath = "");
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