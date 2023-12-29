#include "Shader.h"


namespace Louron {

	void Shader::Bind() { glUseProgram(m_Program); }
	void Shader::UnBind() { glUseProgram(0); }

	Shader::Shader(const GLchar* shaderFile, bool isComputeShader = false)
	{

		std::ifstream file(shaderFile);

		if (!isComputeShader) {

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

			const GLchar* vShaderCode = vString.c_str();
			const GLchar* fShaderCode = fString.c_str();

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

			std::string name = shaderFile;
			auto lastSlash = name.find_last_of("/\\");
			lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
			auto lastDot = name.rfind('.');
			auto count = lastDot == std::string::npos ? name.size() - lastSlash : lastDot - lastSlash;
			name = name.substr(lastSlash, count);
			m_Name = name.c_str();

		}
		else {
			std::string cShaderCodeSrc;
			std::stringstream cShaderCodeStream;
			cShaderCodeStream << file.rdbuf();
			file.close();

			cShaderCodeSrc = cShaderCodeStream.str();

			const GLchar* cShaderCode = cShaderCodeSrc.c_str();

			GLuint compute;

			compute = glCreateShader(GL_COMPUTE_SHADER);
			glShaderSource(compute, 1, &cShaderCode, NULL);
			glCompileShader(compute);
			checkCompileErrors(compute, "COMPUTE");

			m_Program = glCreateProgram();
			glAttachShader(m_Program, compute);

			glLinkProgram(m_Program);
			checkCompileErrors(m_Program, "PROGRAM");

			glDeleteShader(compute);

			std::string name = shaderFile;
			auto lastSlash = name.find_last_of("/\\");
			lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
			auto lastDot = name.rfind('.');
			auto count = lastDot == std::string::npos ? name.size() - lastSlash : lastDot - lastSlash;
			name = name.substr(lastSlash, count);
			m_Name = name.c_str();
		}

		std::cout << "[L20] Loaded Shader (" << m_Name << "): " << shaderFile << std::endl;
	}
	Shader::~Shader() { glDeleteProgram(m_Program); }

	GLuint Shader::GetProgram() { return m_Program; }
	const std::string& Shader::GetName() { return m_Name; }
	void Shader::SetName(const std::string& name) { m_Name = name; }
	void Shader::SetBool(const GLchar* name, bool value) const { glUniform1i(glGetUniformLocation(m_Program, name), (int)value); }
	void Shader::SetInt(const GLchar* name, int value) const { glUniform1i(glGetUniformLocation(m_Program, name), value); }
	void Shader::SetFloat(const GLchar* name, float value) const { glUniform1f(glGetUniformLocation(m_Program, name), value); }
	void Shader::SetVec2(const GLchar* name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetVec3(const GLchar* name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetVec4(const GLchar* name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetMat2(const GLchar* name, const glm::mat2& mat) const { glUniformMatrix2fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]); }
	void Shader::SetMat3(const GLchar* name, const glm::mat3& mat) const { glUniformMatrix3fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]); }
	void Shader::SetMat4(const GLchar* name, const glm::mat4& mat) const { glUniformMatrix4fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]); }

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

	void ShaderLibrary::UnBind() { glUseProgram(0); }

	void ShaderLibrary::Add(Shader* shader) {
		Add(shader->GetName(), shader);
	}

	void ShaderLibrary::Add(const std::string& shaderName, Shader* shader) {
		if (ShaderExists(shaderName)) {

			std::cout << "[L20] Shader Already Loaded! " << shaderName << std::endl;
		}
		else {
			m_Shaders[shaderName] = shader;
		}
	}

	Shader* ShaderLibrary::LoadShader(const std::string& shaderFile, bool isComputeShader) {
		Shader* shader = new Shader(shaderFile.c_str(), isComputeShader);
		Add(shader);
		return shader;
	}

	Shader* ShaderLibrary::LoadShader(const std::string& shaderFile, const std::string& shaderName, bool isComputeShader) {
		Shader* shader = new Shader(shaderFile.c_str(), isComputeShader);
		Add(shaderName, shader);
		return shader;
	}

	// TODO: shaderlibrary should hold ownership of all the Shader objects initialised. I should return a reference to a unique_ptr opposed to a raw pointer.
	Shader* ShaderLibrary::GetShader(const std::string& shaderName) {
		if (!ShaderExists(shaderName))
			std::cout << "[L20] Shader Not Loaded! " << shaderName << std::endl;
		else
			return m_Shaders[shaderName];
		return nullptr;
	}

	bool ShaderLibrary::ShaderExists(const std::string& name) const {
		return m_Shaders.find(name) != m_Shaders.end();
	}
}