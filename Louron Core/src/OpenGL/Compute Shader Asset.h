#pragma once

// Louron Core Headers
#include "../Asset/Asset.h"
#include "../OpenGL/Shader.h"

// C++ Standard Library Headers
#include <unordered_map>

// External Vendor Library Headers
#include <glad/glad.h>

namespace Louron {

	class ComputeBuffer
	{

    public:

        ComputeBuffer(size_t element_count, size_t element_size);
        ~ComputeBuffer();

        void SetData(void* data, size_t element_count, size_t element_size) const;
        void GetData(void* output, size_t element_count, size_t element_size) const;

        void Release();

    private:

        GLuint m_SSBO_ID = -1;
        size_t m_BufferSize = -1;
        bool m_IsReleased = false;

        void Bind(GLuint binding_index = 0) const;

        friend class ComputeShaderAsset;

	};

	class ComputeShaderAsset : public Asset
	{

    public:

        ComputeShaderAsset(const std::filesystem::path& shader_file_path);
        ~ComputeShaderAsset() = default;

        Shader* GetShader() const { return m_ComputeShader.get(); }

        void SetBuffer(ComputeBuffer* buffer, GLuint binding_index);
        void Dispatch(GLuint numGroupsX, GLuint numGroupsY = 1, GLuint numGroupsZ = 1);

        virtual AssetType GetType() const override { return AssetType::Compute_Shader; }

        void Release();

    private:

        std::unique_ptr<Shader> m_ComputeShader = nullptr;
        std::unordered_map<GLuint, ComputeBuffer*> m_BoundBuffers;


	};

}