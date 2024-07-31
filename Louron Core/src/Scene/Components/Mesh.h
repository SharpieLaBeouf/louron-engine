#pragma once

// Louron Core Headers
#include "Camera.h"

#include "../../Core/Engine.h"

#include "../../OpenGL/Texture.h"
#include "../../OpenGL/Material.h"
#include "../../OpenGL/Vertex Array.h"

#include "Components.h"

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

#pragma region Bound Structures

	struct Bounds_Sphere {

		glm::vec3 BoundsCentre = glm::vec3(0.0f);
		float BoundsRadius = 0.0f;

		Bounds_Sphere() = default;
		Bounds_Sphere(const glm::vec3& centre, const float& radius) : BoundsCentre(centre), BoundsRadius(radius) {}
		Bounds_Sphere(const Bounds_Sphere&) = default;
		Bounds_Sphere(Bounds_Sphere&&) = default;
		~Bounds_Sphere() = default;
	};

	struct Bounds_AABB {
		glm::vec3 BoundsMin = glm::vec3(-1.0f);
		glm::vec3 BoundsMax = glm::vec3(1.0f);

		Bounds_AABB() = default;
		Bounds_AABB(const glm::vec3& min, const glm::vec3& max) : BoundsMin(min), BoundsMax(max) {}
		Bounds_AABB(const Bounds_AABB&) = default;
		Bounds_AABB(Bounds_AABB&&) noexcept = default; // Explicitly declare move constructor
		Bounds_AABB& operator=(const Bounds_AABB&) = default;
		Bounds_AABB& operator=(Bounds_AABB&&) noexcept = default; // Explicitly declare move assignment operator
		~Bounds_AABB() = default;

		/// <summary>
		/// Calculate the Centre of the AABB
		/// </summary>
		/// <returns></returns>
		glm::vec3 Center() const {
			return (BoundsMin + BoundsMax) * 0.5f;
		}
			
		/// <summary>
		/// Calculate the Size of the AABB
		/// </summary>
		glm::vec3 Size() const {
			return BoundsMax - BoundsMin;
		}

		/// <summary>
		/// Get the transformation matrix for this AABB
		/// </summary>
		glm::mat4 GetGlobalTransform() const {
			glm::vec3 center = Center();
			glm::vec3 size = Size();

			// Create the transformation matrix
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, center);
			transform = glm::scale(transform, size);

			return transform;
		}
	};

#pragma endregion

#pragma region Mesh Data Structs

	struct SubMesh {

		std::unique_ptr<VertexArray> VAO = nullptr;

		SubMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
		SubMesh(const SubMesh&) = default;
		SubMesh(SubMesh&&) = default;
		~SubMesh() = default;

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

	struct AssetMeshFilter : public Component {

		AssetHandle MeshFilterAssetHandle = NULL_UUID;

		Bounds_AABB TransformedAABB;
		bool AABBNeedsUpdate = true;

		void UpdateTransformedAABB();

		AssetMeshFilter() = default;
		~AssetMeshFilter() = default;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);

		void SetShouldDisplayDebugLines(const bool& shouldDisplay) { m_DisplayDebugAABB = shouldDisplay; }
		bool GetShouldDisplayDebugLines() const { return m_DisplayDebugAABB; }

	private:

		bool m_DisplayDebugAABB = false;

	};

	struct AssetMeshRenderer : public Component {

	public:
		bool Active = true;
		std::vector<AssetHandle> MeshRendererMaterialHandles;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);

	};

#pragma endregion

}