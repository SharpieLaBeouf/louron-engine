#pragma once
#include "Project.h"

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