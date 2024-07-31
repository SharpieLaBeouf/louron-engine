#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <string>
#include <memory>
#include <filesystem>

// External Vendor Library Headers


namespace YAML { class Emitter; }

namespace Louron {

	// Forward Declaration
	class Scene;
	class Entity;
	class SkyboxMaterial;

	class SceneSerializer {

	public:

		SceneSerializer(std::shared_ptr<Scene> scene);

		bool Serialize(const std::filesystem::path& sceneFilePath = "");
		bool Deserialize(const std::filesystem::path& sceneFilePath);

	private:
		std::weak_ptr<Scene> m_Scene;

		void SerializeEntity(YAML::Emitter& out, Entity entity);
	};

}