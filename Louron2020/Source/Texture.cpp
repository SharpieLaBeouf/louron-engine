#include "../Headers/Abstracted GL/Texture.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include "../include/stb_image.h"

void Texture::Bind() {
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void Texture::UnBind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}


Texture::Texture(const char* texturePath) {


	glGenTextures(1, &m_TextureID);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* textureData = stbi_load(texturePath, &width, &height, &nrChannels, 0);
	
	if (textureData)
	{
		GLenum format = GL_RGBA;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		
		glBindTexture(GL_TEXTURE_2D, m_TextureID); 
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, textureData);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		std::string name = texturePath;
		auto lastSlash = name.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = name.rfind('.');
		auto count = lastDot == std::string::npos ? name.size() - lastSlash : lastDot - lastSlash;
		name = name.substr(lastSlash, count);
		m_Name = name.c_str();

		std::cout << "[L20] Loaded Texture: " << texturePath << std::endl;
		stbi_image_free(textureData);
	}
	else {
		std::cout << "[L20] Failed to load texture!" << std::endl;
	}
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_TextureID);
}

GLuint Texture::getID()
{
	return m_TextureID;
}

std::string Texture::getName() { return m_Name; }

void TextureLibrary::UnBind() { glBindTexture(GL_TEXTURE_2D, 0); }


Texture::Texture() {

	m_Name = "blank_texture";
	GLubyte data[] = { 255, 255, 255, 255 };

	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

TextureLibrary::TextureLibrary() {
	Texture* blankTex = new Texture();
	m_Textures[blankTex->getName()] = blankTex;
}

void TextureLibrary::Add(Texture* texture) {
	Add(texture->getName(), texture);
}

void TextureLibrary::Add(const std::string& textureName, Texture* texture) {
	if (textureExists(textureName)) {
		std::cout << "[L20] Texture Already Loaded! " << textureName << std::endl;
	}
	else {
		m_Textures[textureName] = texture;
	}
}

Texture* TextureLibrary::loadTexture(const std::string& textureFile) {

	std::string texture_name = textureFile;
	auto lastSlash = texture_name.find_last_of("/\\");
	lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
	auto lastDot = texture_name.rfind('.');
	auto count = lastDot == std::string::npos ? texture_name.size() - lastSlash : lastDot - lastSlash;
	texture_name = texture_name.substr(lastSlash, count);

	if (textureExists(texture_name)) {
		std::cout << "[L20] Texture Already Loaded: " << texture_name << std::endl;
		return getTexture(texture_name);
	}

	Texture* texture = new Texture(textureFile.c_str());
	Add(texture);
	return texture;
}

Texture* TextureLibrary::loadTexture(const std::string& textureFile, const std::string& textureName) {

	if (textureExists(textureName)) {
		std::cout << "[L20] Texture Already Loaded: " << textureName << std::endl;
		return getTexture(textureName);
	}
	
	Texture* texture = new Texture(textureFile.c_str());
	Add(textureName, texture);
	return texture;
}

Texture* TextureLibrary::getTexture(const std::string& textureName) {
	if (!textureExists(textureName))
		std::cout << "[L20] Texture Not Loaded! " << textureName << std::endl;
	else
		return m_Textures[textureName];
	return nullptr;
}

bool TextureLibrary::textureExists(const std::string& name) const {
	return m_Textures.find(name) != m_Textures.end();
}
