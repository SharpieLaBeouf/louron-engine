#pragma once

#include "Buffer.h"

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

		std::cout << "[L20] OpenGL Data Type Not Defined!" << std::endl;
		return 0;
	}

	class VertexArray {

	public:

		VertexArray();
		~VertexArray();

		void Bind() const;
		void Unbind() const;

		void AddVertexBuffer(VertexBuffer* vertexBuffer);
		void SetIndexBuffer(IndexBuffer* indexBuffer);

		const std::vector<VertexBuffer*> GetVertexBuffers() const { return m_VertexBuffers; }
		const IndexBuffer* GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		GLuint m_VAO = NULL;
		GLuint m_VertexBufferIndex = 0;

		IndexBuffer* m_IndexBuffer = nullptr;
		std::vector<VertexBuffer*> m_VertexBuffers;
	};
}