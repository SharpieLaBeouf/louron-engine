#pragma once

// Louron Core Headers
#include "Buffer.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	static GLenum OpenGLDataType(ShaderDataType type) {

		switch (type)
		{
		case ShaderDataType::Float:		return GL_FLOAT;
		case ShaderDataType::Float2:	return GL_FLOAT;
		case ShaderDataType::Float3:	return GL_FLOAT;
		case ShaderDataType::Float4:	return GL_FLOAT;
		case ShaderDataType::Mat3:		return GL_FLOAT;
		case ShaderDataType::Mat4:		return GL_FLOAT;
		case ShaderDataType::Int:		return GL_INT;
		case ShaderDataType::Int2:		return GL_INT;
		case ShaderDataType::Int3:		return GL_INT;
		case ShaderDataType::Int4:		return GL_INT;
		case ShaderDataType::Bool:		return GL_BOOL;
		}

		L_CORE_WARN("OpenGL Data Type Not Defined");
		return 0;
	}

	class VertexArray {

	public:

		VertexArray();
		~VertexArray();

		VertexArray(const VertexArray& other);
		VertexArray& operator=(const VertexArray& other);

		VertexArray(VertexArray&& other) noexcept;
		VertexArray& operator=(VertexArray&& other) noexcept;

		bool operator==(const VertexArray& other) { return m_VAO == other.m_VAO; }

		void Bind() const;
		void UnBind() const;

		void AddVertexBuffer(VertexBuffer* vertexBuffer);
		void SetIndexBuffer(IndexBuffer* indexBuffer);

		const std::vector<VertexBuffer*> GetVertexBuffers() const { return m_VertexBuffers; }
		IndexBuffer* GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		GLuint m_VAO = NULL;
		GLuint m_VertexBufferIndex = 0;

		IndexBuffer* m_IndexBuffer = nullptr;
		std::vector<VertexBuffer*> m_VertexBuffers;
	};
}