#include "Shader.h"

// Louron Core Headers
#include "../Core/Logging.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

// -- Shader --

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
		}
	}

	Shader::~Shader() { glDeleteProgram(m_Program); }

	GLuint Shader::GetProgram() { return m_Program; }

	const std::string& Shader::GetName() { return m_Name; }

	void Shader::SetName(const std::string& name) { m_Name = name; }

	void Shader::SetBool(const GLchar* name, bool value) const { glUniform1i(glGetUniformLocation(m_Program, name), (int)value); }
	void Shader::SetInt(const GLchar* name, int value) const { glUniform1i(glGetUniformLocation(m_Program, name), value); }
	void Shader::SetFloat(const GLchar* name, float value) const { glUniform1f(glGetUniformLocation(m_Program, name), value); }
	void Shader::SetiVec2(const GLchar* name, const glm::ivec2& value) const { glUniform2iv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetiVec3(const GLchar* name, const glm::ivec3& value) const { glUniform3iv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetiVec4(const GLchar* name, const glm::ivec4& value) const { glUniform4iv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
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
				L_CORE_ERROR("Shader Compilation Error: {0}\n{1}", type, infoLog);
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				L_CORE_ERROR("Shader Linking Error: {0}\n{1}", type, infoLog);
			}
		}
	}

// -- Shader Library --

	void ShaderLibrary::UnBindAllShaders() { glUseProgram(0); }

	/// <summary>
	/// Initialises the ShaderLibrary class so that it always holds a default shader.
	/// </summary>
	ShaderLibrary::ShaderLibrary() {
		m_DefaultShader = LoadShader("assets/Shaders/Default Shader.glsl");
	}

	/// <summary>
	/// This loads a shader based on the name of the shader file path.
	/// </summary>
	/// <param name="shaderFile">File Path to Shader</param>
	/// <param name="isComputeShader">Set to true if it is a Compute Shader</param>
	/// <returns></returns>
	std::shared_ptr<Shader>& ShaderLibrary::LoadShader(const std::string& shaderFile, bool isComputeShader) {

		std::string shaderName = FilePathToShaderName(shaderFile);

		// Check if Shader Already Exists
		if (ShaderExists(shaderName)) {
			L_CORE_INFO("Shader Already Loaded: {0}", shaderName);
			return m_Shaders[shaderName];
		}

		std::shared_ptr<Shader> shader = std::make_unique<Shader>(shaderFile.c_str(), isComputeShader);
		shader->SetName(shaderName);

		// Check if Shader Linked Successfully
		GLint success;
		glGetProgramiv(shader->GetProgram(), GL_LINK_STATUS, &success);

		if (success == GL_FALSE) {
			L_CORE_WARN("Shader Not Loaded - Returning Default Shader");
			return m_DefaultShader;
		}

		L_CORE_INFO("Shader Loaded: {0}", shaderName);
		m_Shaders[shaderName] = std::move(shader);
		return m_Shaders[shaderName];
	}

	/// <summary>
	/// This returns a unique_ptr reference to a given shader. Where there is no shader that exists, it returns a default shader.
	/// </summary>
	/// <param name="shaderName"></param>
	/// <returns></returns>
	std::shared_ptr<Shader>& ShaderLibrary::GetShader(const std::string& shaderName) {
		if (ShaderExists(shaderName))
			return m_Shaders[shaderName];

		L_CORE_WARN("Shader Not Loaded - Returning Default Shader");
		return m_DefaultShader;
	}

	/// <summary>
	/// Checks if the shader exists within the map.
	/// </summary>
	/// <param name="name"></param>
	/// <returns></returns>
	bool ShaderLibrary::ShaderExists(const std::string& name) const {
		return m_Shaders.find(name) != m_Shaders.end();
	}

	/// <summary>
	/// Decodes the filepath to determine the name of the shader.
	/// </summary>
	/// <param name="shaderFile"></param>
	/// <returns></returns>
	std::string ShaderLibrary::FilePathToShaderName(const std::string& shaderFile)
	{
		std::string name = shaderFile;
		auto lastSlash = name.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = name.rfind('.');
		auto count = lastDot == std::string::npos ? name.size() - lastSlash : lastDot - lastSlash;
		name = name.substr(lastSlash, count);
		return name.c_str();
	}
}