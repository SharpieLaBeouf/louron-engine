#pragma once

#include "Scene.h"

#include <string>
#include <memory>

namespace YAML { class Emitter; }

namespace Louron {

	class SceneSerializer {

	public:

		SceneSerializer(const std::shared_ptr<Scene>& scene);


		void Serialize(const std::filesystem::path& sceneFilePath = "");
		bool Deserialize(const std::filesystem::path& sceneFilePath);

	private:
		std::shared_ptr<Scene> m_Scene;

		std::shared_ptr<SkyboxMaterial> DeserializeSkyboxMaterial(const std::filesystem::path& filePath);

		void SerializeEntity(YAML::Emitter& out, Entity entity);
	};

}