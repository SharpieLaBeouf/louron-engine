#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Louron {

	class Shader {

	private:

		GLuint m_Program;
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
		void SetFloat(const GLchar* name, float value) const;
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

	private:

		std::unordered_map<std::string, Shader*> m_Shaders;

	public:

		void UnBind();

		ShaderLibrary() = default;

		void Add(Shader* shader);
		void Add(const std::string& shaderName, Shader* shader);

		Shader* LoadShader(const std::string& shaderFile, bool isComputeShader = false);
		Shader* LoadShader(const std::string& shaderFile, const std::string& shaderName, bool isComputeShader = false);

		Shader* GetShader(const std::string& shaderName);

		bool ShaderExists(const std::string& name) const;
	};
}