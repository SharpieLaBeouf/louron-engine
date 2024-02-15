#pragma once

#include "Scene.h"

#include <string>
#include <memory>

namespace Louron {

	class SceneSerializer {

	public:

		SceneSerializer(const std::shared_ptr<Scene>& scene);

		void Serialize(const std::filesystem::path& sceneFilePath = "");
		bool Deserialize(const std::filesystem::path& sceneFilePath);

	private:
		std::shared_ptr<Scene> m_Scene;
	};

}