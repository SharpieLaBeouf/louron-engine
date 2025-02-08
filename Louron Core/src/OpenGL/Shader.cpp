#include "Shader.h"

// Louron Core Headers
#include "../Core/Logging.h"

// C++ Standard Library Headers
#include <filesystem>
#include <regex>

// External Vendor Library Headers

namespace Louron {

// -- Shader --

	void Shader::Bind() { glUseProgram(m_Program); }
	void Shader::UnBind() { glUseProgram(0); }

	Shader::Shader(const GLchar* shaderFile, bool isComputeShader = false) {
		
		m_ShaderFilePath = shaderFile;
		m_Name = m_ShaderFilePath.stem().string();
		m_IsComputeShader = isComputeShader;

		LoadShader();
	}

	Shader::~Shader() { glDeleteProgram(m_Program); }

	GLuint Shader::GetProgram() { return m_Program; }

	const std::string& Shader::GetName() { return m_Name; }

	void Shader::SetName(const std::string& name) { m_Name = name; }

	void Shader::SetBool(const GLchar* name, bool value) const { glUniform1i(glGetUniformLocation(m_Program, name), (int)value); }
	void Shader::SetBoolVec2(const GLchar* name, const glm::bvec2& value) const { glUniform2i(glGetUniformLocation(m_Program, name), (int)value.x, (int)value.y); }
	void Shader::SetBoolVec3(const GLchar* name, const glm::bvec3& value) const { glUniform3i(glGetUniformLocation(m_Program, name), (int)value.x, (int)value.y, (int)value.z); }
	void Shader::SetBoolVec4(const GLchar* name, const glm::bvec4& value) const { glUniform4i(glGetUniformLocation(m_Program, name), (int)value.x, (int)value.y, (int)value.z, (int)value.w); }

	void Shader::SetInt(const GLchar* name, int value) const { glUniform1i(glGetUniformLocation(m_Program, name), value); }
	void Shader::SetIntVec2(const GLchar* name, const glm::ivec2& value) const { glUniform2iv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetIntVec3(const GLchar* name, const glm::ivec3& value) const { glUniform3iv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetIntVec4(const GLchar* name, const glm::ivec4& value) const { glUniform4iv(glGetUniformLocation(m_Program, name), 1, &value[0]); }

	void Shader::SetUInt(const GLchar* name, GLuint value) const { glUniform1ui(glGetUniformLocation(m_Program, name), value); }
	void Shader::SetUIntVec2(const GLchar* name, const glm::uvec2& value) const { glUniform2uiv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetUIntVec3(const GLchar* name, const glm::uvec3& value) const { glUniform3uiv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetUIntVec4(const GLchar* name, const glm::uvec4& value) const { glUniform4uiv(glGetUniformLocation(m_Program, name), 1, &value[0]); }

	void Shader::SetFloat(const GLchar* name, float value) const { glUniform1f(glGetUniformLocation(m_Program, name), value); }
	void Shader::SetFloatVec2(const GLchar* name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetFloatVec3(const GLchar* name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetFloatVec4(const GLchar* name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(m_Program, name), 1, &value[0]); }

	void Shader::SetDouble(const GLchar* name, double value) const { glUniform1d(glGetUniformLocation(m_Program, name), value); }
	void Shader::SetDoubleVec2(const GLchar* name, const glm::dvec2& value) const { glUniform2dv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetDoubleVec3(const GLchar* name, const glm::dvec3& value) const { glUniform3dv(glGetUniformLocation(m_Program, name), 1, &value[0]); }
	void Shader::SetDoubleVec4(const GLchar* name, const glm::dvec4& value) const { glUniform4dv(glGetUniformLocation(m_Program, name), 1, &value[0]); }

	void Shader::SetMat2(const GLchar* name, const glm::mat2& mat) const { glUniformMatrix2fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]); }
	void Shader::SetMat3(const GLchar* name, const glm::mat3& mat) const { glUniformMatrix3fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]); }
	void Shader::SetMat4(const GLchar* name, const glm::mat4& mat) const { glUniformMatrix4fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, &mat[0][0]); }

	bool Shader::checkCompileErrors(unsigned int shader, std::string type) {
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				L_CORE_ERROR("Shader Compilation Error: {0}\n{1}", type, infoLog);
				return false;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				L_CORE_ERROR("Shader Linking Error: {0}\n{1}", type, infoLog);
				return false;
			}
		}
		return true;
	}

	void Shader::LoadShader() {

		std::ifstream file(m_ShaderFilePath);

		if (!m_IsComputeShader) {

			enum class ShaderType {
				NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2, CUSTOM_MATERIAL_UNIFORNS = 3
			};

			std::string line; std::stringstream ss[4];
			ShaderType type = ShaderType::NONE;
			while (getline(file, line))
			{
				if (line.find("#SHADER") != std::string::npos)
				{
					if (line.find("VERTEX") != std::string::npos)
						type = ShaderType::VERTEX;
					else if (line.find("FRAGMENT") != std::string::npos)
						type = ShaderType::FRAGMENT;
					else if (line.find("GEOMETRY") != std::string::npos)
						type = ShaderType::GEOMETRY;
					else if (line.find("CUSTOM_MATERIAL_UNIFORMS") != std::string::npos)
						type = ShaderType::CUSTOM_MATERIAL_UNIFORNS;
				}
				else
					ss[(int)type] << line << '\n';
			}

			std::string vString = ss[0].str();
			std::string fString = ss[1].str();
			std::string gString = ss[2].str();
			std::string customMaterialStructString = ss[3].str();

			if (customMaterialStructString.empty())
				customMaterialStructString = "struct MaterialUniforms { int DO_NOT_USE_PLACE_HOLDER; };\n";

			// Function to insert customMaterialStructString after the #version line
			auto insertAfterVersion = [](std::string& shaderString, const std::string& customStruct) {
				size_t versionPos = shaderString.find("#version");
				if (versionPos != std::string::npos) {
					// Insert after the #version line (skip the #version line itself)
					size_t newlinePos = shaderString.find('\n', versionPos);
					if (newlinePos != std::string::npos) {
						shaderString.insert(newlinePos + 1, customStruct);  // Insert right after the #version line
					}
				}
			};

			// Insert customMaterialStructString into both vertex and fragment shader strings
			insertAfterVersion(vString, customMaterialStructString);
			insertAfterVersion(fString, customMaterialStructString);

			const GLchar* vShaderCode = vString.c_str();
			const GLchar* fShaderCode = fString.c_str();
			const GLchar* gShaderCode = gString.c_str();

			unsigned int vertex, fragment, geometry;

			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			if (!checkCompileErrors(vertex, "VERTEX")) {
				glDeleteShader(vertex);
				m_Program = -1;
				return;
			}

			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);
			if (!checkCompileErrors(fragment, "FRAGMENT")) {
				glDeleteShader(vertex);
				glDeleteShader(fragment);
				m_Program = -1;
				return;
			}

			if(!gString.empty()) {
				geometry = glCreateShader(GL_GEOMETRY_SHADER);
				glShaderSource(geometry, 1, &gShaderCode, NULL);
				glCompileShader(geometry);
				if (!checkCompileErrors(geometry, "GEOMETRY")) {
					glDeleteShader(vertex);
					glDeleteShader(fragment);
					glDeleteShader(geometry);
					m_Program = -1;
					return;
				}
			}

			unsigned int program = glCreateProgram();
			glAttachShader(program, vertex);
			glAttachShader(program, fragment);
			if (!gString.empty()) glAttachShader(program, geometry);
			glLinkProgram(program);
			
			if (!checkCompileErrors(program, "PROGRAM")) {
				glDeleteShader(vertex);
				glDeleteShader(fragment);
				if (!gString.empty()) glDeleteShader(geometry);
				glDeleteProgram(program);
				m_Program = -1;
				return;
			}

			m_Program = program;

			L_CORE_INFO("Shader Compiled Successfully: {}", m_Name);

			ExtractCustomUniforms(customMaterialStructString);

			glDeleteShader(vertex);
			glDeleteShader(fragment);
			if (!gString.empty()) glDeleteShader(geometry);
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
			if (!checkCompileErrors(compute, "COMPUTE"))
			{
				glDeleteShader(compute);
				m_Program = -1;
				return;
			}

			GLuint program = glCreateProgram();
			glAttachShader(program, compute);
			glLinkProgram(program);
			if (!checkCompileErrors(compute, "PROGRAM"))
			{
				glDeleteShader(compute);
				glDeleteProgram(program);
				m_Program = -1;
				return;
			}

			m_Program = program;

			L_CORE_INFO("Shader Compiled Successfully: {}", m_Name);

			glDeleteShader(compute);
		}


	}



	void Shader::ExtractCustomUniforms(const std::string& source)
	{
		std::istringstream stream(source);
		std::string line;

		while (std::getline(stream, line)) {
			// Trim whitespace
			line = std::regex_replace(line, std::regex("^\s+|\s+$"), "");
			if (line.empty() || line.rfind("//", 0) == 0) continue; // Skip comments

			size_t commentPos = line.find("//");
			if (commentPos != std::string::npos) line = line.substr(0, commentPos);

			size_t pos = 0;
			while ((pos = line.find(";")) != std::string::npos) {
				std::string statement = line.substr(0, pos);
				line = line.substr(pos + 1);

				std::istringstream ss(statement);
				std::string typeStr, varName;
				ss >> typeStr;

				if (typeMap.find(typeStr) == typeMap.end()) 
					continue;

				GLSLType type = typeMap.at(typeStr);

				while (ss >> varName) {
					varName = std::regex_replace(varName, std::regex(",[ ]*"), "");
					if (!varName.empty() && varName != "DO_NOT_USE_PLACE_HOLDER")
					{
						m_CustomUniforms.push_back({ varName, type });
						L_CORE_INFO(" - Custom Uniform Found : (Type:{}, Name : {})", typeStr, varName);
					}
				}
			}
		}
	}
}