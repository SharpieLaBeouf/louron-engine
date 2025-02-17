#include "Texture.h"

// Louron Core Headers
#include "../Core/Logging.h"
#include "../Debug/Assert.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include <stb_image/stb_image.h>

#include "glm/gtx/string_cast.hpp"
#include <glad/glad.h>

namespace Louron 
{

	Texture2D::Texture2D()
	{
		constexpr unsigned char data[] = { 255, 255, 255, 255 };
		CreateTexture(data, TextureFormat::RED_GREEN_BLUE_ALPHA_8);

		m_TextureDataSize = 4;
		ConvertAndCopyTextureData(data, 1, 1, TextureFormat::RED_GREEN_BLUE_ALPHA_8, m_TextureData, TextureFormat::RED_GREEN_BLUE_ALPHA_8);
	}

	Texture2D::Texture2D(int width, int height, const TextureFormat& internal_format, bool keep_data)
		: m_TextureFormat(internal_format), m_IsMutable(keep_data)
	{
		L_CORE_ASSERT(width > 0 && height > 0, "Cannot Create Texture With Invalid Size");

		m_TextureSize = { width, height };
		if (m_TextureSize.x < 1) m_TextureSize.x = 1;
		if (m_TextureSize.y < 1) m_TextureSize.y = 1;

		size_t data_size = (size_t)m_TextureSize.x * m_TextureSize.y * GetChannelsFromFormat(m_TextureFormat);
		unsigned char* data = new unsigned char[data_size];
		std::memset(data, 255, data_size);

		CreateTexture(data, m_TextureFormat);

		if (!keep_data)
		{
			delete[] data;
			return;
		}

		m_TextureDataSize = static_cast<size_t>(width * height * GetChannelsFromFormat(internal_format));
		ConvertAndCopyTextureData(data, width, height, internal_format, m_TextureData, internal_format);
	}

	Texture2D::Texture2D(const unsigned char* data, int width, int height, const TextureFormat& internal_format, const TextureFormat& data_format, bool keep_data)
		: m_TextureFormat(internal_format), m_IsMutable(keep_data)
	{
		L_CORE_ASSERT(width > 0 && height > 0, "Cannot Create Texture With Invalid Size");

		m_TextureSize = { width, height };
		if (m_TextureSize.x < 1) m_TextureSize.x = 1;
		if (m_TextureSize.y < 1) m_TextureSize.y = 1;

		CreateTexture(data, data_format);

		if (!keep_data)
			return;

		m_TextureDataSize = static_cast<size_t>(width * height * GetChannelsFromFormat(internal_format));
		ConvertAndCopyTextureData(data, width, height, data_format, m_TextureData, internal_format);
	}

	Texture2D::Texture2D(const std::filesystem::path& texture_path, bool keep_data)
		: m_IsMutable(keep_data)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(texture_path.string().c_str(), &width, &height, &channels, 0);

		L_CORE_ASSERT(width > 0 && height > 0, "Cannot Create Texture With Invalid Size");

		m_TextureSize = { width, height };
		if (m_TextureSize.x < 1) m_TextureSize.x = 1;
		if (m_TextureSize.y < 1) m_TextureSize.y = 1;

		m_TextureFormat = GetFormatFromChannels(channels);

		CreateTexture(data, m_TextureFormat);

		if (!keep_data)
		{
			stbi_image_free(data);
			return;
		}

		m_TextureDataSize = static_cast<size_t>(width * height * GetChannelsFromFormat(m_TextureFormat));
		ConvertAndCopyTextureData(data, width, height, m_TextureFormat, m_TextureData, m_TextureFormat);

		stbi_image_free(data); // Free original STB image data
	}

	Texture2D::~Texture2D()
	{
		if (m_TextureId != -1)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &m_TextureId);
		}

		if(m_TextureData)
			delete[] m_TextureData;
		m_TextureData = nullptr;
	}

	void Texture2D::CreateTexture(const unsigned char* data, const TextureFormat& data_format)
	{
		if (m_TextureId != -1)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &m_TextureId);
			m_TextureId = -1;
		}

		glGenTextures(1, &m_TextureId);
		glBindTexture(GL_TEXTURE_2D, m_TextureId);

		GLenum gl_data_format = GL_RGBA;
		switch (m_TextureFormat)
		{
			case RED_8:
			{
				gl_data_format = GL_RED;

				GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
				glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
				break;
			}
			case RED_GREEN_BLUE_8:
			{
				gl_data_format = GL_RGB;
				break;
			}
			case RED_GREEN_BLUE_ALPHA_8:
			{
				gl_data_format = GL_RGBA;
				break;
			}
			case BLUE_GREEN_RED_ALPHA_8:
			{
				gl_data_format = GL_BGRA;
				break;
			}
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormatType(m_TextureFormat), m_TextureSize.x, m_TextureSize.y, 0, gl_data_format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		GLenum wrap = GetWrapMode();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

		GLenum filter = GetFilterMode();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (filter == GL_LINEAR) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		if (GLAD_GL_EXT_texture_filter_anisotropic == GL_TRUE) 
		{
			GLfloat max_aniso = 0.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
			if (max_aniso > 1.0f)
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);

		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture2D::SetWrapMode(GLenum wrap_mode)
	{
		if (wrap_mode != GL_REPEAT && 
			wrap_mode != GL_MIRRORED_REPEAT && 
			wrap_mode != GL_CLAMP_TO_EDGE && 
			wrap_mode != GL_CLAMP_TO_BORDER) return;

		// Remove all Wrap Flags
		m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties & ~(Wrap_Repeat | Wrap_Mirror | Wrap_ClampEdge | Wrap_ClampBorder));
	
		// Add new Wrap flag
		switch (wrap_mode)
		{
			case GL_REPEAT:
			{
				m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties | Wrap_Repeat);
				break;
			}
			case GL_MIRRORED_REPEAT:
			{
				m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties | Wrap_Mirror);
				break;
			}
			case GL_CLAMP_TO_EDGE:
			{
				m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties | Wrap_ClampEdge);
				break;
			}
			case GL_CLAMP_TO_BORDER:
			{
				m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties | Wrap_ClampBorder);
				break;
			}
		}
	}

	void Texture2D::SetFilterMode(GLenum filter_mode)
	{
		if (filter_mode != GL_LINEAR &&
			filter_mode != GL_NEAREST) return;

		// Remove all Filter Flags
		m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties & ~(Filter_Linear | Filter_Nearest));

		// Add new Filter flag
		switch (filter_mode)
		{
			case GL_LINEAR:
			{
				m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties | Filter_Linear);
				break;
			}
			case GL_NEAREST:
			{
				m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties | Filter_Nearest);
				break;
			}
		}
	}

	void Texture2D::SetAnisotropic(bool enable_anisotropic_filtering)
	{
		m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties & ~(Anisotropy_Enabled));

		if (enable_anisotropic_filtering)
			m_TextureProperties = static_cast<TextureProperty>(m_TextureProperties | Anisotropy_Enabled);
	}

	GLenum Texture2D::GetWrapMode() const
	{
		if ((m_TextureProperties & Wrap_Repeat) != 0)
			return GL_REPEAT;

		if ((m_TextureProperties & Wrap_Mirror) != 0)
			return GL_MIRRORED_REPEAT;

		if ((m_TextureProperties & Wrap_ClampEdge) != 0)
			return GL_CLAMP_TO_EDGE;

		if ((m_TextureProperties & Wrap_ClampBorder) != 0)
			return GL_CLAMP_TO_BORDER;

		return 0;
	}

	GLenum Texture2D::GetFilterMode() const
	{
		if ((m_TextureProperties & Filter_Linear) != 0)
			return GL_LINEAR;

		if ((m_TextureProperties & Filter_Nearest) != 0)
			return GL_NEAREST;

		return 0;
	}

	bool Texture2D::GetAnisotropic() const
	{
		return ((m_TextureProperties & Anisotropy_Enabled) != 0);
	}

	const Texture2D::TextureFormat& Texture2D::GetFormat() const
	{
		return m_TextureFormat;
	}

	void Texture2D::SetPixel(const glm::vec4& pixel_colour, const glm::ivec2& pixel_coord)
	{
		if (!m_IsMutable || !m_TextureData)
			return;

		if (pixel_coord.x >= m_TextureSize.x || pixel_coord.y >= m_TextureSize.y)
		{
			L_CORE_WARN("Cannot Set Texture Pixel Out of Bounds: Pixel Coord ({}), Texture Size ({})", glm::to_string(pixel_coord), glm::to_string(m_TextureSize));
			return;
		}

		int channels = GetChannelsFromFormat(m_TextureFormat);
		size_t pixelIndex = static_cast<size_t>(pixel_coord.y * m_TextureSize.x + pixel_coord.x) * channels;

		// Validate that we are not writing out of the buffer size
		if (pixelIndex + channels > m_TextureDataSize)
		{
			L_CORE_ERROR("Pixel Index Out of Bounds! Computed Index: {}, Texture Data Size: {}", std::to_string(pixelIndex), std::to_string(m_TextureDataSize));
			return;
		}

		unsigned char* pixel = &m_TextureData[pixelIndex];

		switch (m_TextureFormat)
		{
			case RED_8:
			{
				pixel[0] = static_cast<unsigned char>(glm::clamp(pixel_colour[0], 0.0f, 1.0f) * 255.0f);
				break;
			}

			case RED_GREEN_BLUE_8:
			{
				pixel[0] = static_cast<unsigned char>(glm::clamp(pixel_colour[0], 0.0f, 1.0f) * 255.0f);
				pixel[1] = static_cast<unsigned char>(glm::clamp(pixel_colour[1], 0.0f, 1.0f) * 255.0f);
				pixel[2] = static_cast<unsigned char>(glm::clamp(pixel_colour[2], 0.0f, 1.0f) * 255.0f);
				break;
			}

			case RED_GREEN_BLUE_ALPHA_8:
			{
				pixel[0] = static_cast<unsigned char>(glm::clamp(pixel_colour[0], 0.0f, 1.0f) * 255.0f);
				pixel[1] = static_cast<unsigned char>(glm::clamp(pixel_colour[1], 0.0f, 1.0f) * 255.0f);
				pixel[2] = static_cast<unsigned char>(glm::clamp(pixel_colour[2], 0.0f, 1.0f) * 255.0f);
				pixel[3] = static_cast<unsigned char>(glm::clamp(pixel_colour[3], 0.0f, 1.0f) * 255.0f);
				break;
			}

			case BLUE_GREEN_RED_ALPHA_8:
			{
				pixel[0] = static_cast<unsigned char>(glm::clamp(pixel_colour[2], 0.0f, 1.0f) * 255.0f);
				pixel[1] = static_cast<unsigned char>(glm::clamp(pixel_colour[1], 0.0f, 1.0f) * 255.0f);
				pixel[2] = static_cast<unsigned char>(glm::clamp(pixel_colour[0], 0.0f, 1.0f) * 255.0f);
				pixel[3] = static_cast<unsigned char>(glm::clamp(pixel_colour[3], 0.0f, 1.0f) * 255.0f);
				break;
			}
		}
	}

	void Texture2D::SetPixelData(const unsigned char* pixel_data, const size_t& pixel_data_size, const TextureFormat& pixel_data_format)
	{
		if (!m_IsMutable || !m_TextureData)
			return;

		if (!pixel_data)
		{
			L_CORE_WARN("Cannot Set Pixel Data: Pixel Data is NULL.");
			return;
		}

		if (pixel_data_size > m_TextureDataSize)
			L_CORE_WARN("Cannot Set Full Pixel Data: Size of Data Provided {} is out of bounds (Texture size: {}).", 
				std::to_string(pixel_data_size),
				std::to_string(m_TextureDataSize));

		if (pixel_data_format != m_TextureFormat)
		{
			if (pixel_data_size > m_TextureDataSize / GetChannelsFromFormat(m_TextureFormat))
			{
				int channels = GetChannelsFromFormat(m_TextureFormat);
				std::string pixel_in_string = std::to_string(pixel_data_size) + " * " + std::to_string(channels) + std::string("(channels) = " + std::to_string(pixel_data_size * channels));
				std::string texture_data_string = std::to_string(m_TextureDataSize / channels) + " * " + std::to_string(channels) + std::string("(channels) = " + std::to_string(m_TextureDataSize));
				L_CORE_ERROR("Cannot Convert and Copy Pixel Data: Size of Data Provided ({}) is out of bounds (Texture Data: {}).", pixel_in_string, texture_data_string);
				return;
			}

			ConvertAndCopyTextureData(pixel_data, m_TextureSize.x, m_TextureSize.y, pixel_data_format, m_TextureData, m_TextureFormat);
			return;
		}

		// Do not copy out of bounds of m_TextureData
		size_t bytes_to_copy = std::min(pixel_data_size, m_TextureDataSize);
		std::memcpy(m_TextureData, pixel_data, pixel_data_size);
	}

	void Texture2D::SubmitTextureChanges()
	{
		if (!m_IsMutable || !m_TextureData)
			return;

		CreateTexture(m_TextureData, m_TextureFormat);
	}

	int Texture2D::GetChannelsFromFormat(const Texture2D::TextureFormat& format)
	{
		switch (format)
		{
			case RED_8:						return 1;
			case RED_GREEN_BLUE_8:			return 3;
			case RED_GREEN_BLUE_ALPHA_8:	return 4;
		}
		return 4;
	}

	Texture2D::TextureFormat Texture2D::GetFormatFromChannels(int channels)
	{
		switch (channels)
		{
			case 1: return RED_8;
			case 3: return RED_GREEN_BLUE_8;
			case 4: return RED_GREEN_BLUE_ALPHA_8;
		}

		return RED_GREEN_BLUE_ALPHA_8;
	}

	void Texture2D::ConvertAndCopyTextureData(const unsigned char* in_data, int in_width, int in_height, const TextureFormat& in_format, unsigned char*& out_data, const TextureFormat& out_format)
	{
		size_t data_size = (size_t)in_width * in_height * GetChannelsFromFormat(out_format);
		if (!out_data) 
			out_data = new unsigned char[data_size];

		if (in_format == out_format)
		{
			std::memcpy(out_data, in_data, data_size);
			return;
		}

		int in_channels = GetChannelsFromFormat(in_format);
		int out_channels = GetChannelsFromFormat(out_format);

		for (int i = 0; i < in_width * in_height; ++i)
		{
			unsigned char* out_pixel = &out_data[i * out_channels];
			const unsigned char* in_pixel = &in_data[i * in_channels];

			std::fill(out_pixel, out_pixel + out_channels, 0);

			switch (out_format)
			{

				case RED_8:
				{
					out_pixel[0] = in_pixel[0];
					break;
				}

				case RED_GREEN_BLUE_8:
				{
					switch (in_format)
					{
						case RED_8:
						{
							out_pixel[0] = static_cast<unsigned char>(in_pixel[0]);
							out_pixel[1] = 0;
							out_pixel[2] = 0;
							break;
						}
						case RED_GREEN_BLUE_ALPHA_8:
						{
							out_pixel[0] = static_cast<unsigned char>(in_pixel[0]);
							out_pixel[1] = static_cast<unsigned char>(in_pixel[1]);
							out_pixel[2] = static_cast<unsigned char>(in_pixel[2]);
							break;
						}
						case BLUE_GREEN_RED_ALPHA_8:
						{
							out_pixel[0] = static_cast<unsigned char>(in_pixel[2]);
							out_pixel[1] = static_cast<unsigned char>(in_pixel[1]);
							out_pixel[2] = static_cast<unsigned char>(in_pixel[0]);
							break;
						}
					}
					break;
				}

				case RED_GREEN_BLUE_ALPHA_8:
				{
					switch (in_format)
					{
						case RED_8:
						{
							out_pixel[0] = static_cast<unsigned char>(in_pixel[0]);
							out_pixel[1] = 0;
							out_pixel[2] = 0;
							out_pixel[3] = 255;
							break;
						}
						case RED_GREEN_BLUE_8:
						{
							out_pixel[0] = static_cast<unsigned char>(in_pixel[0]);
							out_pixel[1] = static_cast<unsigned char>(in_pixel[1]);
							out_pixel[2] = static_cast<unsigned char>(in_pixel[2]);
							out_pixel[3] = 255;
							break;
						}
						case BLUE_GREEN_RED_ALPHA_8:
						{
							out_pixel[0] = static_cast<unsigned char>(in_pixel[2]);
							out_pixel[1] = static_cast<unsigned char>(in_pixel[1]);
							out_pixel[2] = static_cast<unsigned char>(in_pixel[0]);
							out_pixel[3] = static_cast<unsigned char>(in_pixel[3]);
							break;
						}
					}
					break;
				}

				case BLUE_GREEN_RED_ALPHA_8:
				{
					switch (in_format)
					{
						case RED_8:
						{
							out_pixel[0] = 0;
							out_pixel[1] = 0;
							out_pixel[2] = static_cast<unsigned char>(in_pixel[0]);
							out_pixel[3] = 255;
							break;
						}
						case RED_GREEN_BLUE_8:
						{
							out_pixel[0] = static_cast<unsigned char>(in_pixel[2]);
							out_pixel[1] = static_cast<unsigned char>(in_pixel[1]);
							out_pixel[2] = static_cast<unsigned char>(in_pixel[0]);
							out_pixel[3] = 255;
							break;
						}
						case RED_GREEN_BLUE_ALPHA_8:
						{
							out_pixel[0] = static_cast<unsigned char>(in_pixel[2]);
							out_pixel[1] = static_cast<unsigned char>(in_pixel[1]);
							out_pixel[2] = static_cast<unsigned char>(in_pixel[0]);
							out_pixel[3] = static_cast<unsigned char>(in_pixel[3]);
							break;
						}
					}
					break;
				}

			}

		}
	}

	GLenum Texture2D::GetGLFormatType(const TextureFormat& format)
	{
		switch (format)
		{
			case RED_8:						return GL_R8;
			case RED_GREEN_BLUE_8:			return GL_RGB8;
			case RED_GREEN_BLUE_ALPHA_8:	return GL_RGBA8;
		}
		return GL_RGBA8;
	}

}