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

		std::unique_ptr<VertexArray> VAO = nullptr;

		SubMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
		~SubMesh() = default;

		SubMesh(const SubMesh&) = default;
		SubMesh& operator=(const SubMesh& other) = default;

		SubMesh(SubMesh&&) = default;
		SubMesh& operator=(SubMesh&& other) = default;

	};

	struct AssetMesh : public Asset {

		virtual AssetType GetType() const override { return AssetType::Mesh; }

		std::vector<std::shared_ptr<SubMesh>> SubMeshes{};

		AssetMesh() = default;
		AssetMesh(const AssetMesh&) = default;
		AssetMesh(AssetMesh&&) = default;
		~AssetMesh() = default;

		Bounds_AABB MeshBounds{};

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