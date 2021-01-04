#pragma once

#include <iostream>
#include <GLEW/glew.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include "../Headers/stb_image.h"

class Texture {

public:
	Texture() = default;
	Texture(const char* texturePath) {


		glGenTextures(1, &m_TextureID);
		glBindTexture(GL_TEXTURE_2D, m_TextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* textureData = stbi_load(texturePath, &width, &height, &nrChannels, 0);
		if (textureData) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else std::cout << "[L20] Failed to load texture!" << std::endl;

		std::cout << "[L20] Loaded Texture: " << texturePath << std::endl;
		stbi_image_free(textureData);
	}
	~Texture() {
		glDeleteTextures(1, &m_TextureID);
	}

	GLuint getID() { return m_TextureID; }

private:
	GLuint m_TextureID;

};