#include "../Headers/Abstracted GL/Material.h"


void Material::Bind() {
	m_Shader->Bind();
}
void Material::UnBind() {

	for (int i = 0; i < m_Textures.size(); i++) {
		if (m_Textures[i]) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	m_Shader->UnBind();
}

void Material::setUniforms() {

	this->Bind();
	this->m_Shader->setFloat("u_Material.shine", m_Shine);
	this->m_Shader->setVec4("u_Material.ambient", m_Ambient);
	this->m_Shader->setVec4("u_Material.diffuse", m_Diffuse);
	this->m_Shader->setVec4("u_Material.specular", m_Specular);

	for (int i = 0; i < m_Textures.size(); i++) {
		if (m_Textures[i]) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, m_Textures[i]->getID());
		}
	}

	glActiveTexture(GL_TEXTURE0);
}

bool Material::shaderActive() { return (m_Shader) ? true : false; }
Shader* Material::getShader() { return m_Shader; }

Material::Material(Texture* texture) {
	m_Shader = nullptr;
	for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
		m_Textures[i] = texture;
}

Material::Material(Shader * shader, Texture* texture) : m_Shader(shader) {
	for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
	{
		m_Textures[i] = texture;

	}
}

Material::Material(Shader* shader, std::unordered_map<int, Texture*>& textures) : m_Shader(shader) {
	for (int i = 0; i < textures.size(); i++)
		m_Textures[i] = textures[i];
}

glm::vec4* Material::getAmbient() { return &m_Ambient; }
glm::vec4* Material::getDiffuse() { return &m_Diffuse; }
glm::vec4* Material::getSpecular() { return &m_Specular; }

void Material::setAmbient(const glm::vec4& val) { m_Ambient = val; }
void Material::setDiffuse(const glm::vec4& val) { m_Diffuse = val; }
void Material::setSpecular(const glm::vec4& val) { m_Specular = val; }

void Material::AddTextureMap(int type, Texture* val) {
	if (m_Textures[type])
		std::cout << "[L20] Replacing Texture: SLOT " << type << std::endl;
	m_Textures[type] = val;
	m_Shader->setInt(std::string("u_Material." + m_TextureUniformNames[type]).c_str(), type);
}

Texture* Material::GetTextureMap(int type) {
	return m_Textures[type];
}

