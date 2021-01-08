#pragma once

#include "Buffer.h"

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
	GLuint m_VAO;
	GLuint m_VertexBufferIndex = 0;

	IndexBuffer* m_IndexBuffer;
	std::vector<VertexBuffer*> m_VertexBuffers;
};