#include "../Headers/Abstracted GL/Texture.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include "../Vendor/stb_image.h"

void Texture::Bind() {
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void Texture::UnBind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(const char* texturePath) {


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
	Texture* texture = new Texture(textureFile.c_str());
	Add(texture);
	return texture;
}

Texture* TextureLibrary::loadTexture(const std::string& textureFile, const std::string& textureName) {
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
