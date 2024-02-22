#include "Skybox.h"

// Louron Core Headers
#include "Camera.h"

#include "../../Debug/Assert.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include <stb_image/stb_image.h>

namespace Louron {
	
#pragma region SkyboxMaterial

	/// <summary>
	/// Initialise the Skybox Material in OpenGL.
	/// </summary>
	SkyboxMaterial::SkyboxMaterial() {
		glGenTextures(1, &m_SkyboxID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);

		m_MaterialShader->Bind();
		m_MaterialShader->SetInt("u_Skybox", 0);
	}

	/// <summary>
	/// Load all textures and setup the skybox.
	/// </summary>
	/// <param name="textures">A vector list of filepaths. Please ensure 
	/// filepaths are ordered as such: right, left, top, bottom, back, front.</param>
	GLboolean SkyboxMaterial::LoadSkybox(const std::array<std::filesystem::path, 6>& textures) {
		
		m_TextureFilePaths = textures;

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < m_TextureFilePaths.size(); i++)
		{
			unsigned char* data = stbi_load(m_TextureFilePaths[i].string().c_str(), &width, &height, &nrChannels, 0);
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
				std::cout << "[L20] Skybox Texture Failed to Load: " << m_TextureFilePaths[i].string() << std::endl;
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
	/// <param name="binding">This is the face of the cube map.</param>
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
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);
	}

#pragma endregion

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
				
		// Create Buffer
		VertexBuffer* vbo = new VertexBuffer(&skyboxVertices[0], sizeof(skyboxVertices));
		vbo->SetLayout({ { ShaderDataType::Float3, "aPos" } });

		// Create Vertex Array and Assign Buffer
		m_VAO = std::make_shared<VertexArray>();
		m_VAO->AddVertexBuffer(vbo);
	}
}