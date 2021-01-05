#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GLEW/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Shader
{

private:

	GLuint m_Program;

public:

	void Bind();
	void UnBind();
	GLuint getProgram();

	Shader();
	Shader(const char* shaderFile);
	~Shader();

	void setBool(const GLchar* name, bool value) const;
	void setInt(const GLchar* name, int value) const;
	void setFloat(const GLchar* name, float value) const;
	void setVec2(const GLchar* name, const glm::vec2& value) const;
	void setVec2(const GLchar* name, float x, float y) const;
	void setVec3(const GLchar* name, const glm::vec3& value) const;
	void setVec3(const GLchar* name, float x, float y, float z) const;
	void setVec4(const GLchar* name, const glm::vec4& value) const;
	void setVec4(const GLchar* name, float x, float y, float z, float w) const;
	void setMat2(const GLchar* name, const glm::mat2& mat) const;
	void setMat3(const GLchar* name, const glm::mat3& mat) const;
	void setMat4(const GLchar* name, const glm::mat4& mat) const;

private:

	void checkCompileErrors(unsigned int shader, std::string type);

};
