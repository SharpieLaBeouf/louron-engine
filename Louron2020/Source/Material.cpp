#include "../Headers/Abstracted GL/Material.h"


GLboolean Material::Bind() {
	if (m_Shader) {
		m_Shader->Bind();
		return GL_TRUE;
	}
	else {
		std::cout << "[L20] Shader Not Linked to Material - Cannot Bind Shader!" << std::endl;
		return GL_FALSE;
	}
}
void Material::UnBind() {

	if (m_Shader) {
		for (int i = 0; i < m_Textures.size(); i++) {
			if (m_Textures[i]) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		glActiveTexture(GL_TEXTURE0);
		m_Shader->UnBind();
	}
	else {
		std::cout << "[L20] Shader Not Linked to Material - Cannot UnBind Shader!" << std::endl;
	}
}

void Material::setUniforms() {

	if (m_Shader)
	{
		m_Shader->setFloat("u_Material.shine", m_Shine);
		m_Shader->setVec4("u_Material.ambient", m_Ambient);
		m_Shader->setVec4("u_Material.diffuse", m_Diffuse);
		m_Shader->setVec4("u_Material.specular", m_Specular);

		for (int i = 0; i < m_Textures.size(); i++) {
			if (m_Textures[i]) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, m_Textures[i]->getID());
			}
		}

		glActiveTexture(GL_TEXTURE0);
	}
	else std::cout << "[L20] Shader Not Linked to Material - Cannot Set Uniforms!" << std::endl;
}

Shader* Material::getShader() { return m_Shader; }

Material::Material(Texture* texture) {
	m_Shader = nullptr;
	for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
		m_Textures[i] = texture;
}

Material::Material(Shader * shader, Texture* texture) : m_Shader(shader) {
	for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
		m_Textures[i] = texture;
}

Material::Material(Shader* shader, std::unordered_map<GLint, Texture*>& textures) : m_Shader(shader) {
	for (int i = 0; i < textures.size(); i++)
		m_Textures[i] = textures[i];
}

glm::vec4* Material::getAmbient() { return &m_Ambient; }
glm::vec4* Material::getDiffuse() { return &m_Diffuse; }
glm::vec4* Material::getSpecular() { return &m_Specular; }

void Material::setAmbient(const glm::vec4& val) { m_Ambient = val; }
void Material::setDiffuse(const glm::vec4& val) { m_Diffuse = val; }
void Material::setSpecular(const glm::vec4& val) { m_Specular = val; }

void Material::AddTextureMap(GLint type, Texture* val) {
	if (m_Shader) 
		m_Shader->setInt(std::string("u_Material." + m_TextureUniformNames[type]).c_str(), type);
	else 
		std::cout << "[L20] Shader Not Linked to Material - Cannot Set Texture Unit!" << std::endl;

	if (type < m_Textures.size() && type > -1) {
		std::cout << "[L20] Replacing Material Texture: " << m_TextureUniformNames[type] << std::endl;
		m_Textures[type] = val;
	}
	else std::cout << "[L20] Texture Slot Out of Bounds!" << std::endl;
}

Texture* Material::GetTextureMap(GLint type) {
	return m_Textures[type];
}

