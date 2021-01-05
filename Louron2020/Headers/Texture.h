#pragma once

#include <iostream>

#include <GLEW/glew.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include "../Headers/stb_image.h"

class Texture {

public:
	Texture() = default;
	Texture(const char* texturePath);
	~Texture();

	GLuint getID();

private:
	GLuint m_TextureID;

};