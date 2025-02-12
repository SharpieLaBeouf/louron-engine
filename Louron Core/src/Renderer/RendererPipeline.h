#pragma once

// Louron Core Headers
#include "../Scene/Components/Components.h"
#include "../OpenGL/Material.h"
#include "../OpenGL/Query.h"
#include "../OpenGL/Vertex Array.h"
#include "../Scene/Frustum.h"
#include "../Scene/OctreeBounds.h"

// C++ Standard Library Headers
#include <memory>

// External Vendor Library Headers
#include <glad/glad.h>

// So we can Key a Pair of Material and Uniform Block
struct _MaterialWrapper {
	std::shared_ptr<Louron::PBRMaterial> material;
	std::shared_ptr<Louron::MaterialUniformBlock> uniform_block;

	// Compare the actual contents, not just pointers
	bool operator==(const _MaterialWrapper& other) const {
		return material.get() == other.material.get() && uniform_block.get() == other.uniform_block.get();
	}
};

namespace std {
	template <>
	struct hash<_MaterialWrapper> {
		std::size_t operator()(const _MaterialWrapper& mw) const {
			return std::hash<std::shared_ptr<Louron::PBRMaterial>>{}(mw.material) ^ (std::hash<std::shared_ptr<Louron::MaterialUniformBlock>>{}(mw.uniform_block) << 1);
		}
	};
}

namespace Louron {

	enum L_RENDER_PIPELINE : uint8_t {
		FORWARD = 0,
		FORWARD_PLUS = 1,
		DEFERRED = 2
	};

	class Scene;
	class Entity;
	class CameraBase;

	struct AssetMesh;
	struct SubMesh;
	struct Bounds_AABB;
	struct Bounds_Sphere;

	class RenderPipeline {

	public:

		RenderPipeline() = default;

		virtual void OnUpdate(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix);
		virtual void OnStartPipeline(std::shared_ptr<Louron::Scene> scene);
		virtual void OnStopPipeline();

		virtual void OnViewportResize();

		virtual void UpdateActiveScene(std::shared_ptr<Louron::Scene> scene);

		virtual void RenderFBOQuad() {}

	private:

		void ConductRenderPass(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix);

	protected:

		std::weak_ptr<Louron::Scene> m_Scene;
	};

	using DepthRenderQueue = std::vector<std::tuple<float, UUID>>;
	using OpaqueRenderQueue = std::unordered_map<_MaterialWrapper, std::unordered_map<std::shared_ptr<SubMesh>, std::vector<UUID>>>;
	using TransparentRenderQueue = std::vector<std::tuple<float, _MaterialWrapper, std::shared_ptr<SubMesh>, UUID>>;
	using GeometryQueryMap = std::unordered_map<UUID, Query>;

	class ForwardPlusPipeline : public RenderPipeline {

	public:

		ForwardPlusPipeline() = default;

		void OnUpdate(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix) override;
		void OnStartPipeline(std::shared_ptr<Louron::Scene> scene) override;
		void OnStopPipeline() override;

		void OnViewportResize() override;

		void RenderFBOQuad() override;

	private:

		void UpdateComputeData();

		void UpdateSSBOData();
		void ConductLightFrustumCull();
		void ConductRenderableFrustumCull(const glm::vec3& camera_position, const glm::mat4& projection_matrix);
		void ConductRenderableOcclusionCull();
		void ConductDepthPass(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix);
		void ConductTiledBasedLightCull(const glm::mat4& projection_matrix, const glm::mat4& view_matrix);
		void ConductShadowMapping(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix);
		void ConductRenderPass(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix);

		bool IsSphereInsideFrustum(const Bounds_Sphere& bounds, const Frustum& frustum);

	private:

		struct ForwardPlusData {

			GLuint DL_Buffer = -1;

			GLuint PL_Buffer = -1;	// Buffer that holds all lights in the scene
			GLuint PL_Indices_Buffer = -1;	// Buffer that holds light indices for each tile

			GLuint SL_Buffer = -1;
			GLuint SL_Indices_Buffer = -1;	// Buffer that holds light indices for each tile

			std::unique_ptr<VertexArray> Screen_Quad_VAO;

			GLuint workGroupsX = -1;
			GLuint workGroupsY = -1;

			GeometryQueryMap EntityOcclusionQueries;

			// TODO: Consider Unordered Set for O(1) opposed to O(n)
			std::vector<Entity> RenderableEntitiesInFrustum;
			std::vector<Entity> PLEntitiesInFrustum;
			std::vector<Entity> SLEntitiesInFrustum;
			std::vector<Entity> DLEntities;

			Frustum Camera_Frustum{};

			bool Debug_ShowLightComplexity = false;
			bool Debug_ShowWireframe = false;
			std::vector<glm::mat4> Debug_RenderAABB;

			DepthRenderQueue DepthRenderables;
			OpaqueRenderQueue OpaqueRenderables;
			TransparentRenderQueue TransparentRenderables;
			std::mutex RenderSortingMutex;
			std::thread RenderQueueSortingThread;

			// Cached weak ptr's to reduce AssetManager Get Calls
			std::unordered_map<AssetHandle, std::weak_ptr<AssetMesh>> CachedMeshAssets;
			std::unordered_map<AssetHandle, std::weak_ptr<PBRMaterial>> CachedMaterialAssets;

			std::thread OctreeUpdateThread;
			std::vector<std::shared_ptr<OctreeDataSource<Entity>>> OctreeEntitiesInCamera;

			GLuint PL_Shadow_Max_Maps = 5;
			GLuint PL_Shadow_Map_Res = 1024;
			GLuint PL_Shadow_FrameBuffer = -1;
			GLuint PL_Shadow_CubeMap_Array = -1;
			std::unordered_map<UUID, GLuint> PL_Shadow_LightIndexMap;

			GLuint SL_Shadow_Max_Maps = 30;
			GLuint SL_Shadow_Map_Res = 1024;
			GLuint SL_Shadow_FrameBuffer = -1;
			GLuint SL_Shadow_Texture_Array = -1;
			GLuint SL_Shadow_LightSpaceMatrix_Buffer = -1;	// Buffer that holds light space matrice for each directional light cascade
			std::unordered_map<UUID, GLuint> SL_Shadow_LightIndexMap;

			GLuint DL_Shadow_Max_Maps = 5;
			GLuint DL_Shadow_Map_Res = 1024;
			GLuint DL_Shadow_FrameBuffer = -1;
			GLuint DL_Shadow_Texture_Array = -1;
			GLuint DL_Shadow_LightSpaceMatrix_Buffer = -1;	// Buffer that holds light space matrice for each directional light cascade

			std::unordered_map<UUID, GLuint> DL_Shadow_LightSpaceMatrixIndex;
			std::unordered_map<UUID, std::array<float, 5>> DL_Shadow_LightShadowCascadeDistances;

		} FP_Data;

	public:

		ForwardPlusData& GetFPData() { return FP_Data; }

	};

	class DeferredPipeline : public RenderPipeline {

	public:

		DeferredPipeline() = default;

		void OnUpdate(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix) override;
		void OnStartPipeline(std::shared_ptr<Louron::Scene> scene) override;
		void OnStopPipeline() override;

	private:

	};

}