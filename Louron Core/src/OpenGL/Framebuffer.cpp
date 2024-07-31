#include "Framebuffer.h"

#include "../Debug/Assert.h"

namespace Louron {
	void FrameBuffer::ResetFrameBuffer() {

		if (m_FBO != -1)
		{
			glDeleteFramebuffers(1, &m_FBO);
			glDeleteTextures(1, &m_ColourTexture);
			glDeleteTextures(1, &m_EntityUUIDTexture);
			glDeleteTextures(1, &m_DepthTexture);
			m_ColourTexture = -1;
			m_EntityUUIDTexture = -1;
			m_DepthTexture = -1;
		}

		glCreateFramebuffers(1, &m_FBO);

		bool multisample = m_Config.Samples > 1;

		glGenTextures(1, &m_ColourTexture);
		glGenTextures(1, &m_EntityUUIDTexture);
		glGenTextures(1, &m_DepthTexture);

		// Bind the color texture and set its parameters
		glBindTexture(GL_TEXTURE_2D, m_ColourTexture);
		if (multisample) {

			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Config.Samples, GL_RGBA8, m_Config.Width, m_Config.Height, GL_FALSE);
		}
		else {

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Config.Width, m_Config.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		// Create texture for entity IDs
		glBindTexture(GL_TEXTURE_2D, m_EntityUUIDTexture);

		if (multisample) {

			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Config.Samples, GL_R32UI, m_Config.Width, m_Config.Height, GL_FALSE);
		}
		else {

			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, m_Config.Width, m_Config.Height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		// Bind the depth texture and set its parameters
		glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
		if (multisample) {

			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Config.Samples, GL_DEPTH_COMPONENT, m_Config.Width, m_Config.Height, GL_FALSE);
		}
		else {

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Config.Width, m_Config.Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		}


		// Bind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColourTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_EntityUUIDTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

		// Set the list of draw buffers
		GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, drawBuffers);

		// Check FBO completeness
		L_CORE_ASSERT((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE), "Framebuffer Creation Invalid!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::Bind() const {
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

	uint32_t FrameBuffer::ReadEntityPixelData(const glm::ivec2& pos) const {

		GLint old_FBO;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_FBO);

		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

		glReadBuffer(GL_COLOR_ATTACHMENT1);
		uint32_t pixelData = UINT32_MAX;
		glReadPixels(pos.x, pos.y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pixelData);

		glBindFramebuffer(GL_FRAMEBUFFER, old_FBO);

		return pixelData;
	}

	void FrameBuffer::ClearEntityPixelData(uint32_t value) const {

		// Clear the Entity ID Buffer
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glClearTexImage(m_EntityUUIDTexture, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &value);
	}

	bool FrameBuffer::IsValid() const {

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

		switch (texture_type) {

		case FrameBufferTexture::ColourTexture:			return m_ColourTexture;
		case FrameBufferTexture::EntityUUIDTexture:		return m_EntityUUIDTexture;
		case FrameBufferTexture::DepthTexture:			return m_DepthTexture;

		}

		L_CORE_WARN("No Texture Type Provided - Returning Colour Texture.");
		return m_ColourTexture;
	}
}

