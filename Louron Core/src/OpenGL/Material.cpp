#include "Material.h"

// Louron Core Headers
#include "../Core/Engine.h"

#include "../Debug/Assert.h"

#include "../Scene/Components/Camera.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	GLboolean Material::Bind() {
		if (m_Shader) {
			m_Shader->Bind();
			return GL_TRUE;
		}
		L_CORE_ERROR("Shader Not Found for Material: {0}", GetName());
		return GL_FALSE;
	}
	void Material::UnBind() {

		for (int i = 0; i < m_Textures.size(); i++) {
			if (m_Textures[i]) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		glActiveTexture(GL_TEXTURE0); 
		glUseProgram(0);
	}

	void Material::UpdateUniforms(const Camera& camera) {

		if (m_Shader) {
			m_Shader->SetFloat("u_Material.shine", m_Shine);
			m_Shader->SetVec4("u_Material.diffuse", m_Diffuse);
			m_Shader->SetVec4("u_Material.specular", m_Specular);

			for (int i = 0; i < L20_TOTAL_ELEMENTS; i++) {

				m_Shader->SetInt(std::string("u_Material." + m_TextureUniformNames[i]).c_str(), i);
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, m_Textures[i]->GetID());
			}

			if (&camera != nullptr) {
				m_Shader->SetMat4("u_VertexIn.Proj", camera.GetProjMatrix());
				m_Shader->SetMat4("u_VertexIn.View", camera.GetViewMatrix());
				m_Shader->SetVec3("u_CameraPos", camera.GetPosition());
			}
		}
		else {
			L_CORE_ERROR("Shader Not Found for Material: {0}", GetName());
		}
	}

	void Material::SetShader(std::shared_ptr<Shader>& shader) {
		m_Shader = shader; 
	}
	void Material::SetTexture(std::shared_ptr<Texture> texture, TextureMapType textureType) {
		m_Textures[textureType] = texture; 
	}

	std::shared_ptr<Shader>& Material::GetShader() {
		return m_Shader; 
	}

	/// <summary>
	/// DEFAULT CONSTRUCTOR - Initialise Material with NO SHADER and BLANK TEXTURE
	/// </summary>
	Material::Material() : m_Shader(Engine::Get().GetShaderLibrary().GetShader("Default Shader")) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = Engine::Get().GetTextureLibrary().GetTexture("blank_texture");
	}

	/// <summary>
	/// Initialise Material with BLANK TEXTURE and CUSTOM NAME
	/// </summary>
	/// <param name="name"></param>
	/// <param name="shader"></param>
	Material::Material(const std::string& name, std::shared_ptr<Shader> shader) : m_Name(name), m_Shader(shader) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = Engine::Get().GetTextureLibrary().GetTexture("blank_texture");
	}

	/// <summary>
	/// Initialise Material with BLANK TEXTURE
	/// </summary>
	Material::Material(std::shared_ptr<Shader> shader) : m_Shader(shader) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = Engine::Get().GetTextureLibrary().GetTexture("blank_texture");
	}

	/// <summary>
	/// Initialise Material with NO SHADER and TEXTURE PARAMETER
	/// </summary>
	Material::Material(std::shared_ptr<Texture> texture) : m_Shader(Engine::Get().GetShaderLibrary().GetShader("Default Shader")) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = texture;
	}

	/// <summary>
	/// Initialise Material with SHADER and TEXTURE PARAMETER
	/// </summary>
	Material::Material(std::shared_ptr<Shader> shader, std::shared_ptr<Texture> texture) : m_Shader(shader) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = texture;
	}

	/// <summary>
	/// Initialise Material with SHADER and TEXTURE UNORDERED MAP PARAMETER
	/// </summary>
	Material::Material(std::shared_ptr<Shader> shader, std::unordered_map<GLint, std::shared_ptr<Texture>>& textures) : m_Shader(shader) {
		for (int i = 0; i < textures.size(); i++)
			m_Textures[i] = textures[i];
	}

	float Material::GetShine() const { return m_Shine; }
	glm::vec4* Material::GetDiffuse() { return &m_Diffuse; }
	glm::vec4* Material::GetSpecular() { return &m_Specular; }

	void Material::SetShine(float shine) { m_Shine = shine; }
	void Material::SetDiffuse(const glm::vec4& val) { m_Diffuse = val; }
	void Material::SetSpecular(const glm::vec4& val) { m_Specular = val; }

	void Material::AddTextureMap(GLint type, std::shared_ptr<Texture> val) {
		if (m_Shader) {
			m_Shader->Bind();
			m_Shader->SetInt(std::string("u_Material." + m_TextureUniformNames[type]).c_str(), type);
			m_Shader->UnBind();
		}
		else
			L_CORE_WARN("Shader Not Linked to Material - Cannot Set Texture Unit"); 

		if (type < m_Textures.size() && type > -1) {
			m_Textures[type] = val;
		}
		else
			L_CORE_WARN("Texture Slot Out of Bounds"); 
	}

	std::shared_ptr<Texture> Material::GetTextureMap(GLint type) {
		return m_Textures[type];
	}
}