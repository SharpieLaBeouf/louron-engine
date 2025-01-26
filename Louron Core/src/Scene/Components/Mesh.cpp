#include "Mesh.h"

// Louron Core Headers
#include "Components.h"
#include "../../Renderer/Renderer.h"
#include "../../Project/Project.h"

// C++ Standard Library Headers
#include <iomanip>

// External Vendor Library Headers

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron {

	SubMesh::SubMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {

		VAO = std::make_unique<VertexArray>();
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

		VAO->AddVertexBuffer(vbo);
		VAO->SetIndexBuffer(ebo);
	}

	void AssetMeshRenderer::Serialize(YAML::Emitter& out) {
		out << YAML::Key << "MeshRendererComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "MeshActive" << YAML::Value << Active;
		out << YAML::Key << "CastShadows" << YAML::Value << CastShadows;

		{
			out << YAML::Key << "MaterialAssetCount" << YAML::Value << (uint32_t)MeshRendererMaterialHandles.size();
			out << YAML::Key << "MaterialAssetHandles" << YAML::Value;
			
			out << YAML::BeginSeq;
			for (const auto& handle : MeshRendererMaterialHandles) {
				out << (uint32_t)handle;
			}
			out << YAML::EndSeq;
		}

		out << YAML::EndMap;
	}

	bool AssetMeshRenderer::Deserialize(const YAML::Node data)
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
				MeshRendererMaterialHandles.push_back(handle.as<uint32_t>());
			}
		}
		else {
			return false;
		}

		return true;
	}

	void AssetMeshFilter::UpdateTransformedAABB() {

		Entity entity = GetEntity();

		if (!entity || !entity.GetScene())
			return;

		if (MeshFilterAssetHandle == NULL_UUID)
			return;
		
		if (auto mesh_asset = Project::GetStaticEditorAssetManager()->GetAsset<AssetMesh>(MeshFilterAssetHandle); mesh_asset) {

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

	void AssetMeshFilter::Serialize(YAML::Emitter& out) const {

		out << YAML::Key << "MeshFilterComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "MeshAssetHandle" << YAML::Value << (uint32_t)MeshFilterAssetHandle;

		out << YAML::EndMap;
	}

	bool AssetMeshFilter::Deserialize(const YAML::Node data) {
		
		YAML::Node component = data;

		if (component["MeshAssetHandle"]) {
			MeshFilterAssetHandle = component["MeshAssetHandle"].as<uint32_t>();
		}
		else {
			return false;
		}

		return true;
	}
}