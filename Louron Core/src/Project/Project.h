#pragma once

#include "../Debug/Assert.h"

#include "../Scene/Scene.h"
#include "../Scene/Scene Serializer.h"

#include <string>
#include <memory>
#include <filesystem>

namespace Louron {

	struct ProjectConfig {

		std::string Name = "Untitled Project";

		std::filesystem::path StartScene;
		std::filesystem::path AssetDirectory;
	};

	class Project {

	public:

		static std::shared_ptr<Project> GetActiveProject() { return s_ActiveProject; }
		static std::shared_ptr<Scene> GetActiveScene() { return s_ActiveProject->m_ActiveScene; }

		static void SetActiveProject(std::shared_ptr<Project> project);
		static void SetActiveScene(std::shared_ptr<Scene> scene);

		static std::shared_ptr<Project> NewProject(const std::filesystem::path& projectFilePath);
		static std::shared_ptr<Project> LoadProject(const std::filesystem::path& projectFilePath);
		static bool SaveProject(const std::filesystem::path& projectFilePath = "");

		std::shared_ptr<Scene> NewScene(const std::filesystem::path& sceneFilePath);
		std::shared_ptr<Scene> LoadScene(const std::filesystem::path& sceneFilePath);
		bool SaveScene(const std::filesystem::path& sceneFilePath = "");
		void SetScene(std::shared_ptr<Scene> scene);

		const ProjectConfig& GetConfig() { return m_Config; }
		void SetConfig(const ProjectConfig& config) { m_Config = config; }

	private:

		inline static std::shared_ptr<Project> s_ActiveProject;

		ProjectConfig m_Config;
		std::shared_ptr<Scene> m_ActiveScene;
		std::filesystem::path m_ProjectFilePath;
		std::filesystem::path m_ProjectDirectory;

	};

}