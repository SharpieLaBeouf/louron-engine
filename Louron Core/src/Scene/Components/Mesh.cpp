#include "Mesh.h"

// Louron Core Headers
#include "Components.h"
#include "../../Renderer/Renderer.h"
#include "../../Asset/Asset Manager API.h"

// C++ Standard Library Headers
#include <iomanip>

// External Vendor Library Headers

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron {

	SubMesh::SubMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {

		m_VAO = std::make_unique<VertexArray>();
		VertexBuffer* vbo = new VertexBuffer(vertices, (GLuint)vertices.size());
		BufferLayout layout = {
			{ ShaderDataType::Float3, "aPos" },
			{ ShaderDataType::Float3, "aNormal" },
			{ ShaderDataType::Float2, "aTexCoord" },
			{ ShaderDataType::Float3, "aTangent" },
			{ ShaderDataType::Float3, "aBitangent" }
		};
		vbo->SetLayout(layout);

		IndexBuffer* ebo = new IndexBuffer(indices, (GLuint)indices.size());

		m_VAO->AddVertexBuffer(vbo);
		m_VAO->SetIndexBuffer(ebo);
	}

	SubMesh::SubMesh(const SubMesh& other)
	{
		m_VAO = std::make_unique<VertexArray>(*other.m_VAO);
	}

	SubMesh& SubMesh::operator=(const SubMesh& other)
	{
		if (this == &other)
			return *this;

		m_VAO = std::make_unique<VertexArray>(*other.m_VAO);

		return *this;
	}

	SubMesh::SubMesh(SubMesh&& other) noexcept
	{
		m_VAO = std::move(other.m_VAO);
		other.m_VAO = nullptr;
	}

	SubMesh& SubMesh::operator=(SubMesh&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_VAO = std::move(other.m_VAO);
		other.m_VAO = nullptr;

		return *this;
	}

	void SubMesh::SetTriangles(uint32_t* triangles, GLuint element_count)
	{
		if (auto index_buffer = m_VAO->GetIndexBuffer(); index_buffer)
		{
			m_Modified = true;
			index_buffer->SetData(triangles, element_count * sizeof(uint32_t));
		}
	}

	const GLuint* SubMesh::GetTriangles(size_t* triangle_count) const
	{
		if (auto index_buffer = m_VAO->GetIndexBuffer(); index_buffer)
		{
			*triangle_count = index_buffer->GetCount();
			return static_cast<const GLuint*>(index_buffer->GetData());
		}
		*triangle_count = 0;
		return nullptr;
	}

	void SubMesh::SetVertices(float* vertices, GLuint element_count)
	{
		bool resize_buffers = false;
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aPos")
			{
				if (buffer->GetSize() < element_count * sizeof(float))
					resize_buffers = true;

				buffer->SetData(vertices, element_count * sizeof(float));

				break;
			}
		}

		if (!resize_buffers)
			return;

		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aPos") 
			{
				continue;
			}
			else if (buffer->GetLayout().GetElements().back().Name == "aTexCoord")
			{
				size_t vertex_count = element_count / 3;	// Assuming each vertex has 3 floats (x, y, z)
				size_t tex_coord_count = vertex_count * 2;	// Each vertex has 2 floats for tex coords
				buffer->ResizeData(tex_coord_count * sizeof(float));
			}
			else
			{
				buffer->ResizeData(element_count * sizeof(float));
			}
		}
	}

	void SubMesh::SetNormals(float* normals, GLuint element_count)
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aNormal")
			{
				m_Modified = true;
				buffer->SetData(normals, element_count * sizeof(float));

				break;
			}
		}
	}

	void SubMesh::SetTextureCoords(float* texture_coords, GLuint element_count)
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aTexCoord")
			{
				m_Modified = true;
				buffer->SetData(texture_coords, element_count * sizeof(float));

				break;
			}
		}
	}

	void SubMesh::SetTangents(float* texture_coords, GLuint element_count)
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aTangent")
			{
				m_Modified = true;
				buffer->SetData(texture_coords, element_count * sizeof(float));

				break;
			}
		}
	}

	void SubMesh::SetBiTangents(float* texture_coords, GLuint element_count)
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aBitangent")
			{
				m_Modified = true;
				buffer->SetData(texture_coords, element_count * sizeof(float));

				break;
			}
		}
	}

	const float* SubMesh::GetVertices(size_t* vertice_count) const
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aPos")
			{
				*vertice_count = buffer->GetSize() / sizeof(float);
				return static_cast<const float*>(buffer->GetData());
			}
		}
		*vertice_count = 0;
		return nullptr;
	}

	const float* SubMesh::GetNormals(size_t* normals_count) const
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aNormal")
			{
				*normals_count = buffer->GetSize() / sizeof(float);
				return static_cast<const float*>(buffer->GetData());
			}
		}
		*normals_count = 0;
		return nullptr;
	}

	const float* SubMesh::GetTextureCoords(size_t* tex_coords_count) const
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aTexCoord")
			{
				*tex_coords_count = buffer->GetSize() / sizeof(float);
				return static_cast<const float*>(buffer->GetData());
			}
		}
		*tex_coords_count = 0;
		return nullptr;
	}

	const float* SubMesh::GetTangents(size_t* tangents_count) const
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aTangent")
			{
				*tangents_count = buffer->GetSize() / sizeof(float);
				return static_cast<const float*>(buffer->GetData());
			}
		}
		*tangents_count = 0;
		return nullptr;
	}

	const float* SubMesh::GetBiTangents(size_t* bitangents_count) const
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			if (buffer->GetLayout().GetElements().back().Name == "aBitangent")
			{
				*bitangents_count = buffer->GetSize() / sizeof(float);
				return static_cast<const float*>(buffer->GetData());
			}
		}
		*bitangents_count = 0;
		return nullptr;
	}

	void SubMesh::RecalculateNormals() 
	{
		size_t vertice_count = 0;
		const float* vertices = GetVertices(&vertice_count); // Assuming 3 floats per vertex

		size_t indice_count = 0;
		const GLuint* indices = GetTriangles(&indice_count); // Triangle indices

		// Allocate normal buffer
		float* new_normals = new float[vertice_count * 3](); // Correct allocation with zero initialization

		// Iterate through triangles (assumes 3 indices per triangle)
		for (size_t i = 0; i < indice_count; i += 3)
		{
			GLuint i0 = indices[i];
			GLuint i1 = indices[i + 1];
			GLuint i2 = indices[i + 2];

			// Get vertex positions
			glm::vec3 v0(vertices[i0 * 3], vertices[i0 * 3 + 1], vertices[i0 * 3 + 2]);
			glm::vec3 v1(vertices[i1 * 3], vertices[i1 * 3 + 1], vertices[i1 * 3 + 2]);
			glm::vec3 v2(vertices[i2 * 3], vertices[i2 * 3 + 1], vertices[i2 * 3 + 2]);

			// Compute face normal
			glm::vec3 edge1 = v1 - v0;
			glm::vec3 edge2 = v2 - v0;
			glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

			// Accumulate normals
			new_normals[i0 * 3] += normal.x;
			new_normals[i0 * 3 + 1] += normal.y;
			new_normals[i0 * 3 + 2] += normal.z;

			new_normals[i1 * 3] += normal.x;
			new_normals[i1 * 3 + 1] += normal.y;
			new_normals[i1 * 3 + 2] += normal.z;

			new_normals[i2 * 3] += normal.x;
			new_normals[i2 * 3 + 1] += normal.y;
			new_normals[i2 * 3 + 2] += normal.z;
		}

		// Normalize vertex normals
		for (size_t i = 0; i < vertice_count; i++)
		{
			glm::vec3 normal(new_normals[i * 3], new_normals[i * 3 + 1], new_normals[i * 3 + 2]);
			normal = glm::normalize(normal);

			new_normals[i * 3] = normal.x;
			new_normals[i * 3 + 1] = normal.y;
			new_normals[i * 3 + 2] = normal.z;
		}

		// Apply new normals
		SetNormals(new_normals, vertice_count);

		delete[] new_normals;
	}

	void SubMesh::CopyGPUData()
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			buffer->GetData(); // Force loads all GPU data into CPU to prepare all data for manipulation
		}
	}

	void SubMesh::ClearCPUData()
	{
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			m_Modified = false;
			buffer->ClearData();
		}
	}

	void SubMesh::SubmitChangesToGPU(bool clean_up_cpu_data)
	{
		m_VAO->Bind();
		m_Modified = false;
		for (auto& buffer : m_VAO->GetVertexBuffers())
		{
			buffer->SubmitData();
			if (clean_up_cpu_data) buffer->ClearData();
		}

		if (auto index_buffer = m_VAO->GetIndexBuffer(); index_buffer)
		{
			index_buffer->SubmitData();
			if (clean_up_cpu_data) index_buffer->ClearData();
		}
	}

	void MeshRendererComponent::Serialize(YAML::Emitter& out) {
		out << YAML::Key << "MeshRendererComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "MeshActive" << YAML::Value << Active;
		out << YAML::Key << "CastShadows" << YAML::Value << CastShadows;

		{
			out << YAML::Key << "MaterialAssetCount" << YAML::Value << (uint32_t)MeshRendererMaterialHandles.size();
			out << YAML::Key << "MaterialAssetHandles" << YAML::Value;
			
			out << YAML::BeginSeq;
			for (const auto& [handle, uniform_block] : MeshRendererMaterialHandles) {
				out << (uint32_t)handle;
			}
			out << YAML::EndSeq;
		}

		out << YAML::EndMap;
	}

	bool MeshRendererComponent::Deserialize(const YAML::Node data)
	{
		YAML::Node component = data;

		if (component["MeshActive"]) {
			Active = component["MeshActive"].as<bool>();
		}

		if (component["CastShadows"]) {
			CastShadows = component["CastShadows"].as<bool>();
		}

		if (component["MaterialAssetCount"] && component["MaterialAssetHandles"]) {
			uint32_t count = component["MaterialAssetCount"].as<uint32_t>();
			YAML::Node handles = component["MaterialAssetHandles"];

			if (handles.size() != count) {
				return false;
			}

			MeshRendererMaterialHandles.clear();
			for (const auto& handle : handles) {
				MeshRendererMaterialHandles.push_back({ handle.as<uint32_t>() , nullptr });
			}
		}
		else {
			return false;
		}

		return true;
	}

	void MeshFilterComponent::UpdateTransformedAABB() {

		Entity entity = GetEntity();

		if (!entity || !entity.GetScene())
			return;

		if (MeshFilterAssetHandle == NULL_UUID)
			return;
		
		if (auto mesh_asset = AssetManager::GetAsset<AssetMesh>(MeshFilterAssetHandle); mesh_asset) {

			// Define the 8 corner points of the AABB
			std::array<glm::vec3, 8> corners = {
				mesh_asset->MeshBounds.BoundsMin,
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMin.y, mesh_asset->MeshBounds.BoundsMin.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMin.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMin.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMin.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMin.x, mesh_asset->MeshBounds.BoundsMin.y, mesh_asset->MeshBounds.BoundsMax.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMin.y, mesh_asset->MeshBounds.BoundsMax.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMax.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMin.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMax.z)
			};

			// Transform the corner points
			glm::mat4 global_transform = GetComponent<TransformComponent>()->GetGlobalTransform();
			for (int i = 0; i < 8; ++i) {
				glm::vec4 transformed_corner = global_transform * glm::vec4(corners[i], 1.0f);
				corners[i] = glm::vec3(transformed_corner);
			}

			// Find the new BoundsMin and BoundsMax
			glm::vec3 newMin = corners[0];
			glm::vec3 newMax = corners[0];
			for (int i = 1; i < 8; ++i) {
				newMin = glm::min(newMin, corners[i]);
				newMax = glm::max(newMax, corners[i]);
			}
			TransformedAABB.BoundsMin = newMin;
			TransformedAABB.BoundsMax = newMax;

			AABBNeedsUpdate = false;
		}
	}

	void MeshFilterComponent::Serialize(YAML::Emitter& out) const {

		out << YAML::Key << "MeshFilterComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "MeshAssetHandle" << YAML::Value << (uint32_t)MeshFilterAssetHandle;

		out << YAML::EndMap;
	}

	bool MeshFilterComponent::Deserialize(const YAML::Node data) {
		
		YAML::Node component = data;

		if (component["MeshAssetHandle"]) {
			MeshFilterAssetHandle = component["MeshAssetHandle"].as<uint32_t>();
		}
		else {
			return false;
		}

		return true;
	}

	AssetMesh::AssetMesh(const AssetMesh& other)
	{
		MeshBounds = other.MeshBounds;
		for (const auto& sub_mesh : other.SubMeshes)
			SubMeshes.push_back(std::make_shared<SubMesh>(*sub_mesh)); // Copy Sub Mesh
	}

	AssetMesh& AssetMesh::operator=(const AssetMesh& other)
	{
		if (this == &other)
			return *this;

		MeshBounds = other.MeshBounds;
		for (const auto& sub_mesh : other.SubMeshes)
			SubMeshes.push_back(std::make_shared<SubMesh>(*sub_mesh)); // Copy Sub Mesh

		return *this;
	}

	AssetMesh::AssetMesh(AssetMesh&& other) noexcept
	{
		MeshBounds = other.MeshBounds; other.MeshBounds = {};
		SubMeshes = other.SubMeshes; other.SubMeshes.clear();
	}

	AssetMesh& AssetMesh::operator=(AssetMesh&& other) noexcept
	{
		if (this == &other)
			return *this;

		MeshBounds = other.MeshBounds; other.MeshBounds = {};
		SubMeshes = other.SubMeshes; other.SubMeshes.clear();

		return *this;
	}

}