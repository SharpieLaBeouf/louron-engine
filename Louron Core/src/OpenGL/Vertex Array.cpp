#include "Vertex Array.h"

namespace Louron {

	VertexArray::VertexArray() {
		glCreateVertexArrays(1, &m_VAO);
	}

	VertexArray::~VertexArray() {

		glDeleteVertexArrays(1, &m_VAO);

		delete m_IndexBuffer;

		for (auto vertex : m_VertexBuffers)
			delete vertex;
	}

	VertexArray::VertexArray(const VertexArray& other)
	{
		glCreateVertexArrays(1, &m_VAO);

		const std::vector<VertexBuffer*>& old_vertex_buffer_vec = other.GetVertexBuffers();

		for (const auto& buffer : old_vertex_buffer_vec)
		{
			size_t size = buffer->GetSize();
			const float* data = static_cast<const float*>(buffer->GetData());
			size /= sizeof(float); // Convert from size in bytes to count of float's

			VertexBuffer* new_buffer = new VertexBuffer(data, static_cast<GLuint>(size));
			new_buffer->SetLayout(buffer->GetLayout());

			AddVertexBuffer(new_buffer);

			buffer->ClearData();
		}

		if (other.m_IndexBuffer)
		{
			const GLuint* data = static_cast<const GLuint*>(other.m_IndexBuffer->GetData());
			
			IndexBuffer* ebo = new IndexBuffer(data, other.m_IndexBuffer->GetCount());
			SetIndexBuffer(ebo);
		}

	}

	VertexArray& VertexArray::operator=(const VertexArray& other)
	{
		if (this == &other)
			return *this;

		glCreateVertexArrays(1, &m_VAO);

		const std::vector<VertexBuffer*>& old_vertex_buffer_vec = other.GetVertexBuffers();

		for (const auto& buffer : old_vertex_buffer_vec)
		{
			size_t size = buffer->GetSize();
			const float* data = static_cast<const float*>(buffer->GetData());
			size /= sizeof(float); // Convert from size in bytes to count of float's

			VertexBuffer* new_buffer = new VertexBuffer(data, static_cast<GLuint>(size));
			new_buffer->SetLayout(buffer->GetLayout());

			AddVertexBuffer(new_buffer);

			buffer->ClearData();
		}

		if (other.m_IndexBuffer)
		{
			const GLuint* data = static_cast<const GLuint*>(other.m_IndexBuffer->GetData());

			IndexBuffer* ebo = new IndexBuffer(data, other.m_IndexBuffer->GetCount());
			SetIndexBuffer(ebo);
		}

		return *this;
	}

	VertexArray::VertexArray(VertexArray&& other) noexcept
	{
		m_VAO = other.m_VAO; other.m_VAO = -1;
		m_VertexBufferIndex = other.m_VertexBufferIndex; other.m_VertexBufferIndex = 0;

		m_IndexBuffer = other.m_IndexBuffer; other.m_IndexBuffer = nullptr;
		m_VertexBuffers = other.m_VertexBuffers; other.m_VertexBuffers.clear();
	}

	VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_VAO = other.m_VAO; other.m_VAO = -1;
		m_VertexBufferIndex = other.m_VertexBufferIndex; other.m_VertexBufferIndex = 0;

		m_IndexBuffer = other.m_IndexBuffer; other.m_IndexBuffer = nullptr;
		m_VertexBuffers = other.m_VertexBuffers; other.m_VertexBuffers.clear();

		return *this;
	}

	void VertexArray::Bind() const {
		glBindVertexArray(m_VAO);
	}

	void VertexArray::UnBind() const {
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
					glVertexAttribPointer(m_VertexBufferIndex, count, OpenGLDataType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)(element.Offset * sizeof(float) * count * i));
					glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default: L_CORE_WARN("Shader Data Type Not Defined");
			}
		}

		m_VertexBuffers.push_back(vertexBuffer);
		glBindVertexArray(0);
		vertexBuffer->Unbind();
	}

	void VertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {

		glBindVertexArray(m_VAO);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}