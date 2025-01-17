#pragma once

// Louron Core Headers
#include "../Scene/Components/Components.h"
#include "../OpenGL/Vertex Array.h"
#include "../Scene/Frustum.h"
#include "../Scene/OctreeBounds.h"

// C++ Standard Library Headers
#include <memory>

// External Vendor Library Headers
#include <glad/glad.h>

namespace Louron {

	enum L_RENDER_PIPELINE : uint8_t {
		FORWARD = 0,
		FORWARD_PLUS = 1,
		DEFERRED = 2
	};

	class Scene;
	class Entity;
	struct Bounds_AABB;
	struct Bounds_Sphere;

	class RenderPipeline {

	public:

		RenderPipeline() = default;

		virtual void OnUpdate();
		virtual void OnStartPipeline(std::shared_ptr<Louron::Scene> scene);
		virtual void OnStopPipeline();

		virtual void OnViewportResize();

		virtual void UpdateActiveScene(std::shared_ptr<Louron::Scene> scene);

	private:

		void ConductRenderPass(Camera* camera);

	protected:

		std::weak_ptr<Louron::Scene> m_Scene;
	};

	class ForwardPlusPipeline : public RenderPipeline {

	public:

		ForwardPlusPipeline() = default;

		void OnUpdate() override;
		void OnStartPipeline(std::shared_ptr<Louron::Scene> scene) override;
		void OnStopPipeline() override;

		void OnViewportResize() override;

	private:

		void UpdateComputeData();

		void UpdateSSBOData();
		void ConductLightFrustumCull();
		void ConductRenderableFrustumCull();
		void ConductDepthPass(Camera* camera);
		void ConductTiledBasedLightCull(Camera* camera);
		void ConductRenderPass(Camera* camera);

		void RenderFBOQuad();

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

			std::vector<Entity> RenderableEntities;
			std::vector<Entity> PLEntities;
			std::vector<Entity> SLEntities;

			Frustum Camera_Frustum{};

			bool ShowLightComplexity = false;
			bool ShowWireframe = false;

			std::thread OctreeUpdateThread;
			std::vector<std::shared_ptr<OctreeDataSource<Entity>>> OctreeEntitiesInCamera;

		} FP_Data;

	public:

		ForwardPlusData& GetFPData() { return FP_Data; }

	};

	class DeferredPipeline : public RenderPipeline {

	public:

		DeferredPipeline() = default;

		void OnUpdate() override;
		void OnStartPipeline(std::shared_ptr<Louron::Scene> scene) override;
		void OnStopPipeline() override;

	private:

	};

}