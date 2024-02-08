#pragma once

#include "Scene.h"

#include <string>
#include <memory>

namespace Louron {

	class SceneSerializer {

	public:

		SceneSerializer(const std::shared_ptr<Scene>& scene);

		void Serialize(const std::string& sceneFilePath = "");

		bool Deserialize(const std::string& sceneFilePath = "");

	private:
		std::shared_ptr<Scene> m_Scene;
	};

}