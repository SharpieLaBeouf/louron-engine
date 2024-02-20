#include "Skybox.h"

#include "../Core/Engine.h"

#include "../Debug/Assert.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include <stb_image/stb_image.h>

namespace Louron {
	
	/// <summary>
	/// Initialise the Skybox Material in OpenGL.
	/// </summary>
	SkyboxMaterial::SkyboxMaterial() {
		glGenTextures(1, &m_SkyboxID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);

		//std::shared_ptr<Texture> tex = std::make_shared<Texture>();
		//for (unsigned int i = 0; i < 6; i++) {

		//	m_SkyboxTextures[i] = tex;
		//	m_SkyboxTextures[i]->SetWrapMode(GL_CLAMP_TO_EDGE);
		//	glCopyTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, 0, 0, m_SkyboxTextures[i]->GetWidth(), m_SkyboxTextures[i]->GetHeight());
		//}

		m_MaterialShader = Engine::Get().GetShaderLibrary().GetShader("Skybox");
		m_MaterialShader->Bind();
		m_MaterialShader->SetInt("u_Skybox", 0);

	}

	/// <summary>
	/// Load all textures and setup the skybox.
	/// </summary>
	/// <param name="textures">A vector list of filepaths. Please ensure 
	/// filepaths are ordered as such: right, left, top, bottom, back, front.</param>
	/// <returns></returns>
	GLboolean SkyboxMaterial::LoadSkybox(const std::vector<std::filesystem::path>& textures) {
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			unsigned char* data = stbi_load(textures[i].string().c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				GLenum format = GL_RGBA;

				switch (nrChannels) {
				case 1:
					format = GL_RED;
					break;
				case 3:
					format = GL_RGB;
					break;
				case 4:
					format = GL_RGBA;
					break;
				}
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "[L20] Skybox Texture Failed to Load: " << textures[i].string() << std::endl;
				stbi_image_free(data);
				return GL_FALSE;
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		std::cout << "[L20] All Skybox Textures Loaded Successfully" << std::endl;
		return GL_TRUE;
	}

	/// <summary>
	/// Load individual texture for indiviaul skybox face.
	/// </summary>
	/// <param name="binding"></param>
	/// <param name="filePath"></param>
	/// <returns></returns>
	GLboolean SkyboxMaterial::LoadSkyboxTexture(const L_SKYBOX_BINDING& binding, const std::filesystem::path& filePath)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);

		int width, height, nrChannels;

		unsigned char* data = stbi_load(filePath.string().c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format = GL_RGBA;

			switch (nrChannels) {
			case 1:
				format = GL_RED;
				break;
			case 3:
				format = GL_RGB;
				break;
			case 4:
				format = GL_RGBA;
				break;
			}
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + binding, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "[L20] Skybox Texture Failed to Load: " << filePath.string() << std::endl;
			stbi_image_free(data);
			return GL_FALSE;
		}
		
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		std::cout << "[L20] Skybox Texture Loaded Successfully: " << filePath.filename().replace_extension().string() << std::endl;
		return GL_TRUE;
	}

	void SkyboxMaterial::UpdateUniforms(const Camera& camera) {

		L_CORE_ASSERT(m_MaterialShader, "Error Updating Uniforms, Shader Not Found for Skybox Material: " + m_MaterialName);
		if (m_MaterialShader && &camera != nullptr) {
			m_MaterialShader->SetMat4("u_VertexIn.Proj", camera.GetProjMatrix());
			glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
			m_MaterialShader->SetMat4("u_VertexIn.View", view);
		}

	}

	GLboolean SkyboxMaterial::Bind() {
		L_CORE_ASSERT(m_MaterialShader, "Shader Not Found for Material: " + this->GetName());
		if (m_MaterialShader) {
			m_MaterialShader->Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);
			return GL_TRUE;
		}
		return GL_FALSE;
	}
	void SkyboxMaterial::UnBind() {

		for (int i = 0; i < m_SkyboxTextures.size(); i++) {
			if (m_SkyboxTextures[i]) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);
	}

	SkyboxComponent::SkyboxComponent() {

		float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};

		VertexBuffer* vbo = new VertexBuffer(skyboxVertices, sizeof(skyboxVertices));
		
		BufferLayout layout = {
			{ ShaderDataType::Float3, "aPos" }
		};
		
		vbo->SetLayout(layout);
		m_VAO.AddVertexBuffer(vbo);
	}

}