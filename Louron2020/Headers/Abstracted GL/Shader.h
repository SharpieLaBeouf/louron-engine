#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <GLEW/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Shader {

private:

	GLuint m_Program;
	std::string m_Name;

public:

	void Bind();
	void UnBind();

	Shader(const GLchar* shaderFile);
	~Shader();

	GLuint getProgram();
	const std::string& getName();

	void setName(const std::string& name);

	void setBool	(const GLchar* name, bool value) const;
	void setInt		(const GLchar* name, int value) const;
	void setFloat	(const GLchar* name, float value) const;
	void setVec2	(const GLchar* name, const glm::vec2& value) const;
	void setVec3	(const GLchar* name, const glm::vec3& value) const;
	void setVec4	(const GLchar* name, const glm::vec4& value) const;
	void setMat2	(const GLchar* name, const glm::mat2& mat) const;
	void setMat3	(const GLchar* name, const glm::mat3& mat) const;
	void setMat4	(const GLchar* name, const glm::mat4& mat) const;

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
	
	Shader* loadShader(const std::string& shaderFile);
	Shader* loadShader(const std::string& shaderFile, const std::string& shaderName);

	Shader* getShader(const std::string& shaderName);

	bool shaderExists(const std::string& name) const;
};
