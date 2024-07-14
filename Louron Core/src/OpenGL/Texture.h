#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>

// External Vendor Library Headers
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Louron {

	class Texture {

	public:

		Texture();
		Texture(const std::string& textureName, int textureWidth, int textureHeight);
		Texture(const std::filesystem::path& texturePath);
		Texture(GLubyte* texture_data, glm::ivec2 texture_size);
		~Texture();

	public:

		// Bind and Unbinding
		void Bind();
		void UnBind();

		// Getters and Setters
		const GLuint& GetID() const { return m_TextureId; }

		void SetWrapMode(GLint parameter);
		const GLint& GetWrapMode() const { return m_WrapMode; }

		void SetFilterMode(GLint parameter);
		const GLint& GetFilterMode() const { return m_FilterMode; }

		const GLuint& GetWidth() const { return m_Size.x; }
		const GLuint& GetHeight() const { return m_Size.y; }
		const glm::ivec2& GetSize() const { return m_Size; }

		void SetName(const std::string& name) { m_Name = name; }
		const std::string& GetName() const { return m_Name; }

		const std::filesystem::path& GetFilePath() const { return m_FilePath; }

		operator bool() const { return (m_TextureId != -1); }

	private:
		GLuint m_TextureId = -1;
		GLint m_WrapMode = GL_REPEAT;
		GLint m_FilterMode = GL_LINEAR;

		glm::ivec2 m_Size = { 0,0 };
		std::string m_Name = "New Untitled Texture";
		std::filesystem::path m_FilePath;

	};

	class TextureLibrary {

	private:

		std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;

	public:
		void UnBind();

		TextureLibrary();

		void Add(std::shared_ptr<Texture> texture);
		void Add(const std::string& textureName, std::shared_ptr<Texture> texture);

		std::shared_ptr<Texture> LoadTexture(const std::string& textureFile);
		std::shared_ptr<Texture> LoadTexture(const std::string& textureFile, const std::string& textureName);

		std::shared_ptr<Texture> GetTexture(const std::string& textureName);
		std::shared_ptr<Texture> GetDefaultTexture();
		std::shared_ptr<Texture> GetDefaultNormalTexture();

		bool TextureExists(const std::string& name) const;
	};
}