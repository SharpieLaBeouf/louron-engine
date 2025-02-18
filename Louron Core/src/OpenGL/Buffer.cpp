#include "Buffer.h"

namespace Louron {

	// VERTEX BUFFER 

	VertexBuffer::VertexBuffer() {
		glCreateBuffers(1, &m_VBO);
	}

	VertexBuffer::VertexBuffer(GLuint count) {
		m_BufferSize = count * sizeof(GLfloat);
		glCreateBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_DRAW);
	}

	VertexBuffer::VertexBuffer(const float* vertices, GLuint count) {
		m_BufferSize = count * sizeof(GLfloat);
		glCreateBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, vertices, GL_STATIC_DRAW);
	}

	VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices, GLuint size) {
		m_BufferSize = vertices.size() * sizeof(Vertex);
		glCreateBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, &vertices[0], GL_STATIC_DRAW);
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

	void VertexBuffer::SetData(const void* data, GLuint size)
	{
		m_Modified = true;

		m_BufferSize = size;
		if (m_BufferData.size() < m_BufferSize)
			m_BufferData.resize(m_BufferSize);

		memcpy(m_BufferData.data(), data, m_BufferSize);
	}

	const void* VertexBuffer::GetData()
	{
		if (m_BufferSize == 0)
			return nullptr;

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

		if (m_BufferData.size() < m_BufferSize)
			m_BufferData.resize(m_BufferSize);

		glGetBufferSubData(GL_ARRAY_BUFFER, 0, m_BufferSize, (void*)m_BufferData.data());

		return m_BufferData.data();
	}

	void VertexBuffer::ClearData()
	{
		m_Modified = false;
		m_BufferData = {};
	}

	void VertexBuffer::ResizeData(size_t data_size)
	{
		m_Modified = true;

		size_t old_size = m_BufferData.size();
		m_BufferData.resize(data_size, 0);

		m_BufferSize = data_size;
	}

	void VertexBuffer::SubmitData()
	{
		if (!m_Modified)
			return;

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, m_BufferData.data(), GL_DYNAMIC_DRAW);
	}

	// INDEX BUFFER

	IndexBuffer::IndexBuffer() {
		glCreateBuffers(1, &m_IBO);
	}

	IndexBuffer::IndexBuffer(GLuint count) : m_Count(count) {
		glCreateBuffers(1, &m_IBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLuint), nullptr, GL_STATIC_DRAW);
	}

	IndexBuffer::IndexBuffer(const GLuint* indices, GLuint count) : m_Count(count) {
		glCreateBuffers(1, &m_IBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
	}

	IndexBuffer::IndexBuffer(const std::vector<GLuint>& indices, GLuint count) : m_Count(count) {
		glCreateBuffers(1, &m_IBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
	}

	IndexBuffer::~IndexBuffer() 
	{
		glDeleteBuffers(1, &m_IBO);
	}

	void IndexBuffer::Bind() const 
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	}

	void IndexBuffer::Unbind() const 
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void IndexBuffer::SetData(const void* data, GLuint size)
	{
		m_Modified = true;

		m_Count = size / sizeof(GLuint);
		if (m_BufferData.size() < size)
			m_BufferData.resize(size);

		memcpy(m_BufferData.data(), data, size);
	}

	const void* IndexBuffer::GetData()
	{
		if (m_Count == 0)
			return nullptr;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);

		if (m_BufferData.size() < m_Count * sizeof(GLuint))
			m_BufferData.resize(m_Count * sizeof(GLuint));

		glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Count * sizeof(GLuint), (void*)m_BufferData.data());

		return m_BufferData.data();
	}

	void IndexBuffer::ClearData()
	{
		m_Modified = false;
		m_BufferData = {};
	}

	void IndexBuffer::SubmitData()
	{
		if (!m_Modified)
			return;

		glBindBuffer(GL_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ARRAY_BUFFER, m_Count * sizeof(GLuint), m_BufferData.data(), GL_DYNAMIC_DRAW);
	}
}