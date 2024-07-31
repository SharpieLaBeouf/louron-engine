#pragma once

// Louron Core Headers
#include "../Scene/Components/Components.h"
#include "../OpenGL/Vertex Array.h"

// C++ Standard Library Headers
#include <memory>

// External Vendor Library Headers
#include <glad/glad.h>

namespace Louron {

	enum L_RENDER_PIPELINE {
		FORWARD = 0,
		FORWARD_PLUS = 1,
		DEFERRED = 2
	};

	class Scene;

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

		std::shared_ptr<Louron::Scene> m_Scene;
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
		void ConductRenderableCull(Camera* camera, std::vector<Entity>* renderables);
		void ConductDepthPass(Camera* camera);
		void ConductLightCull(Camera* camera);
		void ConductRenderPass(Camera* camera);

		void RenderFBOQuad();

	private:

		struct ForwardPlusData {

			GLuint PL_Buffer = -1;
			GLuint PL_Indices_Buffer = -1;
			GLuint SL_Buffer = -1;
			GLuint SL_Indices_Buffer = -1;

			GLuint DL_Buffer = -1;

			std::unique_ptr<VertexArray> Screen_Quad_VAO;

			GLuint workGroupsX = -1;
			GLuint workGroupsY = -1;

			std::vector<Entity> RenderableEntities;

		} FP_Data;

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