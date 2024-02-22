#pragma once

// Louron Core Headers
#include "Project.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	class ProjectSerializer {

	public:
		ProjectSerializer(std::shared_ptr<Project> project);

		bool Serialize(const std::filesystem::path& projectFilePath);
		bool Deserialize(const std::filesystem::path& projectFilePath);
	
	private:
		std::shared_ptr<Project> m_Project;
	};
}