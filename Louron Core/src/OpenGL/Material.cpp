#include "Material.h"
#include "../Core/Engine.h"

namespace Louron {

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

	void Material::SetUniforms() {

		if (m_Shader)
		{
			m_Shader->SetFloat("u_Material.shine", m_Shine);
			m_Shader->SetVec4("u_Material.ambient", m_Ambient);
			m_Shader->SetVec4("u_Material.diffuse", m_Diffuse);
			m_Shader->SetVec4("u_Material.specular", m_Specular);

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

	void Material::SetShader(Shader* shader) { 
		m_Shader = shader; 
	}
	void Material::SetTexture(Texture* texture, TextureMapType textureType) { 
		m_Textures[textureType] = texture; 
	}

	Shader* Material::GetShader() { 
		return m_Shader; 
	}

	/// <summary>
	/// DEFAULT CONSTRUCTOR - Initialise Material with NO SHADER and BLANK TEXTURE
	/// </summary>
	Material::Material() : m_Shader(nullptr) { 
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = Engine::Get().GetTextureLibrary().GetTexture("blank_texture");
	}

	/// <summary>
	/// Initialise Material with BLANK TEXTURE
	/// </summary>
	Material::Material(Shader* shader) {
		m_Shader = shader;
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = Engine::Get().GetTextureLibrary().GetTexture("blank_texture");
	}

	/// <summary>
	/// Initialise Material with NO SHADER and TEXTURE PARAMETER
	/// </summary>
	Material::Material(Texture* texture) {
		m_Shader = nullptr;
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = texture;
	}

	/// <summary>
	/// Initialise Material with SHADER and TEXTURE PARAMETER
	/// </summary>
	Material::Material(Shader* shader, Texture* texture) : m_Shader(shader) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = texture;
	}

	/// <summary>
	/// Initialise Material with SHADER and TEXTURE UNORDERED MAP PARAMETER
	/// </summary>
	Material::Material(Shader* shader, std::unordered_map<GLint, Texture*>& textures) : m_Shader(shader) {
		for (int i = 0; i < textures.size(); i++)
			m_Textures[i] = textures[i];
	}

	// TODO: consider changing these to return references to a unique_ptr which holds the ownership of the details within the class
	float Material::GetShine() { return m_Shine; }
	glm::vec4* Material::GetAmbient() { return &m_Ambient; }
	glm::vec4* Material::GetDiffuse() { return &m_Diffuse; }
	glm::vec4* Material::GetSpecular() { return &m_Specular; }

	void Material::SetShine(float shine) { m_Shine = shine; }
	void Material::SetAmbient(const glm::vec4& val) { m_Ambient = val; }
	void Material::SetDiffuse(const glm::vec4& val) { m_Diffuse = val; }
	void Material::SetSpecular(const glm::vec4& val) { m_Specular = val; }

	void Material::AddTextureMap(GLint type, Texture* val) {
		if (m_Shader) {
			m_Shader->Bind();
			m_Shader->SetInt(std::string("u_Material." + m_TextureUniformNames[type]).c_str(), type);
			m_Shader->UnBind();
		}
		else std::cout << "[L20] Shader Not Linked to Material - Cannot Set Texture Unit!" << std::endl;

		if (type < m_Textures.size() && type > -1) {
			m_Textures[type] = val;
		}
		else std::cout << "[L20] Texture Slot Out of Bounds!" << std::endl;
	}

	Texture* Material::GetTextureMap(GLint type) {
		return m_Textures[type];
	}

	void Material::SetMaterialIndex(GLuint index) {
		m_MaterialIndex = index;
	}

	GLuint Material::GetMaterialIndex()
	{
		return m_MaterialIndex;
	}
}