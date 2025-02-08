#pragma once

#include "../Core/Logging.h"

#include <vector>
#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Louron {

	struct FrameBufferConfig {

		uint32_t Width = 800;
		uint32_t Height = 600;
		uint8_t Samples = 1;

		// Set true if you just want a full screen 
		// quad with the main colour texture.
		bool RenderToScreen = false;

	};

	enum class FrameBufferTexture
	{
		None = 0,

		ColourTexture,
		DepthTexture
	};

	class FrameBuffer {

	public:

		FrameBuffer() { ResetFrameBuffer(); }
		FrameBuffer(const FrameBufferConfig& config) : m_Config(config) { ResetFrameBuffer(); }
		~FrameBuffer();

		void ResetFrameBuffer();

		void Bind() const;
		void Unbind() const;

		/// <summary>
		/// This will resize the framebuffer.
		/// </summary>
		void Resize(const glm::ivec2& size);

		/// <summary>
		/// This will read the data of the pixel for the appropriate COLOUR attachment.
		/// </summary>
		uint32_t ReadEntityPixelData(const glm::ivec2& pos) const;

		/// <summary>
		/// Clears the entity pixel data of the entity texture attachment with the passed value.
		/// </summary>
		void ClearEntityPixelData(uint32_t value) const;

		/// <summary>
		/// Returns the config of the current FrameBuffer.
		/// </summary>
		const FrameBufferConfig& GetConfig() const { return m_Config; }

		/// <summary>
		/// Checks if the FrameBuffer is valid and complete.
		/// </summary>
		bool IsValid() const;

		/// <summary>
		/// Get the texture ID of the required texture type.
		/// </summary>
		GLuint GetTexture(const FrameBufferTexture& texture_type) const;

		/// <summary>
		/// This must be called after all rendering has been completed.
		/// </summary>
		void ResolveMultisampledFBO() const;

		/// <summary>
		/// This must be called after all rendering has been completed.
		/// </summary>
		void ResolveMultisampledFBO(GLuint other_fbo) const;

		void BindEntitySSBO() const;
		void UnBindEntitySSBO() const;

	private:

		GLuint m_ENTITY_SSBO = -1;

		GLuint m_FBO = -1;
		GLuint m_ColourTexture = -1;		// Texture GL_RGBA holds out rendered texture.
		GLuint m_DepthTexture = -1;			// Texture GL_DEPTH_COMPONENT

		GLuint m_MS_FBO = -1;
		GLuint m_MS_ColourTexture = -1;		// Texture GL_RGBA holds out rendered texture.
		GLuint m_MS_DepthTexture = -1;			// Texture GL_DEPTH_COMPONENT

		bool m_MultiSampled = false;

		FrameBufferConfig m_Config;			// Hold information on the config of the FBO
	};

}