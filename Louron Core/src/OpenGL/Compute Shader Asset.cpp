#include "Compute Shader Asset.h"

#include "../Debug/Profiler.h"
#include "../Debug/Assert.h"

#include <memory>

namespace Louron {

#pragma region Compute Buffer

    ComputeBuffer::ComputeBuffer(size_t element_count, size_t element_size)
    {
        m_BufferSize = element_count * element_size;
        glGenBuffers(1, &m_SSBO_ID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_ID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_DRAW);
    }

    ComputeBuffer::~ComputeBuffer() 
    {
        Release();
    }

    void ComputeBuffer::SetData(void* data, size_t element_count, size_t element_size) const
    {
        if (m_IsReleased) 
            return;

        if (element_count * element_size > m_BufferSize)
            return;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_ID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, element_count * element_size, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void ComputeBuffer::GetData(void* output, size_t element_count, size_t element_size) const
    {
        if (m_IsReleased)
            return;

        if (element_count * element_size > m_BufferSize)
            return;

        glFinish();

        // Read back data using glGetBufferSubData
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_ID);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, element_count * element_size, output);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void ComputeBuffer::Bind(GLuint binding_index) const
    {
        if (m_IsReleased) 
            return;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_ID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_index, m_SSBO_ID);
    }

    void ComputeBuffer::Release() 
    {
        if (!m_IsReleased) {
            glDeleteBuffers(1, &m_SSBO_ID);
            m_IsReleased = true;
        }
    }

#pragma endregion

#pragma region Compute Shader Asset

    ComputeShaderAsset::ComputeShaderAsset(const std::filesystem::path& shader_file_path)
    {
        m_ComputeShader = std::make_unique<Shader>(shader_file_path.string().c_str(), true);
    }

    void ComputeShaderAsset::SetBuffer(ComputeBuffer* buffer, GLuint binding_index) {

        if (!m_ComputeShader || !buffer)
            return;
        
        m_BoundBuffers[binding_index] = buffer;
    }

    void ComputeShaderAsset::Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) {

        if (!m_ComputeShader)
            return;

        m_ComputeShader->Bind();

        for (auto& [binding_index, buffer] : m_BoundBuffers) {

            if (!buffer)
                continue;

            buffer->Bind(binding_index);
        }

        m_BoundBuffers.clear(); // Clear for each dispatch call

        glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
        
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        m_ComputeShader->UnBind();
    }

    void ComputeShaderAsset::Release()
    {
        if (m_ComputeShader)
        {
            m_ComputeShader.reset();
            m_ComputeShader = nullptr;
        }

    }

#pragma endregion

}