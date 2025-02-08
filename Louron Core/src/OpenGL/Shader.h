#pragma once

// Louron Core Headers
#include "../Asset/Asset.h"

// C++ Standard Library Headers
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <filesystem>

// External Vendor Library Headers
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Louron {

	enum class GLSLType {
		Unknown,

		// Bool
		Bool, BVec2, BVec3, BVec4,

		// Int
		Int, IVec2, IVec3, IVec4,

		// Unsigned Int
		Uint, UVec2, UVec3, UVec4,

		// Float
		Float, Vec2, Vec3, Vec4,

		// Double
		Double, DVec2, DVec3, DVec4,

		// Matrix
		Mat2, Mat3, Mat4,

		// Textures
		Sampler1D, Sampler1DArray, Sampler1DShadow, Sampler1DArrayShadow, 
		Sampler2D, Sampler2DArray, Sampler2DShadow, Sampler2DArrayShadow,
		Sampler3D,
		SamplerCube, SamplerCubeArray, SamplerCubeShadow, SamplerCubeArrayShadow
	};

	namespace Utils
	{

		static std::string GLSLTypeToString(GLSLType type) {
			static const std::unordered_map<GLSLType, std::string> enumToStringMap = {
				{GLSLType::Bool, "bool"}, {GLSLType::BVec2, "bvec2"}, {GLSLType::BVec3, "bvec3"}, {GLSLType::BVec4, "bvec4"},
				{GLSLType::Int, "int"}, {GLSLType::IVec2, "ivec2"}, {GLSLType::IVec3, "ivec3"}, {GLSLType::IVec4, "ivec4"},
				{GLSLType::Uint, "uint"}, {GLSLType::UVec2, "uvec2"}, {GLSLType::UVec3, "uvec3"}, {GLSLType::UVec4, "uvec4"},
				{GLSLType::Float, "float"}, {GLSLType::Vec2, "vec2"}, {GLSLType::Vec3, "vec3"}, {GLSLType::Vec4, "vec4"},
				{GLSLType::Double, "double"}, {GLSLType::DVec2, "dvec2"}, {GLSLType::DVec3, "dvec3"}, {GLSLType::DVec4, "dvec4"},
				{GLSLType::Mat2, "mat2"}, {GLSLType::Mat3, "mat3"}, {GLSLType::Mat4, "mat4"},
				{GLSLType::Sampler1D, "sampler1D"}, {GLSLType::Sampler1DArray, "sampler1DArray"}, {GLSLType::Sampler1DShadow, "sampler1DShadow"}, {GLSLType::Sampler1DArrayShadow, "sampler1DArrayShadow"},
				{GLSLType::Sampler2D, "sampler2D"}, {GLSLType::Sampler2DArray, "sampler2DArray"}, {GLSLType::Sampler2DShadow, "sampler2DShadow"}, {GLSLType::Sampler2DArrayShadow, "sampler2DArrayShadow"},
				{GLSLType::Sampler3D, "sampler3D"},
				{GLSLType::SamplerCube, "samplerCube"}, {GLSLType::SamplerCubeArray, "samplerCubeArray"}, {GLSLType::SamplerCubeShadow, "samplerCubeShadow"}, {GLSLType::SamplerCubeArrayShadow, "samplerCubeArrayShadow"}
			};

			auto it = enumToStringMap.find(type);
			return it != enumToStringMap.end() ? it->second : "Unknown";
		}

		static GLSLType StringToGLSLType(const std::string& typeStr) {
			static const std::unordered_map<std::string, GLSLType> stringToEnumMap = {
				{"bool", GLSLType::Bool}, {"bvec2", GLSLType::BVec2}, {"bvec3", GLSLType::BVec3}, {"bvec4", GLSLType::BVec4},
				{"int", GLSLType::Int}, {"ivec2", GLSLType::IVec2}, {"ivec3", GLSLType::IVec3}, {"ivec4", GLSLType::IVec4},
				{"uint", GLSLType::Uint}, {"uvec2", GLSLType::UVec2}, {"uvec3", GLSLType::UVec3}, {"uvec4", GLSLType::UVec4},
				{"float", GLSLType::Float}, {"vec2", GLSLType::Vec2}, {"vec3", GLSLType::Vec3}, {"vec4", GLSLType::Vec4},
				{"double", GLSLType::Double}, {"dvec2", GLSLType::DVec2}, {"dvec3", GLSLType::DVec3}, {"dvec4", GLSLType::DVec4},
				{"mat2", GLSLType::Mat2}, {"mat3", GLSLType::Mat3}, {"mat4", GLSLType::Mat4},
				{"sampler1D", GLSLType::Sampler1D}, {"sampler1DArray", GLSLType::Sampler1DArray}, {"sampler1DShadow", GLSLType::Sampler1DShadow}, {"sampler1DArrayShadow", GLSLType::Sampler1DArrayShadow},
				{"sampler2D", GLSLType::Sampler2D}, {"sampler2DArray", GLSLType::Sampler2DArray}, {"sampler2DShadow", GLSLType::Sampler2DShadow}, {"sampler2DArrayShadow", GLSLType::Sampler2DArrayShadow},
				{"sampler3D", GLSLType::Sampler3D},
				{"samplerCube", GLSLType::SamplerCube}, {"samplerCubeArray", GLSLType::SamplerCubeArray}, {"samplerCubeShadow", GLSLType::SamplerCubeShadow}, {"samplerCubeArrayShadow", GLSLType::SamplerCubeArrayShadow}
			};

			auto it = stringToEnumMap.find(typeStr);
			return it != stringToEnumMap.end() ? it->second : GLSLType::Unknown;
		}

	}

	const static std::unordered_map<std::string, GLSLType> typeMap = {

		{"bool", GLSLType::Bool}, {"bvec2", GLSLType::BVec2}, {"bvec3", GLSLType::BVec3}, {"bvec4", GLSLType::BVec4},

		{"int", GLSLType::Int}, {"ivec2", GLSLType::IVec2}, {"ivec3", GLSLType::IVec3}, {"ivec4", GLSLType::IVec4},

		{"uint", GLSLType::Uint}, {"uvec2", GLSLType::UVec2}, {"uvec3", GLSLType::UVec3}, {"uvec4", GLSLType::UVec4},

		{"float", GLSLType::Float}, {"vec2", GLSLType::Vec2}, {"vec3", GLSLType::Vec3}, {"vec4", GLSLType::Vec4},

		{"double", GLSLType::Double}, {"dvec2", GLSLType::DVec2}, {"dvec3", GLSLType::DVec3}, {"dvec4", GLSLType::DVec4},

		{"mat2", GLSLType::Mat2}, {"mat3", GLSLType::Mat3}, {"mat4", GLSLType::Mat4},

		{"sampler1D", GLSLType::Sampler1D}, {"sampler1DArray", GLSLType::Sampler1DArray}, {"sampler1DShadow", GLSLType::Sampler1DShadow}, {"sampler1DArrayShadow", GLSLType::Sampler1DArrayShadow},
		{"sampler2D", GLSLType::Sampler2D}, {"sampler2DArray", GLSLType::Sampler2DArray}, {"sampler2DShadow", GLSLType::Sampler2DShadow}, {"sampler2DArrayShadow", GLSLType::Sampler2DArrayShadow},
		{"sampler3D", GLSLType::Sampler3D},
		{"samplerCube", GLSLType::SamplerCube}, {"samplerCubeArray", GLSLType::SamplerCubeArray}, {"samplerCubeShadow", GLSLType::SamplerCubeShadow}, {"samplerCubeArrayShadow", GLSLType::SamplerCubeArrayShadow}
	};

	struct Uniform 
	{
		std::string name;
		GLSLType type;
	};

	class Shader : public Asset {

	private:

		GLuint m_Program = -1;
		std::string m_Name;

		bool m_IsComputeShader = false;
		std::filesystem::path m_ShaderFilePath;

		std::vector<Uniform> m_CustomUniforms;

	public:

		bool IsValid() const { return m_Program != -1; }
		operator bool() const { return IsValid(); }

		virtual AssetType GetType() const override { return AssetType::Shader; }

		void Bind();
		void UnBind();

		Shader(const GLchar* shaderFile, bool isComputeShader);
		~Shader();

		GLuint GetProgram();
		const std::string& GetName();

		const std::vector<Uniform>& GetCustomUniforms() const { return m_CustomUniforms; }

		void SetName(const std::string& name);

		void SetBool(const GLchar* name, bool value) const;
		void SetBoolVec2(const GLchar* name, const glm::bvec2& value) const;
		void SetBoolVec3(const GLchar* name, const glm::bvec3& value) const;
		void SetBoolVec4(const GLchar* name, const glm::bvec4& value) const;

		void SetInt(const GLchar* name, int value) const;
		void SetIntVec2(const GLchar* name, const glm::ivec2& value) const;
		void SetIntVec3(const GLchar* name, const glm::ivec3& value) const;
		void SetIntVec4(const GLchar* name, const glm::ivec4& value) const;

		void SetUInt(const GLchar* name, GLuint value) const;
		void SetUIntVec2(const GLchar* name, const glm::uvec2& value) const;
		void SetUIntVec3(const GLchar* name, const glm::uvec3& value) const;
		void SetUIntVec4(const GLchar* name, const glm::uvec4& value) const;

		void SetFloat(const GLchar* name, float value) const;
		void SetFloatVec2(const GLchar* name, const glm::vec2& value) const;
		void SetFloatVec3(const GLchar* name, const glm::vec3& value) const;
		void SetFloatVec4(const GLchar* name, const glm::vec4& value) const;

		void SetDouble(const GLchar* name, double value) const;
		void SetDoubleVec2(const GLchar* name, const glm::dvec2& value) const;
		void SetDoubleVec3(const GLchar* name, const glm::dvec3& value) const;
		void SetDoubleVec4(const GLchar* name, const glm::dvec4& value) const;

		void SetMat2(const GLchar* name, const glm::mat2& mat) const;
		void SetMat3(const GLchar* name, const glm::mat3& mat) const;
		void SetMat4(const GLchar* name, const glm::mat4& mat) const;

	private:

		bool checkCompileErrors(unsigned int shader, std::string type);

		void LoadShader();
		void ExtractCustomUniforms(const std::string& source);
	};
}