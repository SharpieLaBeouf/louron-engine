#pragma once

// Louron Core Headers
#include "../Asset/Asset.h"

// C++ Standard Library Headers
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>

// External Vendor Library Headers
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Louron 
{

	class Texture2D : public Asset
	{

	public:

		enum TextureFormat : uint8_t
		{
			RED_8 = 0,
			RED_GREEN_BLUE_8,
			RED_GREEN_BLUE_ALPHA_8,

			BLUE_GREEN_RED_ALPHA_8 // Embedded Assimp Textures
		};

		enum TextureProperty : uint8_t
		{
			Wrap_Repeat			= 1U << 0,
			Wrap_Mirror			= 1U << 1,
			Wrap_ClampEdge		= 1U << 2,
			Wrap_ClampBorder	= 1U << 3,

			Filter_Linear		= 1U << 4,
			Filter_Nearest		= 1U << 5,

			Anisotropy_Enabled	= 1U << 6,

			Default = Wrap_Repeat | Filter_Linear | Anisotropy_Enabled,
		};

	public:

		virtual AssetType GetType() const override { return AssetType::Texture2D; }

		Texture2D();

		/// <summary>
		/// Create a blank texture with a specified format.
		/// </summary>
		/// <param name="internal_format">The format of the texture in OpenGL.</param>
		/// <param name="keep_data">Whether the CPU should retain a copy of the data for manipulation.</param>
		Texture2D(int width, int height, const TextureFormat& internal_format, bool keep_data = false);
				
		/// <summary>
		/// Creates a texture using user defined texture data.
		/// Texture data will be copied  into the GPU, and may 
		/// be copied locally in the texture on CPU side if 
		/// requested. This is useful for runtime created textures.
		/// </summary>
		/// <param name="data">Caller is responsible of releaseing texture data after creating the texture.</param>
		/// <param name="internal_format">The format of the texture in OpenGL.</param>
		/// <param name="data_format">The format of the data passed to the Constructor.</param>
		/// <param name="keep_data">Whether the CPU should retain a copy of the data for manipulation.</param>
		Texture2D(const unsigned char* data, int width, int height, const TextureFormat& internal_format, const TextureFormat& data_format, bool keep_data = false);

		/// <summary>
		/// Create a texture from a filepath.
		/// </summary>
		/// <param name="keep_data">Whether the CPU should retain a copy of the data for manipulation.</param>
		Texture2D(const std::filesystem::path& texture_path, bool keep_data = false);

		~Texture2D();

		void Bind() const { glBindTexture(GL_TEXTURE_2D, m_TextureId); }
		void UnBind() const { glBindTexture(GL_TEXTURE_2D, 0); }
		const GLuint& GetID() const { return m_TextureId; }

		bool IsMutable() const { return m_IsMutable; }

		void SetWrapMode(GLenum wrap_mode);
		void SetFilterMode(GLenum filter_mode);
		void SetAnisotropic(bool enable_anisotropic_filtering);

		GLenum GetWrapMode() const;
		GLenum GetFilterMode() const;
		bool GetAnisotropic() const;

		const TextureFormat& GetFormat() const;

		operator bool() const { return (m_TextureId != -1); }

	public: // Mutable Texture Data Manipulation

		/// <summary>
		/// Set's an individual pixel to a particular colour. You must pass the 
		/// format of the pixel data you have provided to the function.
		/// 
		/// The VEC4 will always be interpreted as an RGBA format as 0.0-1.0. 
		/// Values will be clamped between 0.0 -> 1.0 and then mul by 255.
		/// </summary>
		/// <param name="pixel_colour">The Colour of the pixel in a VEC4.</param>
		/// <param name="pixel_coord">The coordinate starting from 0 ranging to texture size.x||y - 1.</param>
		void SetPixel(const glm::vec4& pixel_colour, const glm::ivec2& pixel_coord);
		void SetPixelData(const unsigned char* pixel_data, const size_t& pixel_data_size, const TextureFormat& pixel_data_format);

		void SubmitTextureChanges();

	private:

		void CreateTexture(const unsigned char* data, const TextureFormat& data_format);

		static GLenum GetGLFormatType(const TextureFormat& format);

		static int GetChannelsFromFormat(const TextureFormat& format);
		static TextureFormat GetFormatFromChannels(int channels);

		static void ConvertAndCopyTextureData(const unsigned char* in_data, int in_width, int in_height, const TextureFormat& in_format, unsigned char*& out_data, const TextureFormat& out_format);

	private:

		glm::ivec2 m_TextureSize = { 1, 1 };
		size_t m_TextureDataSize = 0;

		GLuint m_TextureId = -1;

		TextureFormat m_TextureFormat = RED_GREEN_BLUE_ALPHA_8;
		TextureProperty m_TextureProperties = TextureProperty::Default;

		/// <summary>
		/// Is the data copied into the CPU for direct manipulation?
		/// </summary>
		bool m_IsMutable = false;

		/// <summary>
		/// This is only maintained in CPU if we specify to do so when 
		/// constructing a texture with data already passed in.
		/// 
		/// This is handy for custom textures we want to create
		/// and modify during run-time.
		/// </summary>
		unsigned char* m_TextureData = nullptr;

	};

}