#pragma once

// Louron Core Headers
#include "../../OpenGL/Material.h"
#include "../../OpenGL/Vertex Array.h"
		  
#include "../../Core/Engine.h"

#include "Components.h"

// C++ Standard Library Headers
#include <filesystem>

// External Vendor Library Headers


class YAML::Emitter;
class YAML::Node;

namespace Louron {

	enum L_SKYBOX_BINDING {
		RIGHT = 0,
		LEFT = 1,
		TOP = 2,
		BOTTOM = 3,
		BACK = 4,
		FRONT = 5
	};

	class SkyboxMaterial : public Material {

	public:

		// Constructors and Logic

		SkyboxMaterial();
		~SkyboxMaterial();

		void SetSkyboxFaceTexture(const L_SKYBOX_BINDING& binding, const AssetHandle& texture_asset_handle);

		void UpdateUniforms(Camera* camera);

		virtual AssetType GetType() const override { return AssetType::Material_Skybox; }

		void Serialize(const std::filesystem::path& path = "");
		bool Deserialize(const std::filesystem::path& path = "");

		// Bind and Unbinding
		GLboolean Bind();
		void UnBind();

		void ConstructSkyboxCubeMap();

	private:

		GLuint m_SkyboxID = -1;

		std::array<AssetHandle, 6> m_TextureAssetHandles{ NULL_UUID };

		std::shared_ptr<Shader> m_MaterialShader = Engine::Get().GetShaderLibrary().GetShader("Skybox");

	};


	struct SkyboxComponent : public Component {

	public:

		AssetHandle SkyboxMaterialAssetHandle = NULL_UUID;

	public:

		SkyboxComponent();
		SkyboxComponent(const SkyboxComponent&) = default;

		void Bind() { m_VAO->Bind(); }
		void UnBind() { glBindVertexArray(0); }

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);

	private:

		std::shared_ptr<VertexArray> m_VAO = nullptr;

	};

}