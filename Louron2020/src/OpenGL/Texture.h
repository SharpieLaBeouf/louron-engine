#pragma once

#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <glad/glad.h>

class Texture {

public:

	void Bind();
	void UnBind();

	Texture();
	Texture(const char* texturePath);
	~Texture();

	GLuint getID();
	std::string getName();

private:
	GLuint m_TextureID;
	std::string m_Name;

};

class TextureLibrary {

private:

	std::unordered_map<std::string, Texture*> m_Textures;

public:
	void UnBind();

	TextureLibrary();

	void Add(Texture* texture);
	void Add(const std::string & textureName, Texture* texture);

	Texture* loadTexture(const std::string & textureFile);
	Texture* loadTexture(const std::string & textureFile, const std::string & textureName);

	Texture* getTexture(const std::string & textureName);

	bool textureExists(const std::string & name) const;
};