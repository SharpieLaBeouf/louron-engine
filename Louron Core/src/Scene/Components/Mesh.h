#pragma once

// Louron Core Headers
#include "../../Core/Engine.h"

#include "../../OpenGL/Texture.h"
#include "../../OpenGL/Material.h"
#include "../../OpenGL/Vertex Array.h"

#include "Components.h"

#include "../Bounds.h"

// C++ Standard Library Headers
#include <string>
#include <vector>
#include <map>

// External Vendor Library Headers
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace YAML {

	class Emitter;
	class Node;
}

namespace Louron {

#pragma region Mesh Data Structs

	struct SubMesh {

	public:

		SubMesh() = default;
		SubMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
		~SubMesh() = default;

		SubMesh(const SubMesh& other);
		SubMesh& operator=(const SubMesh& other);

		SubMesh(SubMesh&& other) noexcept;
		SubMesh& operator=(SubMesh&& other) noexcept;

		/// <summary>
		/// This will set the triangles of the mesh 
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetTriangles(uint32_t* triangles, GLuint element_count);
		
		const GLuint* GetTriangles(size_t* triangle_count) const;

		/// <summary>
		/// This will set the vertices of the mesh 
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetVertices(float* vertices, GLuint element_count);
		
		/// <summary>
		/// This will set the normals of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetNormals(float* normals, GLuint element_count);

		/// <summary>
		/// This will set the texture coords of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetTextureCoords(float* texture_coords, GLuint element_count);

		/// <summary>
		/// This will set the tangents of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetTangents(float* texture_coords, GLuint element_count);

		/// <summary>
		/// This will set the bi-tangents of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetBiTangents(float* texture_coords, GLuint element_count);

		const float* GetVertices(size_t* vertice_count) const;
		const float* GetNormals(size_t* normals_count) const;
		const float* GetTextureCoords(size_t* tex_coords_count) const;
		const float* GetTangents(size_t* tangents_count) const;
		const float* GetBiTangents(size_t* bitangents_count) const;

		void RecalculateNormals();

		/// <summary>
		/// This will retrieve all data from the GPU for 
		/// each buffer and store on CPU for modification.
		/// </summary>
		void CopyGPUData();

		/// <summary>
		/// This will clear all CPU memory associated with 
		/// each buffer attachment.
		/// </summary>
		void ClearCPUData();

		/// <summary>
		/// This will submit all changes to GPU of buffers
		/// that have been marked as modified.
		/// </summary>
		void SubmitChangesToGPU(bool clean_up_cpu_data);

		/// <summary>
		/// Set the Vertex Array by copying an existing VAO.
		/// </summary>
		void SetVAO(std::unique_ptr<VertexArray> vao) { m_VAO = std::make_unique<VertexArray>(*vao); }
		
		/// <summary>
		/// Get a const ref to the existing VAO.
		/// </summary>
		const std::unique_ptr<VertexArray>& GetVAO() const { return m_VAO; }

	private:

		std::unique_ptr<VertexArray> m_VAO = nullptr;
		bool m_Modified = false;

	};

	struct AssetMesh : public Asset {

		virtual AssetType GetType() const override { return AssetType::Mesh; }

		std::vector<std::shared_ptr<SubMesh>> SubMeshes{};

		AssetMesh() = default;

		AssetMesh(const AssetMesh& other);
		AssetMesh& operator=(const AssetMesh& other);

		AssetMesh(AssetMesh&& other) noexcept;
		AssetMesh& operator=(AssetMesh&& other) noexcept;

		~AssetMesh() = default;

		Bounds_AABB MeshBounds{};
		bool ModifiedAABB = false;

	};

#pragma endregion

#pragma region Mesh Components

	struct MeshFilterComponent : public Component {

		AssetHandle MeshFilterAssetHandle = NULL_UUID;

		Bounds_AABB TransformedAABB{};
		bool AABBNeedsUpdate = true;
		bool OctreeNeedsUpdate = true;

		void UpdateTransformedAABB();

		MeshFilterComponent() = default;
		~MeshFilterComponent() = default;

		MeshFilterComponent(const MeshFilterComponent& other) = default;
		MeshFilterComponent(MeshFilterComponent&& other) noexcept = default;

		MeshFilterComponent& operator=(const MeshFilterComponent& other) = default;
		MeshFilterComponent& operator=(MeshFilterComponent&& other) = default;

		void Serialize(YAML::Emitter& out) const;
		bool Deserialize(const YAML::Node data);

		void SetShouldDisplayDebugLines(const bool& shouldDisplay) { m_DisplayDebugAABB = shouldDisplay; }
		bool GetShouldDisplayDebugLines() const { return m_DisplayDebugAABB; }

	private:

		bool m_DisplayDebugAABB = false;

	};

	struct MeshRendererComponent : public Component {

	public:
		bool Active = true;
		std::vector<std::pair<AssetHandle, std::shared_ptr<MaterialUniformBlock>>> MeshRendererMaterialHandles;

		bool CastShadows = false;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);

	};

#pragma endregion

}