#pragma once

// Louron Core Headers
#include "../Scene/Components/Components.h"

// C++ Standard Library Headers
#include <memory>

// External Vendor Library Headers

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

		virtual void UpdateActiveScene(std::shared_ptr<Louron::Scene> scene);

	private:

		void ConductRenderPass(Camera* camera);

	protected:

		glm::uvec2 m_FrameSize{ 0, 0 };
		std::shared_ptr<Louron::Scene> m_Scene;
	};

	class ForwardPlusPipeline : public RenderPipeline {

	public:

		ForwardPlusPipeline() = default;

		void OnUpdate() override;
		void OnStartPipeline(std::shared_ptr<Louron::Scene> scene) override;
		void OnStopPipeline() override;

	private:

		void UpdateComputeData();

		void UpdateSSBOData();
		void ConductDepthPass(Camera* camera);
		void ConductLightCull(Camera* camera);
		void ConductRenderPass(Camera* camera);

	private:

		struct ForwardPlusData {

			unsigned int PL_Buffer = -1;
			unsigned int PL_Indices_Buffer = -1;
			unsigned int SL_Buffer = -1;
			unsigned int SL_Indices_Buffer = -1;

			unsigned int DL_Buffer = -1;

			unsigned int DepthMap_FBO = -1;
			unsigned int DepthMap_Texture = -1;

			unsigned int workGroupsX = -1;
			unsigned int workGroupsY = -1;

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