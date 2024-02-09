#include "RendererPipeline.h"

#include "Renderer.h"

#include "../Scene/Entity.h"
#include <entt/entt.hpp>

namespace Louron {
		
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

		UpdateSSBOData(scene);

		ConductDepthPass(scene, camera);
		ConductLightCull(camera); // WIP for Spot Lights
		ConductRenderPass(scene, camera);
	}

	/// <summary>
	/// Set OpenGL state configuration required by renderer and FP_Data and Light SSBOs.
	/// </summary>
	void ForwardPlusPipeline::OnStartPipeline() {

		L_PROFILE_SCOPE("Forward Plus - Set Up Pipeline");

		glEnable(GL_DEPTH_TEST);

		// Set screen to non-resizeable
		glfwSetWindowAttrib((GLFWwindow*)Engine::Get().GetWindow().GetNativeWindow(), GLFW_RESIZABLE, GL_FALSE);

		// Calculate workgroups and generate SSBOs from screen size
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

	/// <summary>
	/// Reset OpenGL state configuration required by renderer and clean FP_Data and Light SSBOs.
	/// </summary>
	void ForwardPlusPipeline::OnStopPipeline() {

		L_PROFILE_SCOPE("Forward Plus - Clean Up Pipeline");

		glDisable(GL_DEPTH_TEST);

		glfwSetWindowAttrib((GLFWwindow*)Engine::Get().GetWindow().GetNativeWindow(), GLFW_RESIZABLE, GL_TRUE);

		glDeleteBuffers(1, &FP_Data.PL_Buffer);
		glDeleteBuffers(1, &FP_Data.PL_Indices_Buffer);
		glDeleteBuffers(1, &FP_Data.SL_Buffer);
		glDeleteBuffers(1, &FP_Data.SL_Indices_Buffer);
		glDeleteBuffers(1, &FP_Data.DL_Buffer);

		glDeleteFramebuffers(1, &FP_Data.DepthMap_FBO);
		glDeleteTextures(1, &FP_Data.DepthMap_Texture);
	}

	/// <summary>
	/// Updates all Light Data in SSBOs
	/// </summary>
	void ForwardPlusPipeline::UpdateSSBOData(Scene* scene) {

		L_PROFILE_SCOPE("Forward Plus - Update SSBO Data");

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, FP_Data.PL_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, FP_Data.PL_Indices_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, FP_Data.SL_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, FP_Data.SL_Indices_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, FP_Data.DL_Buffer);

		// Lights
		{
			// Point Lights
			{
				// Update Light Objects
				std::vector<PointLightComponent> pointLightVector;
				auto view = scene->GetRegistry()->view<Transform, PointLightComponent>();
				
				// Add lights to vector that are contained within the scene up to a maximum of 1024
				int i = 0;
				for (auto entity : view) {
					
					if (i >= MAX_POINT_LIGHTS)
						break;
					else
						i++;

					auto [transform, point_light] = view.get<Transform, PointLightComponent>(entity);

					point_light.position = glm::vec4(transform.GetPosition(), 1.0f);
					point_light.lightProperties.lastLight = false;

					pointLightVector.push_back(point_light);
				}

				// Create Buffer Light at End of Vector if not full
				if (i < MAX_POINT_LIGHTS - 1) {
					PointLightComponent tempLastLight;
					tempLastLight.lightProperties.lastLight = true;
					pointLightVector.push_back(tempLastLight);
				}

				// Update SSBO data with light data
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Buffer);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, pointLightVector.size() * sizeof(PointLightComponent), pointLightVector.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// Spot Lights
			{

				// Update Light Objects
				std::vector<SpotLightComponent> spotLightVector;
				auto view = scene->GetRegistry()->view<Transform, SpotLightComponent>();

				// Add lights to vector that are contained within the scene up to a maximum of 1024
				int i = 0;
				for (auto entity : view) {

					if (i >= MAX_SPOT_LIGHTS)
						break;
					else
						i++;

					auto [transform, spot_light] = view.get<Transform, SpotLightComponent>(entity);

					spot_light.position = glm::vec4(transform.GetPosition(), 1.0f);
					spot_light.lastLight = false;

					spotLightVector.push_back(spot_light);
				}

				// Create Buffer Light at End of Vector if not full
				if (i < MAX_SPOT_LIGHTS - 1) {
					SpotLightComponent tempLastLight;
					tempLastLight.lastLight = true;
					spotLightVector.push_back(tempLastLight);
				}

				// Update SSBO data with light data
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Buffer);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, spotLightVector.size() * sizeof(SpotLightComponent), spotLightVector.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// Directional Lights
			{
				// Update Light Objects
				std::vector<DirectionalLightComponent> directionalLightVector;
				auto view = scene->GetRegistry()->view<Transform, DirectionalLightComponent>();

				// Add lights to vector that are contained within the scene up to a maximum of 1024
				int i = 0;
				for (auto entity : view) {

					if (i >= MAX_DIRECTIONAL_LIGHTS)
						break;
					else
						i++;

					auto [transform, directional_light] = view.get<Transform, DirectionalLightComponent>(entity);

					directional_light.direction = glm::normalize(
						glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) *
						glm::quat(glm::radians(transform.GetRotation())));

					directional_light.lastLight = false;

					directionalLightVector.push_back(directional_light);
				}

				// Create Buffer Light at End of Vector if not full
				if (i < MAX_DIRECTIONAL_LIGHTS - 1) {
					DirectionalLightComponent tempLastLight;
					tempLastLight.lastLight = true;
					directionalLightVector.push_back(tempLastLight);
				}

				// Update SSBO data with light data
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Buffer);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, directionalLightVector.size() * sizeof(DirectionalLightComponent), directionalLightVector.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
		}

	}

	/// <summary>
	/// Conducts a Depth Pass of the scene sorted front to back
	/// </summary>
	void ForwardPlusPipeline::ConductDepthPass(Scene* scene, Camera* camera) {

		L_PROFILE_SCOPE("Forward Plus - Pre Depth Pass");

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

					L_CORE_ASSERT(shader, "FP Depth Shader Not Found!");
					if (shader)
					{
						glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.DepthMap_FBO);
						Renderer::ClearBuffer(GL_DEPTH_BUFFER_BIT);

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

	/// <summary>
	/// Conducts main tiled rendering algorithm, split screen into tiles
	/// and determine which lights impact each tile frustum.
	/// </summary>
	/// <param name="camera"></param>
	void ForwardPlusPipeline::ConductLightCull(Camera* camera) {

		L_PROFILE_SCOPE("Forward Plus - Light Cull");
		// Conduct Light Cull
		std::shared_ptr<Shader> lightCull = Engine::Get().GetShaderLibrary().GetShader("FP_Light_Culling");

		L_CORE_ASSERT(lightCull, "FP Light Cull Compute Shader Not Found!");
		if (lightCull) {
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
	}

	/// <summary>
	/// Conduct final colour pass. Meshes are sorted by material, then
	/// sorted by mesh per material, then drawn individually or 
	/// drawn as instances if the meshes are identical using the same
	/// material.
	/// </summary>
	void ForwardPlusPipeline::ConductRenderPass(Scene* scene, Camera* camera) {

		L_PROFILE_SCOPE("Forward Plus - Colour Render");

		// Render All MeshComponents in Scene
		if (camera) {

			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
							material->GetShader()->SetMat4("u_VertexIn.Model", trans.GetTransform());
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
