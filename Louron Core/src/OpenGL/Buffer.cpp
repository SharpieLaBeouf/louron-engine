#include "Buffer.h"

namespace Louron {

	// VERTEX BUFFER 

	VertexBuffer::VertexBuffer(GLuint size) {
		glCreateBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	VertexBuffer::VertexBuffer(float* vertices, GLuint count) {
		glCreateBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	}

	VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices, GLuint size) {
		glCreateBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	}

	VertexBuffer::~VertexBuffer() {
		glDeleteBuffers(1, &m_VBO);
	}

	void VertexBuffer::Bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	}

	void VertexBuffer::Unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBuffer::SetData(const void* data, GLuint size) {
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	// INDEX BUFFER

	IndexBuffer::IndexBuffer(GLuint* indices, GLuint count) : m_Count(count) {
		glCreateBuffers(1, &m_IBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
	}

	IndexBuffer::IndexBuffer(const std::vector<GLuint>& indices, GLuint count) : m_Count(count) {
		glCreateBuffers(1, &m_IBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	}

	IndexBuffer::~IndexBuffer() {
		glDeleteBuffers(1, &m_IBO);
	}

	void IndexBuffer::Bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	}

	void IndexBuffer::Unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}