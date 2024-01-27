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

		ForwardPlusPipeline();

		void OnUpdate(Scene* scene) override;

		void OnStartPipeline() override;
		void OnStopPipeline() override;

	private:

		void BindLightSSBO(Scene* scene);
		void ConductDepthPass(Scene* scene, Camera* camera);
		void ConductLightCull(Camera* camera);
		void ConductRenderPass(Scene* scene, Camera* camera);

	private:

		struct ForwardPlusData {

			unsigned int PL_Buffer;
			unsigned int PL_Indices_Buffer;
			unsigned int SL_Buffer;
			unsigned int SL_Indices_Buffer;

			unsigned int DL_Buffer;

			unsigned int DepthMap_FBO;
			unsigned int DepthMap_Texture;

			unsigned int workGroupsX;
			unsigned int workGroupsY;

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