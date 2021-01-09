#include "../Headers/Abstracted GL/Vertex Array.h"


VertexArray::VertexArray() {
	glCreateVertexArrays(1, &m_VAO);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &m_VAO);
}

void VertexArray::Bind() const {
	glBindVertexArray(m_VAO);
}

void VertexArray::Unbind() const {
	glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer) {
	glBindVertexArray(m_VAO);
	vertexBuffer->Bind();

	const auto& layout = vertexBuffer->GetLayout();
	for (const auto& element : layout) {
		switch (element.Type) {
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::Bool:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex, element.GetComponentCount(), OpenGLDataType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)element.Offset);
				m_VertexBufferIndex++;
				break;
			}
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			{
				GLint count = element.GetComponentCount();
				for (int i = 0; i < count; i++) {
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex, count, OpenGLDataType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)(element.Offset* sizeof(float) * count * i));
					glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default: std::cout << "[L20] Shader Data Type Not Defined!" << std::endl;
		}
	}

	m_VertexBuffers.push_back(vertexBuffer);
}

void VertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {

	glBindVertexArray(m_VAO);
	indexBuffer->Bind();

	m_IndexBuffer = indexBuffer;
}
