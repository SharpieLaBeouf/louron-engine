#pragma once

namespace Louron {

	class RendererPipeline {

		virtual void OnUpdate() {
			// Standard Forward Rendering Pass
		}

	public:

		RendererPipeline() = default;
	};

	class ForwardPlusPipeline : public RendererPipeline {

	public:

		void OnUpdate() override {
			// Forward Plus Rendering Pass
		}

	public:

		ForwardPlusPipeline() = default;
	};

	class DeferredPipeline : public RendererPipeline {

	public:

		void OnUpdate() override {
			// Deferred Rendering Pass
		}

	public:

		DeferredPipeline() = default;
	};

}