#include "Framebuffer.h"

#include "../Debug/Assert.h"

namespace Louron {

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_FBO);
		glDeleteTextures(1, &m_ColourTexture);
		glDeleteTextures(1, &m_DepthTexture);
		glDeleteBuffers(1, &m_ENTITY_SSBO);
		glDeleteFramebuffers(1, &m_MS_FBO);
		glDeleteTextures(1, &m_MS_ColourTexture);
		glDeleteTextures(1, &m_MS_DepthTexture);
	}

	void FrameBuffer::ResetFrameBuffer() {

		if (m_FBO != -1)
		{
			glDeleteFramebuffers(1, &m_FBO);
			glDeleteTextures(1, &m_ColourTexture);
			glDeleteTextures(1, &m_DepthTexture);
			m_ColourTexture = -1;
			m_DepthTexture = -1;

			glDeleteBuffers(1, &m_ENTITY_SSBO);

			if (m_MultiSampled)
			{
				glDeleteFramebuffers(1, &m_MS_FBO);
				glDeleteTextures(1, &m_MS_ColourTexture);
				glDeleteTextures(1, &m_MS_DepthTexture);
				m_MS_ColourTexture = -1;
				m_MS_DepthTexture = -1;
			}
		}

		m_MultiSampled = m_Config.Samples > 1;

		// Setup Main FBO Texture Attachments
		
		// Colour Attachment
		glGenTextures(1, &m_ColourTexture);
		glBindTexture(GL_TEXTURE_2D, m_ColourTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Config.Width, m_Config.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Depth Attachment
		glGenTextures(1, &m_DepthTexture);
		glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Config.Width, m_Config.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Mouse Picking Entities
		glGenBuffers(1, &m_ENTITY_SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ENTITY_SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, (sizeof(uint32_t) + sizeof(float)) * m_Config.Height * m_Config.Width, nullptr, GL_DYNAMIC_DRAW);
		
		// Setup MS Texture Attachments
		if (m_MultiSampled)
		{
			glGenTextures(1, &m_MS_ColourTexture);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MS_ColourTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Config.Samples, GL_RGBA8, m_Config.Width, m_Config.Height, GL_FALSE);

			glGenTextures(1, &m_MS_DepthTexture);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MS_DepthTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Config.Samples, GL_DEPTH24_STENCIL8, m_Config.Width, m_Config.Height, GL_FALSE);
		}

		GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

		// Normal FBO
		glCreateFramebuffers(1, &m_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColourTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
		glDrawBuffers(2, drawBuffers);

		// Check FBO completeness
		L_CORE_ASSERT((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE), "Framebuffer Creation Failed!");

		// MS FBO
		if (m_MultiSampled)
		{
			glCreateFramebuffers(1, &m_MS_FBO);
			glBindFramebuffer(GL_FRAMEBUFFER, m_MS_FBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_MS_ColourTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_MS_DepthTexture, 0);
			glDrawBuffers(2, drawBuffers);

			// Check FBO completeness
			L_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer Creation Failed!");
		}

		// Entity Pick FBO


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::Bind() const {

		if (m_MultiSampled && IsValid())
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_MS_FBO);
			glViewport(0, 0, m_Config.Width, m_Config.Height);
			return;
		}

		if (m_FBO != -1) {
			glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
			glViewport(0, 0, m_Config.Width, m_Config.Height);
		}
	}

	void FrameBuffer::Unbind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::Resize(const glm::ivec2& size) {

		// Check if Size is invalid
		if (size.x <= 0 || size.y <= 0 || size.x > 8192 || size.y > 8192)
		{
			L_CORE_WARN("Attempted to rezize framebuffer to {0}, {1}", size.x, size.y);
			return;
		}

		m_Config.Width = size.x;
		m_Config.Height = size.y;

		ResetFrameBuffer();
	}

	struct _EntityClearData {
		uint32_t entity_id;
		uint32_t depth;
	};

	uint32_t FrameBuffer::ReadEntityPixelData(const glm::ivec2& pos) const 
	{

		uint32_t index = static_cast<uint32_t>(pos.y) * static_cast<uint32_t>(m_Config.Width) + static_cast<uint32_t>(pos.x);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ENTITY_SSBO);

		_EntityClearData* data = (_EntityClearData*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, index * sizeof(_EntityClearData), sizeof(_EntityClearData), GL_MAP_READ_BIT);
		if (data == nullptr) 
		{
			L_CORE_ERROR("Could Not Get Entity UUID.");
			return -1;
		}

		uint32_t entityID = data->entity_id;

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		return entityID;
	}

	void FrameBuffer::ClearEntityPixelData(uint32_t value) const
	{
		_EntityClearData clearData = { value, 0x3F800000 }; // 1.0f as uint
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ENTITY_SSBO);
		glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, &clearData);
	}

	bool FrameBuffer::IsValid() const {

		if (m_MultiSampled && m_MS_FBO == -1)
			return false;

		if (m_FBO == -1)
			return false;

		GLint old_FBO;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_FBO);

		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		bool result = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

		glBindFramebuffer(GL_FRAMEBUFFER, old_FBO);

		return result;
	}

	GLuint FrameBuffer::GetTexture(const FrameBufferTexture& texture_type) const {

		if (!IsValid())
			return -1;

		switch (texture_type) {

			case FrameBufferTexture::ColourTexture:			return m_ColourTexture;
			case FrameBufferTexture::DepthTexture:			return m_DepthTexture;

		}

		L_CORE_WARN("No Texture Type Provided - Returning Colour Texture.");
		return m_ColourTexture;
	}

	void FrameBuffer::ResolveMultisampledFBO() const
	{
		ResolveMultisampledFBO(m_FBO);
	}

	void FrameBuffer::ResolveMultisampledFBO(GLuint other_fbo) const
	{
		if (!IsValid() || !m_MultiSampled)
			return;

		GLint old_FBO;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_FBO);

		glBindFramebuffer(GL_FRAMEBUFFER, other_fbo);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other_fbo);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MS_FBO);

		glBlitFramebuffer(
			0, 0, m_Config.Width, m_Config.Height,
			0, 0, m_Config.Width, m_Config.Height,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
		);

		// Does Not Work Currently this is because the two colour attachments have different types, one being GL_UNSIGNED_INT (entity), the other GL_UNSIGNED_BYTE (colour)

		glBindFramebuffer(GL_FRAMEBUFFER, old_FBO);
	}

	void FrameBuffer::BindEntitySSBO() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ENTITY_SSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ENTITY_SSBO);
	}

	void FrameBuffer::UnBindEntitySSBO() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

}

