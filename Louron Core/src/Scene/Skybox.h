#pragma once

#include "../OpenGL/Material.h"

#include "../OpenGL/Vertex Array.h"

#include <filesystem>

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

		GLboolean LoadSkybox(const std::vector<std::filesystem::path>& textures);
		GLboolean LoadSkyboxTexture(const L_SKYBOX_BINDING& binding, const std::filesystem::path& filePath);

		void UpdateUniforms(const Camera& camera);

	public:

		// Bind and Unbinding
		GLboolean Bind();
		void UnBind();

		// Getters and Setters
		void SetSkyboxTexture(const L_SKYBOX_BINDING& binding, std::shared_ptr<Texture> texture) { m_SkyboxTextures[binding] = texture; }
		std::shared_ptr<Texture> GetSkyboxTexture(const L_SKYBOX_BINDING& binding) { return m_SkyboxTextures[binding]; }

		std::array<std::shared_ptr<Texture>, 6> m_SkyboxTextures;
	private:

		std::string m_MaterialName = "New Skybox Material";

		GLuint m_SkyboxID = -1;
		std::shared_ptr<Shader> m_MaterialShader;
	};


	struct SkyboxComponent {

	public:

		SkyboxMaterial Material;

		SkyboxComponent();
		SkyboxComponent(const SkyboxComponent&) = default;

		void Bind() { m_VAO.Bind(); }
		void UnBind() { m_VAO.Unbind(); }

	private:

		VertexArray m_VAO;

	};

}