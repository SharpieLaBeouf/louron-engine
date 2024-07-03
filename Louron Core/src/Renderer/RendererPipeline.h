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

		GLuint GetRenderFBO() const;
		GLuint GetRenderColourTexture() const;
		GLuint GetRenderEntityIDTexture() const;
		GLuint GetRenderDepthTexture() const;

		// This is used to let the pipeline know if it should
		// render the screen quad with the colour attachment
		// of the scene_fbo, or not 
		void SetRenderScreenQuad(bool shouldRenderScreenQuad);

		UUID PickRenderEntityID(glm::ivec2 screenPos);

	private:

		void UpdateComputeData();

		void UpdateSSBOData();
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

			GLuint Scene_FBO = -1;
			GLuint Scene_Colour_Texture = -1;
			GLuint Scene_EntityID_Texture = -1;
			GLuint Scene_Depth_Texture = -1;

			GLuint Entity_Texture_Clear = NULL_UUID;
			std::unique_ptr<VertexArray> Screen_Quad_VAO;
			bool Render_Screen_Quad = true;

			GLuint workGroupsX = -1;
			GLuint workGroupsY = -1;

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