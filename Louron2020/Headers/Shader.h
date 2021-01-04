#pragma once

#include <GLEW/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{

private:

	GLuint m_Program;

public:

	void Bind() {  glUseProgram(m_Program); }
	void UnBind() { glUseProgram(0); }
	GLuint getProgram() { return m_Program; }

	Shader() { m_Program = 0; }
	Shader(const char* shaderFile)
	{

		std::ifstream file(shaderFile);
		
		enum class ShaderType {
			NONE = -1, VERTEX = 0, FRAGMENT = 1
		};

		std::string line; std::stringstream ss[2];
		ShaderType type = ShaderType::NONE;
		while (getline(file, line))
		{
			if (line.find("#SHADER") != std::string::npos)
			{
				if (line.find("VERTEX") != std::string::npos)
					type = ShaderType::VERTEX;
				else if (line.find("FRAGMENT") != std::string::npos)
					type = ShaderType::FRAGMENT;
			}
			else
				ss[(int)type] << line << '\n';
		}
		
		std::string vString = ss[0].str();
		std::string fString = ss[1].str();

		const char* vShaderCode = vString.c_str();
		const char* fShaderCode = fString.c_str();

		unsigned int vertex, fragment;

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");

		m_Program = glCreateProgram();
		glAttachShader(m_Program, vertex);
		glAttachShader(m_Program, fragment);
		glLinkProgram(m_Program);
		checkCompileErrors(m_Program, "PROGRAM");

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	~Shader() { glDeleteProgram(m_Program); }

	void setBool(const GLchar* name, bool value) const
	{
		glUseProgram(m_Program);
		glUniform1i(glGetUniformLocation(m_Program, name), (int)value);
	}

	void setInt(const GLchar* name, int value) const
	{
		glUseProgram(m_Program);
		glUniform1i(glGetUniformLocation(m_Program, name), value);
	}

	void setFloat(const GLchar* name, float value) const
	{
		glUseProgram(m_Program);
		glUniform1f(glGetUniformLocation(m_Program, name), value);
	}

	void setVec2(const GLchar* name, const glm::vec2& value) const
	{
		glUseProgram(m_Program);
		glUniform2fv(glGetUniformLocation(m_Program, name), 1, &value[0]);
	}
	void setVec2(const GLchar* name, float x, float y) const
	{
		glUseProgram(m_Program);
		glUniform2f(glGetUniformLocation(m_Program, name), x, y);
	}

	void setVec3(const GLchar* name, const glm::vec3& value) const
	{
		glUseProgram(m_Program);
		glUniform3fv(glGetUniformLocation(m_Program, name), 1, &value[0]);
	}
	void setVec3(const GLchar* name, float x, float y, float z) const
	{
		glUseProgram(m_Program);
		glUniform3f(glGetUniformLocation(m_Program, name), x, y, z);
	}

	void setVec4(const GLchar* name, const glm::vec4& value) const
	{
		glUseProgram(m_Program);
		glUniform4fv(glGetUniformLocation(m_Program, name), 1, &value[0]);
	}
	void setVec4(const GLchar* name, float x, float y, float z, float w)
	{
		glUseProgram(m_Program);
		glUniform4f(glGetUniformLocation(m_Program, name), x, y, z, w);
	}

	void setMat2(const GLchar* name, const glm::mat2& mat) const
	{
		glUseProgram(m_Program);
		glUniformMatrix2fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat3(const GLchar* name, const glm::mat3& mat) const
	{
		glUseProgram(m_Program);
		glUniformMatrix3fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat4(const GLchar* name, const glm::mat4& mat) const
	{
		glUseProgram(m_Program);
		glUniformMatrix4fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]);
	}

private:

    void checkCompileErrors(unsigned int shader, std::string type) {
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "[L20] ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "[L20] ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}

};
