#pragma once

#include <iostream>

#include <GLEW/glew.h>

class Texture {

public:
	Texture() = default;
	Texture(const char* texturePath);
	~Texture();

	GLuint getID();

private:
	GLuint m_TextureID;

};