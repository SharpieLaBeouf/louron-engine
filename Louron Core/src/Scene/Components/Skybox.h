#pragma once

// Louron Core Headers
#include "../../Asset/Asset Manager API.h"
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

	enum L_SKYBOX_BINDING : uint8_t {
		RIGHT = 0,
		LEFT,
		TOP,
		BOTTOM,
		BACK,
		FRONT
	};

	class SkyboxMaterial : public Material {

	public:

		// Constructors and Logic

		SkyboxMaterial();
		~SkyboxMaterial();

		void SetSkyboxFaceTexture(const L_SKYBOX_BINDING& binding, const AssetHandle& texture_asset_handle);

		void UpdateUniforms(std::shared_ptr<MaterialUniformBlock> custom_uniform_block = nullptr) override;

		virtual AssetType GetType() const override { return AssetType::Material_Skybox; }

		void Serialize(YAML::Emitter& out) const override;
		bool Deserialize(const std::filesystem::path& path = "") override;

		// Bind and Unbinding
		bool Bind() const override;
		void UnBind() const override;

		void ConstructSkyboxCubeMap();

		const std::array<AssetHandle, 6>& GetTextureAssetHandles() const { return m_TextureAssetHandles; }

	private:

		GLuint m_SkyboxID = -1;
		std::array<AssetHandle, 6> m_TextureAssetHandles{ NULL_UUID };

	};


	struct SkyboxComponent : public Component {

	public:

		AssetHandle SkyboxMaterialAssetHandle = NULL_UUID;

	public:

		SkyboxComponent();
		SkyboxComponent(const SkyboxComponent&) = default;

		void Bind() { m_VAO->Bind(); }
		void UnBind() { glBindVertexArray(0); }

		void Serialize(YAML::Emitter& out) const;
		bool Deserialize(const YAML::Node data);

	private:

		std::shared_ptr<VertexArray> m_VAO = nullptr;

	};

}