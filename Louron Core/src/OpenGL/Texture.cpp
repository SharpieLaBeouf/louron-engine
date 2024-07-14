#include "Texture.h"

// Louron Core Headers
#include "../Core/Logging.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include <stb_image/stb_image.h>

#include <glad/glad.h>

namespace Louron {

#pragma region Texture

	Texture::Texture() {

		m_Size = { 1,1 };

		GLubyte texture_data[] = { 255, 255, 255, 255 };

		glGenTextures(1, &m_TextureId);
		glBindTexture(GL_TEXTURE_2D, m_TextureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Size.x, m_Size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	}

	Texture::Texture(GLubyte* texture_data, glm::ivec2 texture_size) {

		m_Size = texture_size;

		glGenTextures(1, &m_TextureId);
		glBindTexture(GL_TEXTURE_2D, m_TextureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Size.x, m_Size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	}

	Texture::Texture(const std::string& textureName, int textureWidth, int textureHeight) {

		m_Name = textureName;
		m_Size = { textureWidth, textureHeight };

		glGenTextures(1, &m_TextureId);
		glBindTexture(GL_TEXTURE_2D, m_TextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Size.x, m_Size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}

	Texture::Texture(const std::filesystem::path& texturePath) {

		glGenTextures(1, &m_TextureId);

		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* textureData = stbi_load(texturePath.string().c_str(), &width, &height, &nrChannels, 0);

		if (textureData)
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

			this->Bind();
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, textureData);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_WrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_WrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, m_WrapMode);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_FilterMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_FilterMode);

			m_Name = texturePath.filename().replace_extension().string();
			m_Size.x = width;
			m_Size.y = height;

			L_CORE_INFO("Texture Loaded: {0}", texturePath.string());
			stbi_image_free(textureData);
			this->UnBind();
		}
		else {
			L_CORE_WARN("Texture Not Loaded: {0}", texturePath.string());
			stbi_image_free(textureData);

			glDeleteTextures(1, &m_TextureId);
			m_TextureId = -1;
		}
	}

	Texture::~Texture()
	{
		if (m_TextureId != -1)
			glDeleteTextures(1, &m_TextureId);
	}

	void Texture::Bind() {
		glBindTexture(GL_TEXTURE_2D, m_TextureId);
	}

	void Texture::UnBind() {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::SetWrapMode(GLint parameter) {
		m_WrapMode = parameter;

		this->Bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_WrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_WrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, m_WrapMode);
	}

	void Texture::SetFilterMode(GLint parameter) {
		m_FilterMode = parameter;

		this->Bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_FilterMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_FilterMode);
	}	

#pragma endregion

	// TODO: Fix This Up
#pragma region TextureLibrary


	void TextureLibrary::UnBind() { 
		glBindTexture(GL_TEXTURE_2D, 0); 
	}

	TextureLibrary::TextureLibrary() {

		GLubyte texture_data[] = { 255, 255, 255, 255 };
		std::shared_ptr<Texture> default_texture = std::make_shared<Texture>(texture_data, glm::ivec2{ 1, 1 });
		default_texture->SetName("Default_Texture");

		texture_data[0] = 128;
		texture_data[1] = 128;

		std::shared_ptr<Texture> default_normal_texture = std::make_shared<Texture>(texture_data, glm::ivec2{ 1, 1 });
		default_normal_texture->SetName("Default_Normal_Texture");

		m_Textures[default_texture->GetName()] = default_texture;
		m_Textures[default_normal_texture->GetName()] = default_normal_texture;
	}

	void TextureLibrary::Add(std::shared_ptr<Texture> texture) {
		Add(texture->GetName(), texture);
	}

	void TextureLibrary::Add(const std::string& textureName, std::shared_ptr<Texture> texture) {
		if (TextureExists(textureName)) {
			L_CORE_INFO("Texture Already Loaded: {0}", textureName);
		}
		else {
			m_Textures[textureName] = texture;
		}
	}

	std::shared_ptr<Texture> TextureLibrary::LoadTexture(const std::string& textureFile) {

		std::string texture_name = textureFile;
		auto lastSlash = texture_name.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = texture_name.rfind('.');
		auto count = lastDot == std::string::npos ? texture_name.size() - lastSlash : lastDot - lastSlash;
		texture_name = texture_name.substr(lastSlash, count);

		if (TextureExists(texture_name)) {
			L_CORE_INFO("Texture Already Loaded: {0}", texture_name);
			return GetTexture(texture_name);
		}

		std::shared_ptr<Texture> texture = std::make_shared<Texture>(textureFile.c_str());
		Add(texture);
		return texture;
	}

	std::shared_ptr<Texture> TextureLibrary::LoadTexture(const std::string& textureFile, const std::string& textureName) {

		if (TextureExists(textureName)) {
			L_CORE_INFO("Texture Already Loaded: {0}", textureName);
			return GetTexture(textureName);
		}

		std::shared_ptr<Texture> texture = std::make_shared<Texture>(textureFile.c_str());
		Add(textureName, texture);
		return texture;
	}

	std::shared_ptr<Texture> TextureLibrary::GetTexture(const std::string& textureName) {
		if (!TextureExists(textureName))
			L_CORE_WARN("Texture Not Loaded: {0}", textureName);
		else
			return m_Textures[textureName];
		return nullptr;
	}

	std::shared_ptr<Texture> TextureLibrary::GetDefaultTexture() {
		return GetTexture("Default_Texture");
	}

	std::shared_ptr<Texture> TextureLibrary::GetDefaultNormalTexture() {
		return GetTexture("Default_Normal_Texture");
	}

	bool TextureLibrary::TextureExists(const std::string& name) const {
		return m_Textures.find(name) != m_Textures.end();
	}

#pragma endregion

}