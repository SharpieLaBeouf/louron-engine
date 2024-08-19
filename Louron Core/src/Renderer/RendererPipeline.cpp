#include "RendererPipeline.h"

// Louron Core Headers
#include "Renderer.h"

#include "../Project/Project.h"

#include "../Scene/Entity.h"
#include "../Scene/Components/Components.h"
#include "../Scene/Components/Light.h"
#include "../Scene/Components/Mesh.h"

#include "../Scene/OctreeBounds.h"

#include "../Debug/Profiler.h"

#include "../Core/Time.h"

#include "../OpenGL/Framebuffer.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <entt/entt.hpp>

namespace Louron {

	namespace SSBOLightStructs {

		struct alignas(16) PL_SSBO_DATA_LAYOUT {

			glm::vec4 position = { 0.0f, 0.0f, 0.0f, 1.0f };

			glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

			GLfloat radius = 10.0f;
			GLfloat intensity = 1.0f;
			GLint active = true;
			GLint lastLight = false;

			GLuint shadowCastingType = 0;

			GLfloat m_Padding1 = 0.0f;
			GLfloat m_Padding2 = 0.0f;
			GLfloat m_Padding3 = 0.0f;

			PL_SSBO_DATA_LAYOUT() = default;

			PL_SSBO_DATA_LAYOUT(const PointLightComponent& point_light, Transform& transform) {

				position = { transform.GetGlobalPosition(), 1.0f };
				shadowCastingType = static_cast<GLuint>(point_light.ShadowFlag);

				colour = point_light.Colour;

				radius = point_light.Radius;
				intensity = point_light.Intensity;
				active = point_light.Active ? 1 : 0;

			}

			PL_SSBO_DATA_LAYOUT(const PointLightComponent& point_light) {

				colour = point_light.Colour;

				radius = point_light.Radius;
				intensity = point_light.Intensity;
				active = point_light.Active ? 1 : 0;

			}
			
		};

		struct alignas(16) SL_SSBO_DATA_LAYOUT {

			glm::vec4 position = { 0.0f, 0.0f, 0.0f, 0.0f };
			glm::vec4 direction = { 0.0f, 0.0f, -1.0f, 0.0f };

			glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

			GLfloat range = 10.0f;
			GLfloat angle = 45.0f;
			GLfloat intensity = 1.0f;
			
			GLint active = true;
			GLint lastLight = false;

			GLuint shadowCastingType = 0;

			GLfloat m_Padding1 = 0.0f;
			GLfloat m_Padding2 = 0.0f;

			SL_SSBO_DATA_LAYOUT() = default;

			SL_SSBO_DATA_LAYOUT(const SpotLightComponent& spot_light, Transform& transform) {

				position  = { transform.GetGlobalPosition(), 1.0f };
				direction = { transform.GetGlobalForwardDirection(), 1.0f };
				shadowCastingType = static_cast<GLuint>(spot_light.ShadowFlag);

				colour = spot_light.Colour;

				range = spot_light.Range;
				angle = spot_light.Angle;
				intensity = spot_light.Intensity;
				active = spot_light.Active ? 1 : 0;

			}

			SL_SSBO_DATA_LAYOUT(const SpotLightComponent& spot_light) {

				colour = spot_light.Colour;

				range = spot_light.Range;
				angle = spot_light.Angle;
				intensity = spot_light.Intensity;
				active = spot_light.Active ? 1 : 0;
			}

		};

		struct alignas(16) DL_SSBO_DATA_LAYOUT {

			glm::vec4 direction = { 0.0f, 0.0f, -1.0f, 0.0f };

			glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

			GLint active = true;
			GLfloat intensity = 1.0f;
			GLint lastLight = false;
			GLuint shadowCastingType = 0;

			DL_SSBO_DATA_LAYOUT() = default;

			DL_SSBO_DATA_LAYOUT(const DirectionalLightComponent& directional_light, Transform& transform) {
				
				direction = { transform.GetGlobalForwardDirection(), 1.0f };
				shadowCastingType = static_cast<GLuint>(directional_light.ShadowFlag);

				active = directional_light.Active ? 1 : 0;
				colour = directional_light.Colour;
				intensity = directional_light.Intensity;
			}

			DL_SSBO_DATA_LAYOUT(const DirectionalLightComponent& directional_light) {

				active = directional_light.Active ? 1 : 0;
				colour = directional_light.Colour;
				intensity = directional_light.Intensity;
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

		Renderer::CleanupRenderData();
	}

	void RenderPipeline::OnViewportResize() {

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

		L_PROFILE_SCOPE("Forward Plus");

		if (!m_Scene) {
			L_CORE_ERROR("Invalid Scene! Please Use ForwardPlusPipeline::OnStartPipeline() Before Updating");
			Renderer::ClearColour({ 0.0f, 0.0f, 0.0f, 1.0f });
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			return;
		}

		Camera* camera = nullptr;
		Entity camera_entity = m_Scene->GetPrimaryCameraEntity();
		if (camera_entity) camera = camera_entity.GetComponent<CameraComponent>().CameraInstance.get();

		if (camera) {

			Renderer::ClearRenderStats();

			// 1. SCENE FRUSTUM CULLING
			{
				L_PROFILE_SCOPE("Forward Plus - Frustum Culling");

				// Recalculate Camera Frustum
				FP_Data.Camera_Frustum.RecalculateFrustum(camera->GetProjMatrix() * camera->GetViewMatrix());

				// Wait for Octree Update Thread
				if (FP_Data.OctreeUpdateThread.joinable()) {
					L_PROFILE_SCOPE("Forward Plus - Octree Thread Wait");
					FP_Data.OctreeUpdateThread.join();
				}

				// Gather All Point and Spot Lights Visible in Camera Frustum
				FP_Data.PLEntities.clear();
				FP_Data.SLEntities.clear();
				ConductLightFrustumCull();

				// Gather All Meshes Visible in Camera Frustum
				ConductRenderableFrustumCull();

				// Dispatch Thread
				FP_Data.OctreeUpdateThread = std::thread([&]() -> void {

					L_PROFILE_SCOPE("Forward Plus - Octree Update");

					if (auto oct_ref = m_Scene->GetOctree().lock(); oct_ref) {

						std::unique_lock lock(oct_ref->GetOctreeMutex());
						
						// Check which objects are no longer in the scene
						const auto& octree_data_sources = oct_ref->GetAllOctreeDataSources();
						std::vector<Entity> remove_entities{};
						for (const auto& data_source : octree_data_sources) {

							if (m_Scene->HasEntity(data_source->Data)) {
								Entity entity = data_source->Data;
								if (!entity.HasComponent<AssetMeshFilter>() || !entity.HasComponent<AssetMeshRenderer>()) {
									remove_entities.push_back(entity);
								}
							}
						}

						for (const auto& entity : remove_entities) 
							oct_ref->Remove(entity);

						oct_ref->TryShrinkOctree();
						auto mesh_view = m_Scene->GetAllEntitiesWith<AssetMeshRenderer, AssetMeshFilter>();
						for (const auto& entity_handle : mesh_view) {

							auto& component = mesh_view.get<AssetMeshFilter>(entity_handle);

							if (component.MeshFilterAssetHandle == NULL_UUID)
								continue;

							if (component.AABBNeedsUpdate) {
								component.UpdateTransformedAABB();
							}

							if (component.OctreeNeedsUpdate) {
								if (oct_ref->Update({ entity_handle, m_Scene.get() }, component.TransformedAABB))
									component.OctreeNeedsUpdate = false;
								else {
									L_CORE_WARN("Could Not Be Inserted Into Octree - Deleting Entity: {0}", component.GetEntity().GetName());
									m_Scene->DestroyEntity({ entity_handle, m_Scene.get() });
								}
							}
						}
					}
				});
			}

			// 2. RENDERING
			{
				L_PROFILE_SCOPE("Forward Plus - Rendering");
				UpdateSSBOData();

				// Bind FBO and clear color and depth buffers for the new frame
				m_Scene->GetSceneFrameBuffer()->Bind();
				Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
				glPolygonMode(GL_FRONT_AND_BACK, FP_Data.ShowWireframe ? GL_LINE : GL_FILL);

				ConductDepthPass(camera);
				ConductTiledBasedLightCull(camera);

				glDrawBuffer(GL_COLOR_ATTACHMENT0);

				Renderer::ClearColour(camera_entity.GetComponent<CameraComponent>().ClearColour);
				Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT);

				ConductRenderPass(camera);

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				// Unbind FBO to render to the screen
				m_Scene->GetSceneFrameBuffer()->Unbind();

				// Clear the standard OpenGL back buffer
				Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				if (m_Scene->GetSceneFrameBuffer()->GetConfig().RenderToScreen)
					RenderFBOQuad();
			}
		}
		else {
			L_CORE_WARN("No Primary Camera Found in Scene");
			m_Scene->GetSceneFrameBuffer()->Bind();
			Renderer::ClearColour({ 0.0f, 0.0f, 0.0f, 1.0f });
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_Scene->GetSceneFrameBuffer()->Unbind();
		}
	}

	/// <summary>
	/// Set OpenGL state configuration required by renderer and FP_Data and Light SSBOs.
	/// </summary>
	void ForwardPlusPipeline::OnStartPipeline(std::shared_ptr<Louron::Scene> scene) {

		// We want to benefit from the ConductDepthPass depth values in the depth buffer for the 
		// ConductRenderPass, so we use LEQUAL  to ensure that fragments are not discarded because 
		// the depth values from the depth pass will be EQUAL to the depth values in the render pass
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL); 

		m_Scene = scene;

		if (!m_Scene->GetSceneFrameBuffer()) {
			L_CORE_ERROR("Framebuffer is Invalid.");
			OnStopPipeline();
			return;
		}

		if (!m_Scene->GetSceneFrameBuffer()->IsValid()) {
			L_CORE_ERROR("Framebuffer Setup Not Complete.");
			OnStopPipeline();
			return;
		}

		if (Entity cameraEntity = m_Scene->GetPrimaryCameraEntity())
			cameraEntity.GetComponent<CameraComponent>().CameraInstance->UpdateProjMatrix();
		else {
			cameraEntity = m_Scene->CreateEntity("Main Camera");
			cameraEntity.AddComponent<CameraComponent>();
			cameraEntity.GetComponent<CameraComponent>().CameraInstance = std::make_shared<Louron::Camera>(glm::vec3(0.0f, 10.0f, -10.0f));
		}

		// Calculate workgroups and generate SSBOs from screen size
		FP_Data.workGroupsX = (unsigned int)std::ceil((float)m_Scene->GetSceneFrameBuffer()->GetConfig().Width / 16.0f);
		FP_Data.workGroupsY = (unsigned int)std::ceil((float)m_Scene->GetSceneFrameBuffer()->GetConfig().Height / 16.0f);
		size_t numberOfTiles = static_cast<size_t>(FP_Data.workGroupsX * FP_Data.workGroupsY);

		// Setup Light Buffers

		glGenBuffers(1, &FP_Data.DL_Buffer);

		glGenBuffers(1, &FP_Data.PL_Buffer);
		glGenBuffers(1, &FP_Data.PL_Indices_Buffer);

		glGenBuffers(1, &FP_Data.SL_Buffer);
		glGenBuffers(1, &FP_Data.SL_Indices_Buffer);

		// Directional Lights
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_DIRECTIONAL_LIGHTS * sizeof(SSBOLightStructs::DL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW);

		// Point Lights
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_POINT_LIGHTS * sizeof(SSBOLightStructs::PL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW); // All light data

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_POINT_LIGHTS, nullptr, GL_STATIC_DRAW); // List of visible lights per tile

		// Spot Lights
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_SPOT_LIGHTS * sizeof(SSBOLightStructs::SL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW); // All light data

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_SPOT_LIGHTS, nullptr, GL_STATIC_DRAW); // List of visible lights per tile

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		if(!FP_Data.Screen_Quad_VAO)
		{
			FP_Data.Screen_Quad_VAO = std::make_unique<VertexArray>();

			std::vector<Vertex> vertices{
				Vertex(glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec3(), glm::vec2( 0.0f, 1.0f )),
				Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(), glm::vec2( 0.0f, 0.0f )),
				Vertex(glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec3(), glm::vec2( 1.0f, 0.0f )),
				Vertex(glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec3(), glm::vec2( 1.0f, 1.0f )),
			};

			std::vector<GLuint> indices{
				0, 1, 2,
				0, 2, 3
			};

			BufferLayout layout = {
				{ ShaderDataType::Float3, "aPos" },
				{ ShaderDataType::Float3, "aNormal" },
				{ ShaderDataType::Float2, "aTexCoord" }
			};

			VertexBuffer* vbo = new VertexBuffer(vertices, (GLuint)vertices.size());
			vbo->SetLayout(layout);

			IndexBuffer* ebo = new IndexBuffer(indices, (GLuint)indices.size());

			FP_Data.Screen_Quad_VAO->AddVertexBuffer(vbo);
			FP_Data.Screen_Quad_VAO->SetIndexBuffer(ebo);

		}

		FP_Data.RenderableEntities.reserve(1024);
		FP_Data.OctreeEntitiesInCamera.reserve(1024);

		FP_Data.PLEntities.reserve(MAX_SPOT_LIGHTS);
		FP_Data.SLEntities.reserve(MAX_SPOT_LIGHTS);
	}

	/// <summary>
	/// Reset OpenGL state configuration required by renderer and clean FP_Data and Light SSBOs.
	/// </summary>
	void ForwardPlusPipeline::OnStopPipeline() {
				
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		
		glDeleteBuffers(1, &FP_Data.PL_Buffer);
		glDeleteBuffers(1, &FP_Data.PL_Indices_Buffer);

		glDeleteBuffers(1, &FP_Data.SL_Buffer);
		glDeleteBuffers(1, &FP_Data.SL_Indices_Buffer);

		glDeleteBuffers(1, &FP_Data.DL_Buffer);

		if (FP_Data.Screen_Quad_VAO) {
			FP_Data.Screen_Quad_VAO.reset();
			FP_Data.Screen_Quad_VAO = nullptr;
		}

		Renderer::CleanupRenderData();
	}

	/// <summary>
	/// This will update required FP_Data for any viewport resizing.
	/// </summary>
	void ForwardPlusPipeline::OnViewportResize() {

		if (!m_Scene) {
			L_CORE_ERROR("Invalid Scene! Please Use ForwardPlusPipeline::OnStartPipeline() Before Updating Viewport");
			Renderer::ClearColour({ 1.0f, 1.0f, 1.0f, 1.0f });
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			return;
		}

		// Update Projection Matrix of All Scene Cameras
		auto camera_view = m_Scene->GetAllEntitiesWith<CameraComponent>();
		for (const auto& entity : camera_view) {
			if (auto cam_ref = camera_view.get<CameraComponent>(entity).CameraInstance; cam_ref) {
				cam_ref->UpdateProjMatrix(glm::ivec2(m_Scene->GetSceneFrameBuffer()->GetConfig().Width, m_Scene->GetSceneFrameBuffer()->GetConfig().Height));
			}
		}

		UpdateComputeData();
	}

	/// <summary>
	/// This updates the Forward+ data for the applicable Compute and Vertex/Fragment shaders.
	/// </summary>
	void ForwardPlusPipeline::UpdateComputeData() {

		// Calculate Workgroups and Generate SSBOs from Screen Size
		FP_Data.workGroupsX = (unsigned int)std::ceil((float)m_Scene->GetSceneFrameBuffer()->GetConfig().Width / 16.0f);
		FP_Data.workGroupsY = (unsigned int)std::ceil((float)m_Scene->GetSceneFrameBuffer()->GetConfig().Height / 16.0f);
		size_t numberOfTiles = static_cast<size_t>(FP_Data.workGroupsX * FP_Data.workGroupsY);

		// Update Light Indice Buffers

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_POINT_LIGHTS, nullptr, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_SPOT_LIGHTS, nullptr, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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
				static std::vector<SSBOLightStructs::PL_SSBO_DATA_LAYOUT> s_PointLightVector(MAX_POINT_LIGHTS);
				s_PointLightVector.clear();
				
				// Add lights to vector that are contained within the scene up to a maximum of 1024
				for (auto& entity : FP_Data.PLEntities) {
					
					if (s_PointLightVector.size() >= MAX_POINT_LIGHTS)
						break;

					auto& point_light = entity.GetComponent<PointLightComponent>();

					if(point_light.Active)
						s_PointLightVector.push_back({ point_light, entity.GetComponent<Transform>() });
				}

				// Create Buffer Light at End of Vector if not full
				if (s_PointLightVector.size() < MAX_POINT_LIGHTS) {
					PointLightComponent tempLastLight;
					s_PointLightVector.push_back(tempLastLight);
					s_PointLightVector.back().lastLight = true;
				}

				// Update SSBO data with light data
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Buffer);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, s_PointLightVector.size() * sizeof(SSBOLightStructs::PL_SSBO_DATA_LAYOUT), s_PointLightVector.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// Spot Lights
			{

				// Update Light Objects
				static std::vector<SSBOLightStructs::SL_SSBO_DATA_LAYOUT> s_SpotLightVector(MAX_SPOT_LIGHTS);
				s_SpotLightVector.clear();

				// Add lights to vector that are contained within the scene up to a maximum of 1024
				for (auto& entity : FP_Data.SLEntities) {

					if (s_SpotLightVector.size() >= MAX_POINT_LIGHTS)
						break;

					auto& spot_light = entity.GetComponent<SpotLightComponent>();
					if (spot_light.Active)
						s_SpotLightVector.push_back({ spot_light, entity.GetComponent<Transform>() });
				}

				// Create Buffer Light at End of Vector if not full
				if (s_SpotLightVector.size() < MAX_SPOT_LIGHTS) {
					SpotLightComponent tempLastLight;
					s_SpotLightVector.push_back(tempLastLight);
					s_SpotLightVector.back().lastLight = true;
				}

				// Update SSBO data with light data
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Buffer);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, s_SpotLightVector.size() * sizeof(SSBOLightStructs::SL_SSBO_DATA_LAYOUT), s_SpotLightVector.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// Directional Lights
			{
				// Update Light Objects
				auto view = m_Scene->GetAllEntitiesWith<Transform, DirectionalLightComponent>();

				static std::vector<SSBOLightStructs::DL_SSBO_DATA_LAYOUT> s_DirectionalLightVector(MAX_DIRECTIONAL_LIGHTS);
				s_DirectionalLightVector.clear();

				// Add lights to vector that are contained within the scene up to a maximum of 10
				for (auto entity : view) {

					if (s_DirectionalLightVector.size() >= MAX_DIRECTIONAL_LIGHTS)
						break;

					auto [transform, directional_light] = view.get<Transform, DirectionalLightComponent>(entity);

					if (directional_light.Active)
						s_DirectionalLightVector.push_back({ directional_light, transform });
				}

				// Create Buffer Light at End of Vector if not full
				if (s_DirectionalLightVector.size() < MAX_DIRECTIONAL_LIGHTS) {
					DirectionalLightComponent tempLastLight;
					s_DirectionalLightVector.push_back(tempLastLight);
					s_DirectionalLightVector.back().lastLight = true;
				}

				// Update SSBO data with light data
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Buffer);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, s_DirectionalLightVector.size() * sizeof(SSBOLightStructs::DL_SSBO_DATA_LAYOUT), s_DirectionalLightVector.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
		}

	}

	/// <summary>
	/// This will cull all lights outside camera frustum and update
	/// the visible PL and SL light vectors in FP_Data
	/// </summary>
	void ForwardPlusPipeline::ConductLightFrustumCull() {

		L_PROFILE_SCOPE("Forward Plus - Frustum Culling (LIGHTS)");

		auto pl_view = m_Scene->GetAllEntitiesWith<PointLightComponent>();
		for (const auto& entity_handle : pl_view) {

			if (!pl_view.get<PointLightComponent>(entity_handle).Active)
				continue;

			Entity entity = { entity_handle, m_Scene.get() };
			if (IsSphereInsideFrustum({ entity.GetComponent<Transform>().GetGlobalPosition(), entity.GetComponent<PointLightComponent>().Radius }, FP_Data.Camera_Frustum)) {
				FP_Data.PLEntities.push_back(entity);
			}

		}

		auto sl_view = m_Scene->GetAllEntitiesWith<SpotLightComponent>();
		for (const auto& entity_handle : sl_view) {

			if (!sl_view.get<SpotLightComponent>(entity_handle).Active)
				continue;

			Entity entity = { entity_handle, m_Scene.get() };

			auto& transform = entity.GetComponent<Transform>();
			auto& light = entity.GetComponent<SpotLightComponent>();

			float half_angle = glm::radians(light.Angle * 0.5f);
			float cosPenumbra = cos(half_angle);

			Bounds_Sphere sphere;
			if (half_angle > glm::pi<float>() / 4.0f) {
				sphere.BoundsCentre = transform.GetGlobalPosition() + cosPenumbra * light.Range * transform.GetGlobalForwardDirection();
				sphere.BoundsRadius = sin(half_angle) * light.Range;
			}
			else
			{
				sphere.BoundsCentre = transform.GetGlobalPosition() + light.Range / (2.0f * cosPenumbra) * transform.GetGlobalForwardDirection();
				sphere.BoundsRadius = light.Range / (2.0f * cosPenumbra);
			}

			if (IsSphereInsideFrustum(sphere, FP_Data.Camera_Frustum)) {
				FP_Data.SLEntities.push_back(entity);
			}

		}
	}

	/// <summary>
	/// This will cull all scene geometry outside camera frustum and 
	/// update the renderables vector in FP_Data.
	/// </summary>
	void ForwardPlusPipeline::ConductRenderableFrustumCull() {

		L_PROFILE_SCOPE("Forward Plus - Frustum Culling Octree Query");
		if (auto oct_ref = m_Scene->GetOctree().lock(); oct_ref) {

			const auto& query_vec = oct_ref->Query(FP_Data.Camera_Frustum);

			if (query_vec.size() > FP_Data.OctreeEntitiesInCamera.capacity())
				FP_Data.OctreeEntitiesInCamera.reserve(FP_Data.OctreeEntitiesInCamera.capacity() * 2);
			
			FP_Data.OctreeEntitiesInCamera.insert(FP_Data.OctreeEntitiesInCamera.begin(), query_vec.begin(), query_vec.end());
		}

		// Transfer Data Over because we don't want 
		// to hold the shared_ptr's to OctreeData
		FP_Data.RenderableEntities.clear();

		if (FP_Data.OctreeEntitiesInCamera.size() > FP_Data.RenderableEntities.capacity())
			FP_Data.RenderableEntities.reserve(FP_Data.RenderableEntities.capacity() * 2);

		for (const auto& data : FP_Data.OctreeEntitiesInCamera) 
			FP_Data.RenderableEntities.push_back(data->Data);

		FP_Data.OctreeEntitiesInCamera.clear();
	}

	/// <summary>
	/// Conducts a Depth Pass of the scene sorted front to back.
	/// 
	/// IF VSYNC IS ON, the profiling from this section will wait
	/// for some reason for the specified time to ensure it's only
	/// running at the Hz rate of the monitor. 
	/// 
	/// </summary>
	void ForwardPlusPipeline::ConductDepthPass(Camera* camera) {

		L_PROFILE_SCOPE("Forward Plus - Depth Pass");

		m_Scene->GetSceneFrameBuffer()->ClearEntityPixelData(NULL_UUID);

		// Key #1 - Distance - Value UUID of Entities that have submeshes
		std::vector<std::tuple<float, UUID, std::shared_ptr<SubMesh>>> sorted_entities;

		const glm::vec3& cam_position = camera->GetGlobalPosition();

		if (!FP_Data.RenderableEntities.empty()) {

			for (auto& entity : FP_Data.RenderableEntities) {

				auto asset_mesh = Project::GetStaticEditorAssetManager()->GetAsset<AssetMesh>(entity.GetComponent<AssetMeshFilter>().MeshFilterAssetHandle);

				if (!asset_mesh)
					continue;

				const glm::vec3& objectPosition = entity.GetComponent<Transform>().GetGlobalPosition();
				float distance = glm::length(objectPosition - cam_position);

				for (std::shared_ptr<SubMesh> sub_mesh : asset_mesh->SubMeshes) {
					sorted_entities.emplace_back(distance, entity.GetUUID(), sub_mesh);
				}

			}
						
			// Lambda function compares the distances of two entities (a and b) and orders them in a way that ensures front-to-back sorting
			std::sort(sorted_entities.begin(), sorted_entities.end(), [](const auto& a, const auto& b) {
				return std::get<0>(a) < std::get<0>(b);
			});
			
			if (std::shared_ptr<Shader> shader = Engine::Get().GetShaderLibrary().GetShader("FP_Depth"); shader) {

				shader->Bind();
				shader->SetMat4("u_Proj", camera->GetProjMatrix());
				shader->SetMat4("u_View", camera->GetViewMatrix());

				for (auto& [distance, entity_uuid, sub_mesh] : sorted_entities) {
					shader->SetMat4("u_Model", m_Scene->FindEntityByUUID(entity_uuid).GetComponent<Transform>().GetGlobalTransform());
					shader->SetUInt("u_EntityID", entity_uuid);

					Renderer::DrawSubMesh(sub_mesh);
				}

				shader->UnBind();
			}
			else {
				L_CORE_ERROR("FP Depth Shader Not Found");
			}
		}

		glFlush();
	}

	/// <summary>
	/// Conducts main tiled rendering algorithm, split screen into tiles
	/// and determine which lights impact each tile frustum.
	/// </summary>
	void ForwardPlusPipeline::ConductTiledBasedLightCull(Camera* camera) {
		L_PROFILE_SCOPE("Tile Based Cull");
		// Conduct Light Cull
		std::shared_ptr<Shader> lightCull = Engine::Get().GetShaderLibrary().GetShader("FP_Light_Culling");
		if (lightCull) {
			lightCull->Bind();

			lightCull->SetMat4("u_View", camera->GetViewMatrix());
			lightCull->SetMat4("u_Proj", camera->GetProjMatrix());
			lightCull->SetiVec2("u_ScreenSize", glm::ivec2(m_Scene->GetSceneFrameBuffer()->GetConfig().Width, m_Scene->GetSceneFrameBuffer()->GetConfig().Height));

			// Bind depth to texture 4 so this does not interfere with any diffuse, normal, or specular textures used 
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, m_Scene->GetSceneFrameBuffer()->GetTexture(FrameBufferTexture::DepthTexture));
			lightCull->SetInt("u_Depth", 4);

			glDispatchCompute(FP_Data.workGroupsX, FP_Data.workGroupsY, 1);

			glFlush();

			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);
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

		L_PROFILE_SCOPE("Forward Plus - Render Pass");

		//// Render All MeshComponents in Scene

		if (!FP_Data.RenderableEntities.empty()) {

			// Key #1 = Material Asset ID
			// Key #2 = SubMesh Pointer
			// Value = array of entities to render using material and submesh
			std::unordered_map<AssetHandle, std::unordered_map<std::shared_ptr<SubMesh>, std::vector<UUID>>> renderables;

			// Kinda like AssetMap in the AssetManager, but this is pretty much purely 
			// for the renderer so GetAsset calls are reduced to the AssetManager
			static std::unordered_map<AssetHandle, std::shared_ptr<AssetMesh>> fast_loaded_asset_references; 

			auto& debug_line_shader = Engine::Get().GetShaderLibrary().GetShader("Debug_Line_Draw");

			if(debug_line_shader)
			{
				debug_line_shader->Bind();
				debug_line_shader->SetVec4("u_LineColor", { 0.0f, 1.0f, 0.0f, 1.0f });
				debug_line_shader->SetMat4("u_VertexIn.Proj", camera->GetProjMatrix());
				debug_line_shader->SetMat4("u_VertexIn.View", camera->GetViewMatrix());
				debug_line_shader->SetBool("u_UseInstanceData", false);
			}

			// Identify Renderables - we have culling happy! :)
			for (auto& entity : FP_Data.RenderableEntities) {

				auto& component = entity.GetComponent<AssetMeshFilter>();
				auto& mesh_asset = fast_loaded_asset_references[component.MeshFilterAssetHandle];

				if (!mesh_asset) {
					mesh_asset = Project::GetStaticEditorAssetManager()->GetAsset<AssetMesh>(component.MeshFilterAssetHandle);
					if (!mesh_asset) continue;
				}

				auto& submeshes = mesh_asset->SubMeshes;
				auto& material_handles = entity.GetComponent<AssetMeshRenderer>().MeshRendererMaterialHandles;

				// Ensure the renderables map is reserved to avoid frequent reallocations
				for (size_t i = 0; i < material_handles.size() && i < submeshes.size(); ++i) {
					auto& material_map = renderables[material_handles[i]];
					auto& entity_list = material_map[submeshes[i]];
					entity_list.reserve(256); // TODO: CHANGE THIS LATER YALL
					entity_list.push_back(entity.GetUUID());
				}

				if (component.GetShouldDisplayDebugLines() && debug_line_shader) {
					debug_line_shader->SetMat4("u_VertexIn.Model", component.TransformedAABB.GetGlobalBoundsMat4());
					Renderer::DrawDebugCube();
				}
			}

			// Lets colour in some triangles!
			for (const auto& [material_asset_handle, mesh_map] : renderables) {

				auto material_asset = Project::GetStaticEditorAssetManager()->GetAsset<PBRMaterial>(material_asset_handle);

				if (!material_asset)
					continue;
				
				if (!material_asset->Bind())
					continue;

				auto shader = material_asset->GetShader();
				material_asset->UpdateUniforms(*camera);

				// Update Specific Forward Plus Uniforms
				shader->SetInt("u_TilesX", FP_Data.workGroupsX);
				shader->SetInt("u_ShowLightComplexity", FP_Data.ShowLightComplexity);


				for (const auto& [sub_mesh, entities] : mesh_map) {

					size_t entity_count = entities.size();
					if (entity_count == 0)
						continue;

					bool use_instance_data = (entity_count > 1);
					shader->SetBool("u_UseInstanceData", use_instance_data);

					if (use_instance_data) {
						std::vector<glm::mat4> transforms;
						transforms.reserve(entity_count);

						for (const auto& entity : entities) {
							const auto& transform = m_Scene->FindEntityByUUID(entity).GetComponent<Transform>().GetGlobalTransform();
							transforms.push_back(transform);
						}

						Renderer::DrawInstancedSubMesh(sub_mesh, transforms);

					}
					else {
						const auto& transform = m_Scene->FindEntityByUUID(entities[0]).GetComponent<Transform>().GetGlobalTransform();
						shader->SetMat4("u_VertexIn.Model", transform);
						Renderer::DrawSubMesh(sub_mesh);
					}
				}
			}
		}

		auto skyboxView = m_Scene->GetAllEntitiesWith<CameraComponent, SkyboxComponent>();
		if (skyboxView.begin() != skyboxView.end()) {

			for (const auto& entity : skyboxView) {
				auto [scene_camera, skybox] = skyboxView.get<CameraComponent, SkyboxComponent>(entity);

				// Only draw the skybox for the primary camera
				if (scene_camera.Primary && scene_camera.ClearFlags == CameraClearFlags::SKYBOX) {

					if (auto mat_ref = Project::GetStaticEditorAssetManager()->GetAsset<SkyboxMaterial>(skybox.SkyboxMaterialAssetHandle); mat_ref){

						skybox.Bind();
						if (mat_ref->Bind()) {

							mat_ref->UpdateUniforms(camera);
							Renderer::DrawSkybox(skybox);
							mat_ref->UnBind();
						}
						skybox.UnBind();
					}
				}
			}
		}

		// RENDER DEBUG VIEW FOR OCTREE
		if (auto octree_ref = m_Scene->GetOctree().lock(); octree_ref && m_Scene->GetDisplayOctree()) {

			auto& debug_line_shader = Engine::Get().GetShaderLibrary().GetShader("Debug_Line_Draw");

			// Draw Octree
			if (debug_line_shader) {
				debug_line_shader->Bind();
				debug_line_shader->SetVec4("u_LineColor", { 1.0f, 0.0f, 0.0f, 1.0f });
				debug_line_shader->SetMat4("u_VertexIn.Proj", camera->GetProjMatrix());
				debug_line_shader->SetMat4("u_VertexIn.View", camera->GetViewMatrix());
				debug_line_shader->SetBool("u_UseInstanceData", true);
			}

			Renderer::DrawInstancedDebugCube(octree_ref->GetAllOctreeBoundsMat4());

			if (debug_line_shader) {
				debug_line_shader->Bind();
				debug_line_shader->SetVec4("u_LineColor", { 0.0f, 1.0f, 0.0f, 1.0f });
				debug_line_shader->SetMat4("u_VertexIn.Proj", camera->GetProjMatrix());
				debug_line_shader->SetMat4("u_VertexIn.View", camera->GetViewMatrix());
				debug_line_shader->SetBool("u_UseInstanceData", true);
			}

			// Draw All Bounds of Data Sources in Octree
			static std::vector<glm::mat4> bounds_matricies{};
			if(bounds_matricies.capacity() == 0) bounds_matricies.reserve(1024);
			bounds_matricies.clear();

			for (auto& entity : FP_Data.RenderableEntities) {
				if(entity.HasComponent<AssetMeshFilter>())
					bounds_matricies.push_back(entity.GetComponent<AssetMeshFilter>().TransformedAABB.GetGlobalBoundsMat4());
			}

			Renderer::DrawInstancedDebugCube(bounds_matricies);
		}


		// Clean Up Scene Render Pass
		for (int i = 0; i < 4; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);
	}

	void ForwardPlusPipeline::RenderFBOQuad() {

		if (!FP_Data.Screen_Quad_VAO) {
			return;
		}

		std::shared_ptr<Shader> shader = Engine::Get().GetShaderLibrary().GetShader("FBO Texture Shader");
		if (shader)
		{
			shader->Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_Scene->GetSceneFrameBuffer()->GetTexture(FrameBufferTexture::ColourTexture));
			shader->SetInt("u_ScreenTexture", 0);

			FP_Data.Screen_Quad_VAO->Bind();
			glDrawElements(GL_TRIANGLES, FP_Data.Screen_Quad_VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else {
			L_CORE_ERROR("FBO Texture Shader Invalid.");
		}
	}

	bool ForwardPlusPipeline::IsSphereInsideFrustum(const Bounds_Sphere& bounds, const Frustum& frustum) {
		for (const auto& plane : frustum.planes) {
			float distance = glm::dot(plane.normal, bounds.BoundsCentre) + plane.distance;
			if (distance < -bounds.BoundsRadius) {
				return false; // Sphere is outside this plane
			}
		}
		return true; // Sphere is inside or intersecting the frustum
	}

#pragma endregion

#pragma region DeferredPipeline

	void DeferredPipeline::OnUpdate() {

	}

	void DeferredPipeline::OnStartPipeline(std::shared_ptr<Louron::Scene> scene) {

	}

	void DeferredPipeline::OnStopPipeline() {

		Renderer::CleanupRenderData();
	}

#pragma endregion

	void Frustum::RecalculateFrustum(const glm::mat4& viewProjectionMatrix)
	{		// Left plane
		planes[0].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0];
		planes[0].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0];
		planes[0].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0];
		planes[0].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0];

		// Right plane
		planes[1].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0];
		planes[1].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0];
		planes[1].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0];
		planes[1].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0];

		// Bottom plane
		planes[2].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1];
		planes[2].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1];
		planes[2].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1];
		planes[2].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1];

		// Top plane
		planes[3].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1];
		planes[3].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1];
		planes[3].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1];
		planes[3].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1];

		// Near plane
		planes[4].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][2];
		planes[4].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][2];
		planes[4].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][2];
		planes[4].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][2];

		// Far plane
		planes[5].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2];
		planes[5].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2];
		planes[5].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2];
		planes[5].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2];

		// Normalize the planes
		for (int i = 0; i < 6; ++i) {
			planes[i].normalize();
		}
	}

}
