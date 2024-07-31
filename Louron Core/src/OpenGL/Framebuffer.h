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

		// Set this to true if you want the Engine to render 
		// this to the OGL swapchain as a fullscreen QUAD
		bool RenderToScreen = false;

	};

	enum class FrameBufferTexture
	{
		None = 0,

		ColourTexture,
		EntityUUIDTexture,
		DepthTexture
	};

	class FrameBuffer {

	public:

		FrameBuffer() {
			ResetFrameBuffer();
		}
		FrameBuffer(const FrameBufferConfig& config) : m_Config(config) {
			ResetFrameBuffer();
		}

		void ResetFrameBuffer();

		void Bind() const;

		void Unbind() const;

		/// <summary>
		/// This will resize the framebuffer.
		/// </summary>
		void Resize(const glm::ivec2& size);

		/// <summary>
		/// This will read the data of the pixel for the appropriate COLOUR attachment.
		/// 
		/// This does not read the depth attachment.
		/// </summary>
		uint32_t ReadEntityPixelData(const glm::ivec2& pos) const;

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		void ClearEntityPixelData(uint32_t value) const;

		const FrameBufferConfig& GetConfig() const { return m_Config; }

		bool IsValid() const;

		GLuint GetTexture(const FrameBufferTexture& texture_type) const;

	private:

		GLuint m_FBO = -1;
		GLuint m_ColourTexture = -1;		// Texture GL_RGBA holds out rendered texture.
		GLuint m_EntityUUIDTexture = -1;	// Texture GL_RED_INTEGER holds uint32_t of the entity UUID being drawn.
		GLuint m_DepthTexture = -1;			// Texture GL_DEPTH_COMPONENT

		FrameBufferConfig m_Config;			// Hold information on the config of the FBO
	};

}