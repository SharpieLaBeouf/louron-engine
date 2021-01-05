#include "../Headers/Shader.h"

void Shader::Bind() { glUseProgram(m_Program); }
void Shader::UnBind() { glUseProgram(0); }
GLuint Shader::getProgram() { return m_Program; }

Shader::Shader() { 
	m_Program = 0; 
}

Shader::Shader(const char* shaderFile)
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
Shader::~Shader() { 
	glDeleteProgram(m_Program); 
}

void Shader::setBool(const GLchar* name, bool value) const
{
	glUniform1i(glGetUniformLocation(m_Program, name), (int)value);
}

void Shader::setInt(const GLchar* name, int value) const
{
	glUniform1i(glGetUniformLocation(m_Program, name), value);
}

void Shader::setFloat(const GLchar* name, float value) const
{
	glUniform1f(glGetUniformLocation(m_Program, name), value);
}

void Shader::setVec2(const GLchar* name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(m_Program, name), 1, &value[0]);
}
void Shader::setVec2(const GLchar* name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(m_Program, name), x, y);
}

void Shader::setVec3(const GLchar* name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(m_Program, name), 1, &value[0]);
}
void Shader::setVec3(const GLchar* name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(m_Program, name), x, y, z);
}

void Shader::setVec4(const GLchar* name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(m_Program, name), 1, &value[0]);
}
void Shader::setVec4(const GLchar* name, float x, float y, float z, float w) const
{
	glUniform4f(glGetUniformLocation(m_Program, name), x, y, z, w);
}

void Shader::setMat2(const GLchar* name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat3(const GLchar* name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat4(const GLchar* name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(unsigned int shader, std::string type) {
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