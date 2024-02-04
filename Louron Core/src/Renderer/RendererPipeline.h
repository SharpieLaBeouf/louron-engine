#pragma once

#include <memory>

#include "../Scene/Components.h"

namespace Louron {

	class Scene;

	class RenderPipeline {
	public:

		RenderPipeline() = default;

		virtual void OnUpdate(Scene* scene) { }
		virtual void OnStartPipeline() { }
		virtual void OnStopPipeline() { }
	};

	class ForwardPlusPipeline : public RenderPipeline {

	public:

		ForwardPlusPipeline() = default;

		void OnUpdate(Scene* scene) override;

		void OnStartPipeline() override;
		void OnStopPipeline() override;

	private:

		void UpdateSSBOData(Scene* scene);
		void ConductDepthPass(Scene* scene, Camera* camera);
		void ConductLightCull(Camera* camera);
		void ConductRenderPass(Scene* scene, Camera* camera);

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

		void OnUpdate(Scene* scene) override {
			// Deferred Rendering Pass
		}

	public:

		DeferredPipeline() = delete;
		DeferredPipeline(std::shared_ptr<Scene> scene);
	};

}