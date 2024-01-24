#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <initializer_list>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Louron {

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;

		// TODO: implement tangent and bitangent functionality

		//glm::vec3 tangent;
		//glm::vec3 bitangent;
	};

	enum class ShaderDataType { None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool };
	static GLuint ShaderDataTypeSize(ShaderDataType type);

	struct BufferElement {

		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		size_t Offset;
		bool Normalized;

		BufferElement() = default;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false) : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {	}

		uint32_t GetComponentCount() const {
			switch (Type)
			{
			case ShaderDataType::Float:   return 1;
			case ShaderDataType::Float2:  return 2;
			case ShaderDataType::Float3:  return 3;
			case ShaderDataType::Float4:  return 4;
			case ShaderDataType::Mat3:    return 3;
			case ShaderDataType::Mat4:    return 4;
			case ShaderDataType::Int:     return 1;
			case ShaderDataType::Int2:    return 2;
			case ShaderDataType::Int3:    return 3;
			case ShaderDataType::Int4:    return 4;
			case ShaderDataType::Bool:    return 1;
			}

			std::cout << "[L20] Shader Data Type Not Defined!" << std::endl;
			return 0;
		}
	};
	class BufferLayout {

	public:
		BufferLayout() { }
		BufferLayout(std::initializer_list<BufferElement> elements) : m_Elements(elements) {
			SetupBufferLayout();
		}

		GLuint GetStride() const { return m_Stride; }
		const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		GLuint m_Stride = 0;
		std::vector<BufferElement> m_Elements;

		void SetupBufferLayout() {
			GLint offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	};

	class VertexBuffer {

	public:
		VertexBuffer(GLuint size);
		VertexBuffer(float* vertices, GLuint size);
		VertexBuffer(const std::vector<Vertex>& vertices, GLuint size);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;

		void SetData(const void* data, GLuint size);

		const BufferLayout& GetLayout() { return m_Layout; }
		void SetLayout(const BufferLayout& layout) { m_Layout = layout; }

	private:
		GLuint m_VBO;
		BufferLayout m_Layout;
	};

	class IndexBuffer {

	public:
		IndexBuffer(GLuint* indices, GLuint count);
		IndexBuffer(const std::vector<GLuint>& indices, GLuint count);
		~IndexBuffer();

		void Bind() const;
		void Unbind() const;

		GLuint GetCount() const { return m_Count; }
	private:
		GLuint m_IBO;
		GLuint m_Count;
	};

	static GLuint ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Float2:   return 4 * 2;
		case ShaderDataType::Float3:   return 4 * 3;
		case ShaderDataType::Float4:   return 4 * 4;
		case ShaderDataType::Mat3:     return 4 * 3 * 3;
		case ShaderDataType::Mat4:     return 4 * 4 * 4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Int2:     return 4 * 2;
		case ShaderDataType::Int3:     return 4 * 3;
		case ShaderDataType::Int4:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		}

		std::cout << "[L20] Shader Data Type Not Defined!" << std::endl;
		return 0;
	}
}