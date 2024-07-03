#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

// External Vendor Library Headers
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Louron {

	class Shader {

	private:

		GLuint m_Program = -1;
		std::string m_Name;

	public:

		void Bind();
		void UnBind();

		Shader(const GLchar* shaderFile, bool isComputeShader);
		~Shader();

		GLuint GetProgram();
		const std::string& GetName();

		void SetName(const std::string& name);

		void SetBool(const GLchar* name, bool value) const;
		void SetInt(const GLchar* name, int value) const;
		void SetUInt(const GLchar* name, GLuint value) const;
		void SetFloat(const GLchar* name, float value) const;
		void SetiVec2(const GLchar* name, const glm::ivec2& value) const;
		void SetiVec3(const GLchar* name, const glm::ivec3& value) const;
		void SetiVec4(const GLchar* name, const glm::ivec4& value) const; 
		void SetVec2(const GLchar* name, const glm::vec2& value) const;
		void SetVec3(const GLchar* name, const glm::vec3& value) const;
		void SetVec4(const GLchar* name, const glm::vec4& value) const;
		void SetMat2(const GLchar* name, const glm::mat2& mat) const;
		void SetMat3(const GLchar* name, const glm::mat3& mat) const;
		void SetMat4(const GLchar* name, const glm::mat4& mat) const;

	private:

		void checkCompileErrors(unsigned int shader, std::string type);
	};

	class ShaderLibrary {

	public:

		void UnBindAllShaders();

		ShaderLibrary();

		std::shared_ptr<Shader>& GetShader(const std::string& shaderName);
		std::shared_ptr<Shader>& LoadShader(const std::string& shaderFile, bool isComputeShader = false);

		const std::shared_ptr<Shader>& GetDefault() { return m_DefaultShader; }

		bool ShaderExists(const std::string& name) const;

	private:

		std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
		std::shared_ptr<Shader> m_DefaultShader;

	private:
		std::string FilePathToShaderName(const std::string& shaderFile);
	};
}