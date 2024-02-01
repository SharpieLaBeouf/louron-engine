#include "RendererPipeline.h"

#include "Renderer.h"

#include "../Scene/Entity.h"
#include <entt/entt.hpp>

namespace Louron {

	ForwardPlusPipeline::ForwardPlusPipeline() {

		FP_Data.workGroupsX = (unsigned int)std::ceil((float)Engine::Get().GetWindow().GetWidth() / 16.0f);
		FP_Data.workGroupsY = (unsigned int)std::ceil((float)Engine::Get().GetWindow().GetHeight() / 16.0f);
		size_t numberOfTiles = static_cast<size_t>(FP_Data.workGroupsX * FP_Data.workGroupsY);

		// Setup Light Buffers

		glGenBuffers(1, &FP_Data.PL_Buffer);
		glGenBuffers(1, &FP_Data.PL_Indices_Buffer);

		glGenBuffers(1, &FP_Data.SL_Buffer);
		glGenBuffers(1, &FP_Data.SL_Indices_Buffer);

		glGenBuffers(1, &FP_Data.DL_Buffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_POINT_LIGHTS * sizeof(PointLightComponent), 0, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_POINT_LIGHTS, 0, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_SPOT_LIGHTS * sizeof(SpotLightComponent), 0, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_SPOT_LIGHTS, 0, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_DIRECTIONAL_LIGHTS * sizeof(DirectionalLightComponent), 0, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// Setup Depth Texture

		glGenFramebuffers(1, &FP_Data.DepthMap_FBO);
		glGenTextures(1, &FP_Data.DepthMap_Texture);

		glBindTexture(GL_TEXTURE_2D, FP_Data.DepthMap_Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Engine::Get().GetWindow().GetWidth(), Engine::Get().GetWindow().GetHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.DepthMap_FBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FP_Data.DepthMap_Texture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	void ForwardPlusPipeline::OnUpdate(Scene* scene) {
		
		Camera* camera = nullptr;
		{
			const auto& view = scene->GetRegistry()->view<Transform, CameraComponent>();
			for (const auto& entity : view) {
				const auto& temp_camera = view.get<CameraComponent>(entity);

				if (temp_camera.Primary) {
					camera = temp_camera.Camera.get();
					break;
				}
			}
		}

		ConductDepthPass(scene, camera);
		BindLightSSBO(scene);
		ConductLightCull(camera); // WIP for Spot Lights
		ConductRenderPass(scene, camera);
	}

	/// <summary>
	/// Set OpenGL state configuration required by renderer.
	/// </summary>
	void ForwardPlusPipeline::OnStartPipeline() {

		glEnable(GL_DEPTH_TEST); 

	}

	/// <summary>
	/// Reset OpenGL state configuration required by renderer.
	/// </summary>
	void ForwardPlusPipeline::OnStopPipeline() {

		glDisable(GL_DEPTH_TEST);
	}

	void ForwardPlusPipeline::BindLightSSBO(Scene* scene) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, FP_Data.PL_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, FP_Data.PL_Indices_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, FP_Data.SL_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, FP_Data.SL_Indices_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, FP_Data.DL_Buffer);

		// Lights
		{
			// Point Lights
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Buffer);
				PointLightComponent* pointLights = (PointLightComponent*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

				if (pointLights) {
					auto view = scene->GetRegistry()->view<Transform, PointLightComponent>();

					int i = 0;
					for (auto entity : view) {
						auto [transform, point_light] = view.get<Transform, PointLightComponent>(entity);

						point_light.position = glm::vec4(transform.GetPosition(), 1.0f);

						pointLights[i] = point_light;
						pointLights[i].lightProperties.lastLight = false;

						i++;

						if (i >= MAX_POINT_LIGHTS)
							break;
					}
					pointLights[i].lightProperties.lastLight = true;
				}
				else {
					std::cout << "[L20] Point Light Buffer Not Mapped Successfully!" << std::endl;
				}

				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// Spot Lights
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Buffer);
				SpotLightComponent* spotLights = (SpotLightComponent*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

				if (spotLights) {
					auto view = scene->GetRegistry()->view<Transform, SpotLightComponent>();

					int i = 0;
					for (auto entity : view) {
						auto [transform, spot_light] = view.get<Transform, SpotLightComponent>(entity);

						spot_light.position = glm::vec4(transform.GetPosition(), 1.0f);

						spotLights[i] = spot_light;
						spotLights[i].lastLight = false;

						i++;

						if (i >= MAX_SPOT_LIGHTS)
							break;
					}
					spotLights[i].lastLight = true;
				}
				else {
					std::cout << "[L20] Spot Light Buffer Not Mapped Successfully!" << std::endl;
				}

				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// Directional Lights
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Buffer);
				DirectionalLightComponent* directionalLights = (DirectionalLightComponent*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

				if (directionalLights) {
					auto view = scene->GetRegistry()->view<Transform, DirectionalLightComponent>();

					int i = 0;
					for (auto entity : view) {

						directionalLights[i] = view.get<DirectionalLightComponent>(entity);
						directionalLights[i].lastLight = false;
						directionalLights[i].direction = glm::normalize(
							glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) *
							glm::quat(glm::radians(view.get<Transform>(entity).GetRotation())));

						i++;

						if (i >= MAX_DIRECTIONAL_LIGHTS)
							break;
					}
					directionalLights[i].lastLight = true;
				}
				else {
					std::cout << "[L20] Spot Light Buffer Not Mapped Successfully!" << std::endl;
				}

				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
		}

	}

	void ForwardPlusPipeline::ConductDepthPass(Scene* scene, Camera* camera) {
		if (camera) {

			// Call Renderer for all Meshes
			{
				const auto& view = scene->GetRegistry()->view<Transform, MeshRenderer, MeshFilter>();

				// First: Distance, Second: Transform, Third: MeshFilter
				std::vector<std::tuple<float, Transform, MeshFilter>> sortedEntities;

				if (view.begin() != view.end()) {

					for (const auto& entity : view) {
						const auto& [transform, meshRenderer, meshFilter] = view.get<Transform, MeshRenderer, MeshFilter>(entity);

						if (!meshRenderer.active)
							continue;

						glm::vec3 objectPosition = transform.GetPosition();
						float distance = glm::length(objectPosition - camera->GetPosition());

						sortedEntities.emplace_back(distance, transform, meshFilter);
					}

					// Lambda function compares the distances of two entities (a and b) and orders them in a way that ensures front-to-back sorting
					std::sort(sortedEntities.begin(), sortedEntities.end(), [](const auto& a, const auto& b) {

						return std::get<0>(a) < std::get<0>(b);

					});

					std::shared_ptr<Shader> shader = Engine::Get().GetShaderLibrary().GetShader("FP_Depth");

					if (shader)
					{
						glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.DepthMap_FBO);
						glClear(GL_DEPTH_BUFFER_BIT);

						shader->Bind();
						shader->SetMat4("u_Proj", camera->GetProjMatrix());
						shader->SetMat4("u_View", camera->GetViewMatrix());

						for (const auto& [distance, transform, meshFilter] : sortedEntities) {
							shader->SetMat4("u_Model", transform.GetTransform());

							for (const auto& mesh : *meshFilter.Meshes)
								Renderer::DrawMesh(mesh);
						}

						shader->UnBind();

						glBindFramebuffer(GL_FRAMEBUFFER, 0);
					}
				}
			}
		}
	}

	void ForwardPlusPipeline::ConductLightCull(Camera* camera) {
		
		// Conduct Light Cull
		std::shared_ptr<Shader> lightCull = Engine::Get().GetShaderLibrary().GetShader("FP_Light_Culling");

		lightCull->Bind();

		lightCull->SetMat4("u_View", camera->GetViewMatrix());
		lightCull->SetMat4("u_Proj", camera->GetProjMatrix());
		lightCull->SetiVec2("u_ScreenSize", glm::ivec2((int)Engine::Get().GetWindow().GetWidth(), (int)Engine::Get().GetWindow().GetHeight()));
		glActiveTexture(GL_TEXTURE4);
		lightCull->SetInt("u_Depth", 4);
		glBindTexture(GL_TEXTURE_2D, FP_Data.DepthMap_Texture);

		glDispatchCompute(FP_Data.workGroupsX, FP_Data.workGroupsY, 1);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void ForwardPlusPipeline::ConductRenderPass(Scene* scene, Camera* camera) {
		
		// Render All MeshComponents in Scene
		if (camera) {

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Gather all entities within scene with appropriate components
			auto view = scene->GetRegistry()->view<Transform, MeshRenderer, MeshFilter>();
			if (view.begin() != view.end()) {
				std::unordered_map<std::shared_ptr<Material>, std::unordered_map<std::shared_ptr<Mesh>, std::vector<Transform>>> materialMeshTransMap;

				// SORT MESHES BY MATERIAL: Loop entities and access relevant components required for rendering
				for (const auto& entity : view) {
					auto [transform, meshRenderer, meshFilter] = view.get<Transform, MeshRenderer, MeshFilter>(entity);

					// Check if MeshRenderer is toggled for rendering, if not, continue to next entity
					if (!meshRenderer.active)
						continue;

					// Loop through each submesh within the meshfilter
					for (auto& subMesh : *meshFilter.Meshes)
						if (!(*meshRenderer.Materials).empty())
							if ((*meshRenderer.Materials)[subMesh->MaterialIndex] != nullptr) {
								std::shared_ptr<Material> temp_mat = (*meshRenderer.Materials)[subMesh->MaterialIndex];
								materialMeshTransMap[(*meshRenderer.Materials)[subMesh->MaterialIndex]][subMesh].push_back(transform);
							}
							else {
								std::cout << "[L20] Mesh with invalid material encountered!" << std::endl;
							}
				}

				// Colour Render Pass
				for (const auto& materialMeshPair : materialMeshTransMap) {
					const auto& material = materialMeshPair.first;
					const auto& meshes = materialMeshPair.second;
					
					// Update appropriate uniforms per Material
					if (material) {

						if (material->Bind()) {

							// Update Standard Material Uniforms

							material->UpdateUniforms(*camera);

							// Update Specific Forward Plus Uniforms

							material->GetShader()->SetInt("u_TilesX", FP_Data.workGroupsX); // Number of tiles across the screen in the X axis
							material->GetShader()->SetiVec2("u_ScreenSize", glm::ivec2((int)Engine::Get().GetWindow().GetWidth(), (int)Engine::Get().GetWindow().GetHeight())); // Size of screen

							glActiveTexture(GL_TEXTURE4);
							material->GetShader()->SetInt("u_Depth", 4); // Depth texture map
							glBindTexture(GL_TEXTURE_2D, FP_Data.DepthMap_Texture);
						}
					}
					else {
						std::cout << "[L20] Invalid material encountered during rendering!" << std::endl;
						continue;
					}

					// Render all Meshes per material
					for (auto& meshAndTransform : meshes) {

						// IF singular mesh, draw individually
						if (meshAndTransform.second.size() == 1) {
							material->GetShader()->SetBool("u_UseInstanceData", false);

							Transform trans = meshAndTransform.second[0];
							material->GetShader()->SetMat4("model", trans.GetTransform());
							Renderer::DrawMesh(meshAndTransform.first);
						}
						// IF multiple of the same mesh, draw them using instancing
						else if (meshAndTransform.second.size() > 1) {

							material->GetShader()->SetBool("u_UseInstanceData", true);
							Renderer::DrawInstancedMesh(meshAndTransform.first, meshAndTransform.second);
						}
					}
				}

				// Clean Up Scene Render Pass
				for (int i = 0; i < 3; i++) {
					glActiveTexture(GL_TEXTURE0 + i);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				glActiveTexture(GL_TEXTURE0);
				glUseProgram(0);
			}
		}
	}

}
