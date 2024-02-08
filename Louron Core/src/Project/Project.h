#pragma once

#include "../Debug/Assert.h"

#include <string>
#include <filesystem>

namespace Louron {

	struct ProjectConfig {

		std::string Name = "Untitled Project";

		std::filesystem::path AssetDirectory;
	};

	class Project {
	public:
		static const std::filesystem::path& GetProjectDirectory() {
			// Check if there is an active project
			L_CORE_ASSERT(s_ActiveProject, "There is no active scene!");
			return s_ActiveProject->m_ProjectDirectory;
		}

	private:

		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;

		inline static std::shared_ptr<Project> s_ActiveProject;
	};

}