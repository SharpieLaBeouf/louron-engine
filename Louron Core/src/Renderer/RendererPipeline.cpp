#include "RendererPipeline.h"

// Louron Core Headers
#include "Renderer.h"
#include "../Scene/Entity.h"
#include "../Scene/Components/Components.h"
#include "../Scene/Components/Light.h"
#include "../Scene/Components/Mesh.h"
#include "../Debug/Profiler.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <entt/entt.hpp>

namespace Louron {

	namespace SSBOLightStructs {

		struct alignas(16) PL_SSBO_DATA_LAYOUT {

			glm::vec4 position = { 0.0f, 0.0f, 0.0f, 1.0f };

			glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 specular = { 1.0f, 1.0f, 1.0f, 1.0f };

			struct LightProperties {
				GLfloat radius = 10.0f;
				GLfloat intensity = 1.0f;
				GLint active = true;
				GLint lastLight = false;
			} lightProperties;

			PL_SSBO_DATA_LAYOUT(const PointLightComponent& other) {

				position = other.position;
				ambient = other.ambient;
				diffuse = other.diffuse;
				specular = other.specular;

				lightProperties.radius = other.lightProperties.radius;
				lightProperties.intensity = other.lightProperties.intensity;
				lightProperties.active = other.lightProperties.active;
				lightProperties.lastLight = other.lightProperties.lastLight;
			}
		};

		struct alignas(16) SL_SSBO_DATA_LAYOUT {

			glm::vec4 position;
			glm::vec4 direction;

			glm::vec4 ambient;
			glm::vec4 diffuse;
			glm::vec4 specular;

			struct LightProperties {
				GLfloat range;
				GLfloat angle;
				GLfloat intensity;
				GLint active;
			} lightProperties;

			GLint lastLight;

			GLfloat m_Padding1;
			GLfloat m_Padding2;
			GLfloat m_Padding3;

			SL_SSBO_DATA_LAYOUT(const SpotLightComponent& other) {

				position = other.position;
				direction = other.direction;

				ambient = other.ambient;
				diffuse = other.diffuse;
				specular = other.specular;

				lightProperties.range = other.lightProperties.range;
				lightProperties.angle = other.lightProperties.angle;
				lightProperties.intensity = other.lightProperties.intensity;
				lightProperties.active = other.lightProperties.active;

				lastLight = other.lastLight;

				m_Padding1 = 0.0f;
				m_Padding2 = 0.0f;
				m_Padding3 = 0.0f;
			}

		};

		struct alignas(16) DL_SSBO_DATA_LAYOUT {

			glm::vec4 direction;

			glm::vec4 ambient;
			glm::vec4 diffuse;
			glm::vec4 specular;

			GLint lastLight;

			GLfloat m_Padding1;
			GLfloat m_Padding2;
			GLfloat m_Padding3;

			DL_SSBO_DATA_LAYOUT(const DirectionalLightComponent& other) {

				direction = other.GetDirection();

				ambient = other.ambient;
				diffuse = other.diffuse;
				specular = other.specular;

				lastLight = other.lastLight;

				m_Padding1 = 0.0f;
				m_Padding2 = 0.0f;
				m_Padding3 = 0.0f;

			}

		};

	}
		
#pragma region ForwardPipeline

	void RenderPipeline::OnUpdate() { 
	
	}

	void RenderPipeline::OnStartPipeline(std::shared_ptr<Louron::Scene> scene) { 
		m_Scene = scene; 
	}

	void RenderPipeline::OnStopPipeline() { 

		Renderer::CleanupInstanceData();
	}

	void RenderPipeline::UpdateActiveScene(std::shared_ptr<Louron::Scene> scene) {

		if (scene) {
			L_CORE_INFO("Changing Scene To: {0}", scene->GetConfig().Name);
			m_Scene = scene;
		}
		else {
			L_CORE_WARN("Cannot Change to Invalid Scene");
		}

	}

	void RenderPipeline::ConductRenderPass(Camera* camera) {


	}

#pragma endregion

#pragma region ForwardPlusPipeline

	/// <summary>
	/// This is the main loop for rendering logic
	/// in the Forward+ Pipeline.
	/// </summary>
	void ForwardPlusPipeline::OnUpdate() {
		
		if (!m_Scene) {
			L_CORE_ERROR("Invalid Scene! Please Use ForwardPlusPipeline::OnStartPipeline() Before Updating");
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			return;
		}

		Camera* camera = nullptr;
		if (Entity cameraEntity = m_Scene->GetPrimaryCameraEntity())
			camera = cameraEntity.GetComponent<CameraComponent>().Camera.get();

		if (camera) {

			glm::uvec2 frameSize = Engine::Get().GetWindow().GetSize();
			if (m_FrameSize != frameSize) {
				m_FrameSize = frameSize;

				UpdateComputeData();
				camera->UpdateProjMatrix();
			}

			UpdateSSBOData();

			ConductDepthPass(camera);
			ConductLightCull(camera);
			ConductRenderPass(camera);
		}
		else {
			L_CORE_WARN("No Primary Camera Found in Scene");
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}

	/// <summary>
	/// Set OpenGL state configuration required by renderer and FP_Data and Light SSBOs.
	/// </summary>
	void ForwardPlusPipeline::OnStartPipeline(std::shared_ptr<Louron::Scene> scene) {

		L_PROFILE_SCOPE("Forward Plus - Set Up Pipeline");

		glEnable(GL_DEPTH_TEST);

		m_Scene = scene;
		m_FrameSize = Engine::Get().GetWindow().GetSize();

		if (Entity cameraEntity = m_Scene->GetPrimaryCameraEntity())
			cameraEntity.GetComponent<CameraComponent>().Camera->UpdateProjMatrix();
		else {
			cameraEntity = m_Scene->CreateEntity("Main Camera");
			cameraEntity.AddComponent<CameraComponent>();
			cameraEntity.GetComponent<CameraComponent>().Camera = std::make_shared<Louron::Camera>(glm::vec3(0.0f, 10.0f, -10.0f));
		}

		// Calculate workgroups and generate SSBOs from screen size
		FP_Data.workGroupsX = (unsigned int)std::ceil((float)m_FrameSize.x / 16.0f);
		FP_Data.workGroupsY = (unsigned int)std::ceil((float)m_FrameSize.y / 16.0f);
		size_t numberOfTiles = static_cast<size_t>(FP_Data.workGroupsX * FP_Data.workGroupsY);

		// Setup Light Buffers

		glGenBuffers(1, &FP_Data.PL_Buffer);
		glGenBuffers(1, &FP_Data.PL_Indices_Buffer);

		glGenBuffers(1, &FP_Data.SL_Buffer);
		glGenBuffers(1, &FP_Data.SL_Indices_Buffer);

		glGenBuffers(1, &FP_Data.DL_Buffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_POINT_LIGHTS * sizeof(SSBOLightStructs::PL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_POINT_LIGHTS, nullptr, GL_STATIC_DRAW); 

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_SPOT_LIGHTS * sizeof(SSBOLightStructs::SL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_SPOT_LIGHTS, nullptr, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_DIRECTIONAL_LIGHTS * sizeof(SSBOLightStructs::DL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// Setup Depth Texture

		glGenFramebuffers(1, &FP_Data.DepthMap_FBO);
		glGenTextures(1, &FP_Data.DepthMap_Texture);

		glBindTexture(GL_TEXTURE_2D, FP_Data.DepthMap_Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_FrameSize.x, m_FrameSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

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
		
		glDeleteBuffers(1, &FP_Data.PL_Buffer);
		glDeleteBuffers(1, &FP_Data.PL_Indices_Buffer);

		glDeleteBuffers(1, &FP_Data.SL_Buffer);
		glDeleteBuffers(1, &FP_Data.SL_Indices_Buffer);

		glDeleteBuffers(1, &FP_Data.DL_Buffer);

		glDeleteFramebuffers(1, &FP_Data.DepthMap_FBO);
		glDeleteTextures(1, &FP_Data.DepthMap_Texture);

		Renderer::CleanupInstanceData();
	}

	/// <summary>
	/// This updates the Forward+ data for the applicable Compute and Vertex/Fragment shaders.
	/// </summary>
	void ForwardPlusPipeline::UpdateComputeData() {

		// Calculate Workgroups and Generate SSBOs from Screen Size

		FP_Data.workGroupsX = (unsigned int)std::ceil((float)m_FrameSize.x / 16.0f);
		FP_Data.workGroupsY = (unsigned int)std::ceil((float)m_FrameSize.y / 16.0f);
		size_t numberOfTiles = static_cast<size_t>(FP_Data.workGroupsX * FP_Data.workGroupsY);

		// Update Light Indice Buffers

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_POINT_LIGHTS, nullptr, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_SPOT_LIGHTS, nullptr, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// Update Depth Texture Size

		glBindTexture(GL_TEXTURE_2D, FP_Data.DepthMap_Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_FrameSize.x, m_FrameSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	/// <summary>
	/// Updates all Light Data in SSBOs
	/// </summary>
	void ForwardPlusPipeline::UpdateSSBOData() {

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
				std::vector<SSBOLightStructs::PL_SSBO_DATA_LAYOUT> pointLightVector;
				auto view = m_Scene->GetAllEntitiesWith<Transform, PointLightComponent>();
				
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
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, pointLightVector.size() * sizeof(SSBOLightStructs::PL_SSBO_DATA_LAYOUT), pointLightVector.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// Spot Lights
			{

				// Update Light Objects
				std::vector<SSBOLightStructs::SL_SSBO_DATA_LAYOUT> spotLightVector;
				auto view = m_Scene->GetAllEntitiesWith<Transform, SpotLightComponent>();

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
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, spotLightVector.size() * sizeof(SSBOLightStructs::SL_SSBO_DATA_LAYOUT), spotLightVector.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// Directional Lights
			{
				// Update Light Objects
				std::vector<SSBOLightStructs::DL_SSBO_DATA_LAYOUT> directionalLightVector;
				auto view = m_Scene->GetAllEntitiesWith<Transform, DirectionalLightComponent>();

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
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, directionalLightVector.size() * sizeof(SSBOLightStructs::DL_SSBO_DATA_LAYOUT), directionalLightVector.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
		}

	}

	/// <summary>
	/// Conducts a Depth Pass of the scene sorted front to back
	/// </summary>
	void ForwardPlusPipeline::ConductDepthPass(Camera* camera) {

		L_PROFILE_SCOPE("Forward Plus - Pre Depth Pass");

		// Call Renderer for all Meshes
		{
			const auto& view = m_Scene->GetAllEntitiesWith<Transform, MeshRenderer, MeshFilter>();

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
				else {
					L_CORE_ERROR("FP Depth Shader Not Found");
				}
			}
		}
	}

	/// <summary>
	/// Conducts main tiled rendering algorithm, split screen into tiles
	/// and determine which lights impact each tile frustum.
	/// </summary>
	void ForwardPlusPipeline::ConductLightCull(Camera* camera) {

		L_PROFILE_SCOPE("Forward Plus - Light Cull");
		// Conduct Light Cull
		std::shared_ptr<Shader> lightCull = Engine::Get().GetShaderLibrary().GetShader("FP_Light_Culling");
		if (lightCull) {
			lightCull->Bind();

			lightCull->SetMat4("u_View", camera->GetViewMatrix());
			lightCull->SetMat4("u_Proj", camera->GetProjMatrix());
			lightCull->SetiVec2("u_ScreenSize", (glm::ivec2)m_FrameSize);
			glActiveTexture(GL_TEXTURE4);
			lightCull->SetInt("u_Depth", 4);
			glBindTexture(GL_TEXTURE_2D, FP_Data.DepthMap_Texture);

			glDispatchCompute(FP_Data.workGroupsX, FP_Data.workGroupsY, 1);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else {
			L_CORE_ERROR("FP Light Cull Compute Shader Not Found");
		}
	}

	/// <summary>
	/// Conduct final colour pass. Meshes are sorted by material, then
	/// sorted by mesh per material, then drawn individually or 
	/// drawn as instances if the meshes are identical using the same
	/// material.
	/// </summary>
	void ForwardPlusPipeline::ConductRenderPass(Camera* camera) {

		L_PROFILE_SCOPE("Forward Plus - Colour Render");

		//// Render All MeshComponents in Scene
		//glBindFramebuffer(GL_FRAMEBUFFER, Engine::Get().GetRenderFBO().FBO);
		//glEnable(GL_DEPTH_TEST);
		Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Gather all entities within scene with appropriate components
		auto view = m_Scene->GetAllEntitiesWith<Transform, MeshRenderer, MeshFilter>();
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
							L_CORE_WARN("Mesh Has Invalid Material");
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
						material->GetShader()->SetiVec2("u_ScreenSize", (glm::ivec2)m_FrameSize); // Size of screen

						glActiveTexture(GL_TEXTURE4);
						material->GetShader()->SetInt("u_Depth", 4); // Depth texture map
						glBindTexture(GL_TEXTURE_2D, FP_Data.DepthMap_Texture);
					}
				}
				else {
					L_CORE_WARN("Invalid Material Encountered During Rendering");
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
		}

		auto skyboxView = m_Scene->GetAllEntitiesWith<CameraComponent, SkyboxComponent>();
		if (skyboxView.begin() != skyboxView.end()) {

			for (const auto& entity : skyboxView) {
				auto [scene_camera, skybox] = skyboxView.get<CameraComponent, SkyboxComponent>(entity);

				if (scene_camera.ClearFlags == CameraClearFlags::SKYBOX) {

					if (skybox.Material->Bind()) {
						glDepthFunc(GL_LEQUAL);

						skybox.Material->UpdateUniforms(*camera);
						Renderer::DrawSkybox(skybox);

						skybox.UnBind();
						glDepthFunc(GL_LESS);
					}
				}
				else {
					L_CORE_WARN("Camera Has Skybox But Clear Flags Not Set to Skybox!");
				}
			}
		}

		// Clean Up Scene Render Pass
		for (int i = 0; i < 4; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_DEPTH_TEST);
		//Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT);
	}

#pragma endregion

#pragma region DeferredPipeline

	void DeferredPipeline::OnUpdate() {

	}

	void DeferredPipeline::OnStartPipeline(std::shared_ptr<Louron::Scene> scene) {

	}

	void DeferredPipeline::OnStopPipeline() {

		Renderer::CleanupInstanceData();
	}

#pragma endregion

}
