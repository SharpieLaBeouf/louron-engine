#pragma once

// Louron Core Headers
#include "../../OpenGL/Material.h"
#include "../../OpenGL/Vertex Array.h"
		  
#include "../../Core/Engine.h"

#include "Components.h"

// C++ Standard Library Headers
#include <filesystem>

// External Vendor Library Headers

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
		~SkyboxMaterial() = default;

		GLboolean LoadSkybox(const std::array<std::filesystem::path, 6>& textures);
		GLboolean LoadMaterial(const std::array<std::filesystem::path, 6>& materialFilePath) { return false; } // TODO: IMPLEMENT
		GLboolean LoadSkyboxTexture(const L_SKYBOX_BINDING& binding, const std::filesystem::path& filePath);

		void UpdateUniforms(const Camera& camera);

	public:

		// Bind and Unbinding
		GLboolean Bind();
		void UnBind();


		void SetMaterialFilePath(const std::filesystem::path& filePath) { m_MaterialFilePath = filePath; }
		const std::filesystem::path& GetMaterialFilePath() const { return m_MaterialFilePath; }

		const std::array<std::filesystem::path, 6>& GetTextureFilePaths() const { return m_TextureFilePaths; }

		void SetName(const std::string& name) { m_MaterialName = name; }
		const std::string& GetName() const { return m_MaterialName; }

	private:

		std::string m_MaterialName = "New Skybox Material";

		GLuint m_SkyboxID = -1;
		std::shared_ptr<Shader> m_MaterialShader = Engine::Get().GetShaderLibrary().GetShader("Skybox");

		std::filesystem::path m_MaterialFilePath;
		std::array<std::filesystem::path, 6> m_TextureFilePaths;
	};


	struct SkyboxComponent : public Component {

	public:

		std::shared_ptr<SkyboxMaterial> Material = std::make_shared<SkyboxMaterial>();

	public:

		SkyboxComponent();
		SkyboxComponent(const SkyboxComponent&) = default;

		void Bind() { m_VAO->Bind(); }
		void UnBind() { 
			Material->UnBind();
			glBindVertexArray(0); 
		}

	private:

		std::shared_ptr<VertexArray> m_VAO = nullptr;

	};

}