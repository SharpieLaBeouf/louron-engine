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
			GLuint shadowLayerIndex = -1;

			GLfloat m_Padding1 = 0.0f;
			GLfloat m_Padding2 = 0.0f;

			PL_SSBO_DATA_LAYOUT() = default;

			PL_SSBO_DATA_LAYOUT(const PointLightComponent& point_light, TransformComponent& transform) {

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

			SL_SSBO_DATA_LAYOUT(const SpotLightComponent& spot_light, TransformComponent& transform) {

				position  = { transform.GetGlobalPosition(), 1.0f };
				direction = { transform.GetForwardDirection(), 1.0f };
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

			std::array<float, 5> shadowCascadePlaneDistances = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

			GLuint shadowCastingType = 0;
			GLuint shadowLightIndex = 0;

			// DO NOT USE - this is for SSBO alignment purposes ONLY
			GLfloat m_Padding1 = 0.0f;
			GLfloat m_Padding2 = 0.0f;

			DL_SSBO_DATA_LAYOUT() = default;

			DL_SSBO_DATA_LAYOUT(const DirectionalLightComponent& directional_light, TransformComponent& transform) {
				
				direction = { transform.GetForwardDirection(), 1.0f };
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

	void RenderPipeline::OnUpdate(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix)
	{
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

	void RenderPipeline::ConductRenderPass(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix) {


	}

#pragma endregion

#pragma region ForwardPlusPipeline

	/// <summary>
	/// This is the main loop for rendering logic
	/// in the Forward+ Pipeline.
	/// </summary>
	void ForwardPlusPipeline::OnUpdate(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix) {

		L_PROFILE_SCOPE("Forward Plus");

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene! Please Use ForwardPlusPipeline::OnStartPipeline() Before Updating");
			Renderer::ClearColour({ 0.0f, 0.0f, 0.0f, 1.0f });
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			return;
		}

		Renderer::ClearRenderStats();

		// 1. SCENE FRUSTUM CULLING
		{
			L_PROFILE_SCOPE("Forward Plus - Frustum Culling");

			// Recalculate Camera Frustum
			FP_Data.Camera_Frustum.RecalculateFrustum(projection_matrix * view_matrix);

			// Wait for Octree Update Thread
			if (FP_Data.OctreeUpdateThread.joinable()) {
				L_PROFILE_SCOPE("Forward Plus - Octree Thread Wait");
				FP_Data.OctreeUpdateThread.join();
			}

			// Gather All Point and Spot Lights Visible in Camera Frustum
			FP_Data.PLEntities.clear();
			FP_Data.SLEntities.clear();
			FP_Data.DLEntities.clear();
			ConductLightFrustumCull();

			// Gather All Meshes Visible in Camera Frustum
			ConductRenderableFrustumCull();

			// Dispatch Thread
			FP_Data.OctreeUpdateThread = std::thread([&]() -> void {

				auto oct_scene_ref = m_Scene.lock();

				L_PROFILE_SCOPE("Forward Plus - Octree Update");
					
				if (auto oct_ref = oct_scene_ref->GetOctree().lock(); oct_ref) {

					std::unique_lock lock(oct_ref->GetOctreeMutex());
						
					// Check which objects are no longer in the scene
					const auto& octree_data_sources = oct_ref->GetAllOctreeDataSources();
					std::vector<Entity> remove_entities{};
					for (const auto& data_source : octree_data_sources) {

						if (oct_scene_ref->HasEntity(data_source->Data)) {
							Entity entity = data_source->Data;
							if (!entity.HasComponent<AssetMeshFilter>() || !entity.HasComponent<AssetMeshRenderer>()) {
								remove_entities.push_back(entity);
							}
						}
						else {
							remove_entities.push_back(data_source->Data);
						}
					}

					for (const auto& entity : remove_entities) 
						oct_ref->Remove(entity);

					oct_ref->TryShrinkOctree();
					auto mesh_view = oct_scene_ref->GetAllEntitiesWith<AssetMeshRenderer, AssetMeshFilter>();
					for (const auto& entity_handle : mesh_view) {

						auto& component = mesh_view.get<AssetMeshFilter>(entity_handle);

						if (component.MeshFilterAssetHandle == NULL_UUID)
							continue;

						if (component.AABBNeedsUpdate) {
							component.UpdateTransformedAABB();
						}

						if (component.OctreeNeedsUpdate) {
							if (oct_ref->Update({ entity_handle, oct_scene_ref.get() }, component.TransformedAABB))
								component.OctreeNeedsUpdate = false;
							else {
								L_CORE_WARN("Could Not Be Inserted Into Octree - Deleting Entity: {0}", component.GetEntity().GetName());
								oct_scene_ref->DestroyEntity({ entity_handle, oct_scene_ref.get() });
							}
						}
					}
				}
			});
		}

		// 2. RENDERING
		{
			L_PROFILE_SCOPE("Forward Plus - Rendering");

			ConductShadowMapping(camera_position, projection_matrix, view_matrix);

			UpdateSSBOData();

			// Bind FBO and clear color and depth buffers for the new frame
			scene_ref->GetSceneFrameBuffer()->Bind();
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				
			glPolygonMode(GL_FRONT_AND_BACK, FP_Data.ShowWireframe ? GL_LINE : GL_FILL);

			ConductDepthPass(camera_position, projection_matrix, view_matrix);
			ConductTiledBasedLightCull(projection_matrix, view_matrix);

			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			Entity camera_entity = scene_ref->GetPrimaryCameraEntity();
			Renderer::ClearColour(camera_entity ? camera_entity.GetComponent<CameraComponent>().ClearColour : glm::vec4(49.0f, 77.0f, 121.0f, 1.0f));
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT);
			
			ConductRenderPass(camera_position, projection_matrix, view_matrix);
			
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			// Unbind FBO to render to the screen
			scene_ref->GetSceneFrameBuffer()->Unbind();

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
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL); 

		m_Scene = scene;

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

		if (!scene_ref->GetSceneFrameBuffer()) {
			L_CORE_ERROR("Framebuffer is Invalid.");
			OnStopPipeline();
			return;
		}

		if (!scene_ref->GetSceneFrameBuffer()->IsValid()) {
			L_CORE_ERROR("Framebuffer Setup Not Complete.");
			OnStopPipeline();
			return;
		}

		if (Entity cameraEntity = scene_ref->GetPrimaryCameraEntity()) {

			auto& camera_component = cameraEntity.GetComponent<CameraComponent>();

			if (!camera_component.CameraInstance) {
				cameraEntity.GetComponent<CameraComponent>().CameraInstance = std::make_shared<SceneCamera>();

				auto& frame_buffer_config = scene_ref->GetSceneFrameBuffer()->GetConfig();
				cameraEntity.GetComponent<CameraComponent>().CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);
			}
			else {
				auto& frame_buffer_config = scene_ref->GetSceneFrameBuffer()->GetConfig();
				cameraEntity.GetComponent<CameraComponent>().CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);
			}
		}
		else {
			cameraEntity = scene_ref->CreateEntity("Main Camera");
			cameraEntity.AddComponent<CameraComponent>();
			cameraEntity.GetComponent<CameraComponent>().CameraInstance = std::make_shared<SceneCamera>();

			auto& frame_buffer_config = scene_ref->GetSceneFrameBuffer()->GetConfig();
			cameraEntity.GetComponent<CameraComponent>().CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);
		}

		// Calculate workgroups and generate SSBOs from screen size
		FP_Data.workGroupsX = (unsigned int)std::ceil((float)scene_ref->GetSceneFrameBuffer()->GetConfig().Width / 16.0f);
		FP_Data.workGroupsY = (unsigned int)std::ceil((float)scene_ref->GetSceneFrameBuffer()->GetConfig().Height / 16.0f);
		size_t numberOfTiles = static_cast<size_t>(FP_Data.workGroupsX * FP_Data.workGroupsY);

		// Setup Light Buffers

		glGenBuffers(1, &FP_Data.DL_Buffer);
		glGenBuffers(1, &FP_Data.DL_Shadow_LightSpaceMatrix_Buffer);

		glGenBuffers(1, &FP_Data.PL_Buffer);
		glGenBuffers(1, &FP_Data.PL_Indices_Buffer);

		glGenBuffers(1, &FP_Data.SL_Buffer);
		glGenBuffers(1, &FP_Data.SL_Indices_Buffer);

		// Directional Lights
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_DIRECTIONAL_LIGHTS * sizeof(SSBOLightStructs::DL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Shadow_LightSpaceMatrix_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 5 * sizeof(glm::mat4) * 5, nullptr, GL_DYNAMIC_DRAW);

		// Point Lights
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_POINT_LIGHTS * sizeof(SSBOLightStructs::PL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW); // All light data

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_POINT_LIGHTS, nullptr, GL_DYNAMIC_DRAW); // List of visible lights per tile

		// Spot Lights
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_SPOT_LIGHTS * sizeof(SSBOLightStructs::SL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW); // All light data

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_SPOT_LIGHTS, nullptr, GL_DYNAMIC_DRAW); // List of visible lights per tile

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// SHADOWS - Cube Map Array
		glGenTextures(1, &FP_Data.PL_Shadow_CubeMap_Array);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, FP_Data.PL_Shadow_CubeMap_Array);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_DEPTH_COMPONENT24, 
			FP_Data.PL_Shadow_Map_Res, FP_Data.PL_Shadow_Map_Res, FP_Data.PL_Shadow_Max_Maps * 6);

		// SHADOWS - Frame Buffer
		glGenFramebuffers(1, &FP_Data.PL_Shadow_FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.PL_Shadow_FrameBuffer);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FP_Data.PL_Shadow_CubeMap_Array, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// SHADOWS - Directional Depth Texture
		glGenTextures(1, &FP_Data.DL_Shadow_Texture_Array);
		glBindTexture(GL_TEXTURE_2D_ARRAY, FP_Data.DL_Shadow_Texture_Array);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, FP_Data.DL_Shadow_Map_Res, FP_Data.DL_Shadow_Map_Res, FP_Data.DL_Shadow_Max_Maps * 5); // Max 25 shadow maps, 5 cascades per light, max 5 shadow directional lights

		// SHADOWS - Framebuffer
		glGenFramebuffers(1, &FP_Data.DL_Shadow_FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.DL_Shadow_FrameBuffer);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FP_Data.DL_Shadow_Texture_Array, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

		FP_Data.PLEntities.reserve(MAX_POINT_LIGHTS);
		FP_Data.SLEntities.reserve(MAX_SPOT_LIGHTS);
		FP_Data.DLEntities.reserve(MAX_DIRECTIONAL_LIGHTS);
	}

	/// <summary>
	/// Reset OpenGL state configuration required by renderer and clean FP_Data and Light SSBOs.
	/// </summary>
	void ForwardPlusPipeline::OnStopPipeline() {
		
		if (FP_Data.OctreeUpdateThread.joinable()) {
			FP_Data.OctreeUpdateThread.join();
		}

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		
		glDeleteBuffers(1, &FP_Data.PL_Buffer);
		glDeleteBuffers(1, &FP_Data.PL_Indices_Buffer);

		glDeleteBuffers(1, &FP_Data.SL_Buffer);
		glDeleteBuffers(1, &FP_Data.SL_Indices_Buffer);

		glDeleteBuffers(1, &FP_Data.DL_Buffer);
		glDeleteBuffers(1, &FP_Data.DL_Shadow_LightSpaceMatrix_Buffer);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &FP_Data.PL_Shadow_FrameBuffer);
		glDeleteTextures(1, &FP_Data.PL_Shadow_CubeMap_Array);

		glDeleteFramebuffers(1, &FP_Data.DL_Shadow_FrameBuffer);
		glDeleteTextures(1, &FP_Data.DL_Shadow_Texture_Array);

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

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene! Please Use ForwardPlusPipeline::OnStartPipeline() Before Updating Viewport");
			Renderer::ClearColour({ 1.0f, 1.0f, 1.0f, 1.0f });
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			return;
		}

		// Update Projection Matrix of All Scene Cameras
		auto camera_view = scene_ref->GetAllEntitiesWith<CameraComponent>();
		auto& size = scene_ref->GetSceneFrameBuffer()->GetConfig();
		for (const auto& entity : camera_view) {
			if (auto cam_ref = camera_view.get<CameraComponent>(entity).CameraInstance; cam_ref) {
				cam_ref->SetViewportSize(size.Width, size.Height);
			}
		}

		UpdateComputeData();
	}

	/// <summary>
	/// This updates the Forward+ data for the applicable Compute and Vertex/Fragment shaders.
	/// </summary>
	void ForwardPlusPipeline::UpdateComputeData() {

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

		// Calculate Workgroups and Generate SSBOs from Screen Size
		FP_Data.workGroupsX = (unsigned int)std::ceil((float)scene_ref->GetSceneFrameBuffer()->GetConfig().Width / 16.0f);
		FP_Data.workGroupsY = (unsigned int)std::ceil((float)scene_ref->GetSceneFrameBuffer()->GetConfig().Height / 16.0f);
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

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

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
					{
						s_PointLightVector.push_back({ point_light, entity.GetComponent<TransformComponent>() });
						s_PointLightVector.back().radius *= 2.0f;

						if(FP_Data.PL_Shadow_LightIndexMap.find(entity.GetUUID()) != FP_Data.PL_Shadow_LightIndexMap.end()) {
							s_PointLightVector.back().shadowLayerIndex = FP_Data.PL_Shadow_LightIndexMap.at(entity.GetUUID());
						}
					}
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
						s_SpotLightVector.push_back({ spot_light, entity.GetComponent<TransformComponent>() });
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
				static std::vector<SSBOLightStructs::DL_SSBO_DATA_LAYOUT> s_DirectionalLightVector(MAX_DIRECTIONAL_LIGHTS);
				s_DirectionalLightVector.clear();

				// Add lights to vector that are contained within the scene up to a maximum of 10
				for (auto& entity : FP_Data.DLEntities) {

					if (s_DirectionalLightVector.size() >= MAX_DIRECTIONAL_LIGHTS)
						break;

					auto& directional_light = entity.GetComponent<DirectionalLightComponent>();
					if (directional_light.Active)
					{
						s_DirectionalLightVector.push_back({ directional_light, entity.GetComponent<TransformComponent>() });

						if (FP_Data.DL_Shadow_LightSpaceMatrixIndex.find(entity.GetUUID()) != FP_Data.DL_Shadow_LightSpaceMatrixIndex.end())
							s_DirectionalLightVector.back().shadowLightIndex = FP_Data.DL_Shadow_LightSpaceMatrixIndex.at(entity.GetUUID());

						if (FP_Data.DL_Shadow_LightShadowCascadeDistances.find(entity.GetUUID()) != FP_Data.DL_Shadow_LightShadowCascadeDistances.end())
							s_DirectionalLightVector.back().shadowCascadePlaneDistances = FP_Data.DL_Shadow_LightShadowCascadeDistances.at(entity.GetUUID());
					}
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

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

		L_PROFILE_SCOPE("Forward Plus - Frustum Culling (LIGHTS)");

		auto pl_view = scene_ref->GetAllEntitiesWith<PointLightComponent>();
		for (const auto& entity_handle : pl_view) {

			if (FP_Data.PLEntities.size() >= MAX_POINT_LIGHTS)
				break;

			if (!pl_view.get<PointLightComponent>(entity_handle).Active)
				continue;

			Entity entity = { entity_handle, scene_ref.get() };
			if (IsSphereInsideFrustum({ entity.GetComponent<TransformComponent>().GetGlobalPosition(), entity.GetComponent<PointLightComponent>().Radius }, FP_Data.Camera_Frustum)) {
				FP_Data.PLEntities.push_back(entity);
			}

		}

		auto sl_view = scene_ref->GetAllEntitiesWith<SpotLightComponent>();
		for (const auto& entity_handle : sl_view) {

			if (FP_Data.SLEntities.size() >= MAX_SPOT_LIGHTS)
				break;

			if (!sl_view.get<SpotLightComponent>(entity_handle).Active)
				continue;

			Entity entity = { entity_handle, scene_ref.get() };

			auto& transform = entity.GetComponent<TransformComponent>();
			auto& light = entity.GetComponent<SpotLightComponent>();

			float half_angle = glm::radians(light.Angle * 0.5f);
			float cosPenumbra = cos(half_angle);

			Bounds_Sphere sphere;
			if (half_angle > glm::pi<float>() / 4.0f) {
				sphere.BoundsCentre = transform.GetGlobalPosition() + cosPenumbra * light.Range * transform.GetForwardDirection();
				sphere.BoundsRadius = sin(half_angle) * light.Range;
			}
			else
			{
				sphere.BoundsCentre = transform.GetGlobalPosition() + light.Range / (2.0f * cosPenumbra) * transform.GetForwardDirection();
				sphere.BoundsRadius = light.Range / (2.0f * cosPenumbra);
			}

			if (IsSphereInsideFrustum(sphere, FP_Data.Camera_Frustum)) {
				FP_Data.SLEntities.push_back(entity);
			}

		}

		auto dl_view = scene_ref->GetAllEntitiesWith<DirectionalLightComponent>();
		for (const auto& entity_handle : dl_view) {

			if (FP_Data.DLEntities.size() >= MAX_DIRECTIONAL_LIGHTS)
				break;

			if (!dl_view.get<DirectionalLightComponent>(entity_handle).Active)
				continue;

			Entity entity = { entity_handle, scene_ref.get() };
			FP_Data.DLEntities.push_back(entity);
		}
	}

	/// <summary>
	/// This will cull all scene geometry outside camera frustum and 
	/// update the renderables vector in FP_Data.
	/// </summary>
	void ForwardPlusPipeline::ConductRenderableFrustumCull() {

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

		L_PROFILE_SCOPE("Forward Plus - Frustum Culling Octree Query");
		if (auto oct_ref = scene_ref->GetOctree().lock(); oct_ref) {

			std::unique_lock lock(oct_ref->GetOctreeMutex());
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
		{
			if (!data->Data)
				continue;

			auto& component = data->Data.GetComponent<AssetMeshRenderer>();
			if (component.Active)
				FP_Data.RenderableEntities.push_back(data->Data);
		}

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
	void ForwardPlusPipeline::ConductDepthPass(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix) {

		L_PROFILE_SCOPE("Forward Plus - Depth Pass");

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

		scene_ref->GetSceneFrameBuffer()->ClearEntityPixelData(NULL_UUID);

		// Key #1 - Distance - Value UUID of Entities that have submeshes
		std::vector<std::tuple<float, UUID, std::shared_ptr<SubMesh>>> sorted_entities;

		if (!FP_Data.RenderableEntities.empty()) {

			for (auto& entity : FP_Data.RenderableEntities) 
			{

				if (!scene_ref->ValidEntity(entity)) continue;

				auto asset_mesh = Project::GetStaticEditorAssetManager()->GetAsset<AssetMesh>(entity.GetComponent<AssetMeshFilter>().MeshFilterAssetHandle);

				if (!asset_mesh)
					continue;

				const glm::vec3& objectPosition = entity.GetComponent<TransformComponent>().GetGlobalPosition();
				float distance = glm::length(objectPosition - camera_position);

				for (std::shared_ptr<SubMesh> sub_mesh : asset_mesh->SubMeshes) {
					sorted_entities.emplace_back(distance, entity.GetUUID(), sub_mesh);
				}

			}
						
			// Lambda function compares the distances of two entities (a and b) and orders them in a way that ensures front-to-back sorting
			std::sort(sorted_entities.begin(), sorted_entities.end(), [](const auto& a, const auto& b) {
				return std::get<0>(a) < std::get<0>(b);
			});
			
			if (std::shared_ptr<Shader> shader = Engine::Get().GetShaderLibrary().GetShader("FP_Depth"); shader) 
			{

				shader->Bind();
				shader->SetMat4("u_Proj", projection_matrix);
				shader->SetMat4("u_View", view_matrix);

				for (auto& [distance, entity_uuid, sub_mesh] : sorted_entities) 
				{
					shader->SetMat4("u_Model", scene_ref->FindEntityByUUID(entity_uuid).GetComponent<TransformComponent>().GetGlobalTransform());
					shader->SetUInt("u_EntityID", entity_uuid);

					Renderer::DrawSubMesh(sub_mesh);
				}

				shader->UnBind();
			}
			else 
			{
				L_CORE_ERROR("FP Depth Shader Not Found");
			}
		}

		glFlush();
	}

	/// <summary>
	/// Conducts main tiled rendering algorithm, split screen into tiles
	/// and determine which lights impact each tile frustum.
	/// </summary>
	void ForwardPlusPipeline::ConductTiledBasedLightCull(const glm::mat4& projection_matrix, const glm::mat4& view_matrix) {

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

		L_PROFILE_SCOPE("Tile Based Cull");
		// Conduct Light Cull
		std::shared_ptr<Shader> lightCull = Engine::Get().GetShaderLibrary().GetShader("FP_Light_Culling");
		if (lightCull) {

			lightCull->Bind();

			lightCull->SetMat4("u_View", view_matrix);
			lightCull->SetMat4("u_Proj", projection_matrix);
			
			auto& size = scene_ref->GetSceneFrameBuffer()->GetConfig();
			lightCull->SetiVec2("u_ScreenSize", glm::ivec2(size.Width, size.Height));

			// Bind depth to texture 4 so this does not interfere with any diffuse, normal, or specular textures used 
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, scene_ref->GetSceneFrameBuffer()->GetTexture(FrameBufferTexture::DepthTexture));
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

	void ForwardPlusPipeline::ConductShadowMapping(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix)
	{
		L_PROFILE_SCOPE("Shadow Mapping Total");

		//constexpr int update_per_number_of_frames = 5;
		//static int frames_passed = update_per_number_of_frames;
		//if (frames_passed < update_per_number_of_frames) {
		//	frames_passed++;
		//	return;
		//}
		//frames_passed = 0;
		
		auto scene_ref = m_Scene.lock();
		if (!scene_ref)
			return;

		const glm::mat4& camera_proj_view = projection_matrix * view_matrix;

		#pragma region Directional Light Shadows

		std::vector<Entity> dl_shadow_casting_vec;
		std::vector<Entity> dl_shadow_renderable_entities;
		std::vector<glm::mat4> dl_shadow_light_space_matricies;
		std::unordered_map<UUID, glm::mat4> dl_light_proj_view_matrix_map; // What meshes are inside this directional light?

		// 1. Gather Directional Lights
			
		{
			L_PROFILE_SCOPE("Directional Shadow Mapping 1. Gathering");
			dl_shadow_casting_vec.reserve(FP_Data.DLEntities.size());
			for (auto& entity : FP_Data.DLEntities)
				if (entity.GetComponent<DirectionalLightComponent>().ShadowFlag != ShadowTypeFlag::NoShadows)
				{
					if (dl_shadow_casting_vec.size() >= FP_Data.DL_Shadow_Max_Maps)
						break;

					dl_shadow_casting_vec.push_back(entity);
				}
		}

		// 2. Calculate Light Space World Space AABB - Get Meshes Intersecting with AABB from Octree
		// TODO: Need to fix this because it is not including objects that are behind camera frustum that 
		// may cast shadow into frustum. Maybe we do this after generating the cascades and use the 
		// world space AABB of the light projection to find our meshes?
		Bounds_AABB world_light_bounds;
		{
			L_PROFILE_SCOPE("Directional Shadow Mapping 2a. Calculate Light Space Bounds");

			world_light_bounds = Frustum::GetWorldSpaceTightBoundingBox(camera_proj_view);

		}

		{
			L_PROFILE_SCOPE("Directional Shadow Mapping 2b. Get Meshes");

			if (auto oct_ref = scene_ref->GetOctree().lock(); oct_ref) {

				std::unique_lock lock(oct_ref->GetOctreeMutex());

				const auto& query_vec = oct_ref->Query(world_light_bounds);

				dl_shadow_renderable_entities.reserve(query_vec.size());

				for (const auto& data : query_vec)
				{
					if (!data->Data)
						continue;

					auto& component = data->Data.GetComponent<AssetMeshRenderer>();
					if (component.Active && component.CastShadows)
						dl_shadow_renderable_entities.push_back(data->Data);
				}
			}

		}

		// 3. Calculate Light Space Matricies Per Light Per Cascade - 40 x glm::mat4's is the max = MAX_DIRECTIONAL_LIGHTS * 4 cascades (per directional light)
			
		{
			L_PROFILE_SCOPE("Directional Shadow Mapping 3. Calculate Cascade Light Matricies");
			dl_shadow_light_space_matricies.reserve(dl_shadow_casting_vec.size() * 5);
			FP_Data.DL_Shadow_LightSpaceMatrixIndex.clear();
			FP_Data.DL_Shadow_LightShadowCascadeDistances.clear();

			for (int light_index = 0; light_index < dl_shadow_casting_vec.size(); light_index++)
			{
				Entity entity = dl_shadow_casting_vec[light_index];
				if (!entity)
					continue;

				std::array<float, 5>& shadow_cascade_distances = FP_Data.DL_Shadow_LightShadowCascadeDistances[entity.GetUUID()];
				std::array<glm::mat4, 5> light_space_matrices = Frustum::CalculateCascadeLightSpaceMatrices(projection_matrix, view_matrix, entity.GetComponent<TransformComponent>().GetForwardDirection(), shadow_cascade_distances);

				dl_shadow_light_space_matricies.insert(dl_shadow_light_space_matricies.end(), light_space_matrices.begin(), light_space_matrices.end());
				FP_Data.DL_Shadow_LightSpaceMatrixIndex[entity.GetUUID()] = light_index;
			}
		}

		// 4. Update Light Space Matrix SSBO Data
			
		{
			L_PROFILE_SCOPE("Directional Shadow Mapping 4. Updating Light Matrix SSBO Data");
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, FP_Data.DL_Shadow_LightSpaceMatrix_Buffer);

			// Update SSBO data with light data
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Shadow_LightSpaceMatrix_Buffer);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dl_shadow_light_space_matricies.size() * sizeof(glm::mat4), dl_shadow_light_space_matricies.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		// 5. Render Shadow Map from Light Space Matrix View & Proj

		{
			L_PROFILE_SCOPE("Directional Shadow Mapping 5. Rendering Cascaded Shadow Maps");
			glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.DL_Shadow_FrameBuffer);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FP_Data.DL_Shadow_Texture_Array, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			glCullFace(GL_FRONT);

			glViewport(0, 0, FP_Data.DL_Shadow_Map_Res, FP_Data.DL_Shadow_Map_Res);
			glClear(GL_DEPTH_BUFFER_BIT);

			auto& shader = Engine::Get().GetShaderLibrary().GetShader("FP_Shadow_Directional");
			shader->Bind();

			for (int light_index = 0; light_index < dl_shadow_casting_vec.size(); ++light_index) {

				Entity entity = dl_shadow_casting_vec[light_index];
				if (!entity)
					continue;

				shader->SetUInt("u_LightIndex", light_index);

				for (auto& mesh_entity : dl_shadow_renderable_entities) {

					if (!mesh_entity)
						continue;

					glm::mat4 transform = mesh_entity.GetComponent<TransformComponent>().GetGlobalTransform();
					shader->SetMat4("u_Model", transform);

					std::shared_ptr<AssetMesh> asset_mesh = Project::GetStaticEditorAssetManager()->GetAsset<AssetMesh>(mesh_entity.GetComponent<AssetMeshFilter>().MeshFilterAssetHandle);
					for (auto& sub_mesh : asset_mesh->SubMeshes)
						Renderer::DrawSubMesh(sub_mesh);
				}

			}

			shader->UnBind();

			if (scene_ref)
				scene_ref->GetSceneFrameBuffer()->Bind();

			glCullFace(GL_BACK);
		}

		#pragma endregion

		#pragma region Point Light Shadows

		FP_Data.PL_Shadow_LightIndexMap.clear();

		std::vector<Entity> pl_shadow_casting_vec;
		std::unordered_map<UUID, std::vector<Entity>> pl_shadow_casting_meshes_map; // What meshes are inside this point light?
		constexpr int numShadowCastingLights = 5; // Number of shadow-casting lights
			
		// 1. Gather and Sort Point Lights that have shadow mapping enabled
		{
			L_PROFILE_SCOPE("Point Shadow Mapping 1. Sorting");
			pl_shadow_casting_vec.reserve(FP_Data.PLEntities.size());
			for (auto& entity : FP_Data.PLEntities)
				if (entity.GetComponent<PointLightComponent>().ShadowFlag != ShadowTypeFlag::NoShadows)
					pl_shadow_casting_vec.push_back(entity);

			// Sort array based on distance
			std::sort(pl_shadow_casting_vec.begin(), pl_shadow_casting_vec.end(),
				[&camera_position](Entity& a, Entity& b) {
					glm::vec3 posA = a.GetComponent<TransformComponent>().GetGlobalPosition();
					glm::vec3 posB = b.GetComponent<TransformComponent>().GetGlobalPosition();
					return glm::length(posA - camera_position) < glm::length(posB - camera_position);
				});

			// Keep only the closest 5 point lights
			// TODO: Increase this so there is like a shadow map atlas with lower resolutions? 
			// E.g., One cube map in the array could hold 4 more point light textures if the resolution is halved?
			// Maybe we implement an algorithm to determine which are the most important point lights, 
			//		- Create a cube map array with 25 x 2k textures
			//		- assign a hard limit of maybe 5 x 2K cube maps for the most important point lights, 
			//		- then have another 5 cube maps that are made up of 20 1K point lights, and so on
			if (pl_shadow_casting_vec.size() > numShadowCastingLights)
				pl_shadow_casting_vec.erase(pl_shadow_casting_vec.begin() + numShadowCastingLights, pl_shadow_casting_vec.end());

			for (auto& point_light : pl_shadow_casting_vec) {

				Bounds_Sphere sphere{};
				sphere.BoundsCentre = point_light.GetComponent<TransformComponent>().GetGlobalPosition();
				sphere.BoundsRadius = point_light.GetComponent<PointLightComponent>().Radius;

				if (auto oct_ref = scene_ref->GetOctree().lock(); oct_ref) {

					std::unique_lock lock(oct_ref->GetOctreeMutex());

					const auto& query_vec = oct_ref->Query(sphere);

					std::vector<Entity>& entities_in_light = pl_shadow_casting_meshes_map[point_light.GetUUID()];
					entities_in_light.reserve(query_vec.size());

					for (const auto& data : query_vec)
					{
						if (!data->Data)
							continue;

						auto& component = data->Data.GetComponent<AssetMeshRenderer>();
						if (component.Active && component.CastShadows)
							entities_in_light.push_back(data->Data);
					}
				}

			}

		}

		// 2. Initialise and Draw Shadow CubeMap Array
		{
			L_PROFILE_SCOPE("Point Shadow Mapping 2. Drawing");

			glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.PL_Shadow_FrameBuffer);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FP_Data.PL_Shadow_CubeMap_Array, 0);
			glDrawBuffer(GL_DEPTH_ATTACHMENT);
			glReadBuffer(GL_NONE);

			glCullFace(GL_FRONT);

			glViewport(0, 0, FP_Data.PL_Shadow_Map_Res, FP_Data.PL_Shadow_Map_Res);
			glClear(GL_DEPTH_BUFFER_BIT);

			auto& shader = Engine::Get().GetShaderLibrary().GetShader("FP_Shadow_Point");
			shader->Bind();

			for (int lightIndex = 0; lightIndex < pl_shadow_casting_vec.size(); ++lightIndex) {

				glm::vec3 light_pos = pl_shadow_casting_vec[lightIndex].GetComponent<TransformComponent>().GetGlobalPosition();

				float near_plane = 1.0f;
				float far_plane = pl_shadow_casting_vec[lightIndex].GetComponent<PointLightComponent>().Radius * 2.0f;
				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane);
				std::vector<glm::mat4> shadowTransforms{};
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

				for (unsigned int i = 0; i < 6; ++i)
					shader->SetMat4(std::string("u_ShadowMatrices[" + std::to_string(i) + "]").c_str(), shadowTransforms[i]);

				int layerOffset = lightIndex * 6;
				shader->SetInt("u_LayerOffset", layerOffset);
				shader->SetVec3("u_LightPosition", light_pos);
				shader->SetFloat("u_FarPlane", far_plane);

				// Link up the offset to the light map so we can 
				// update the data into the SSBO for shader access
				FP_Data.PL_Shadow_LightIndexMap.insert({ pl_shadow_casting_vec[lightIndex].GetUUID(), lightIndex });

				// Render all entities THAT ARE IN RANGE of this light in one pass.
				for (auto& entity : pl_shadow_casting_meshes_map[pl_shadow_casting_vec[lightIndex].GetUUID()]) {

					if (!entity)
						continue;

					glm::mat4 transform = entity.GetComponent<TransformComponent>().GetGlobalTransform();
					shader->SetMat4("u_Model", transform);

					std::shared_ptr<AssetMesh> asset_mesh = Project::GetStaticEditorAssetManager()->GetAsset<AssetMesh>(entity.GetComponent<AssetMeshFilter>().MeshFilterAssetHandle);
					for (auto& sub_mesh : asset_mesh->SubMeshes)
						Renderer::DrawSubMesh(sub_mesh);
				}
			}

			shader->UnBind();

			if (scene_ref)
				scene_ref->GetSceneFrameBuffer()->Bind();

			glCullFace(GL_BACK);
		}

		#pragma endregion

	}

	/// <summary>
	/// Conduct final colour pass. Meshes are sorted by material, then
	/// sorted by mesh per material, then drawn individually or 
	/// drawn as instances if the meshes are identical using the same
	/// material.
	/// </summary>
	void ForwardPlusPipeline::ConductRenderPass(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix) {

		L_PROFILE_SCOPE("Forward Plus - Render Pass");

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

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
				debug_line_shader->SetMat4("u_VertexIn.Proj", projection_matrix);
				debug_line_shader->SetMat4("u_VertexIn.View", view_matrix);
				debug_line_shader->SetBool("u_UseInstanceData", false);
			}

			// Identify Renderables - we have culling happy! :)
			for (auto& entity : FP_Data.RenderableEntities) {

				if (!scene_ref->ValidEntity(entity)) continue;

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
				material_asset->UpdateUniforms(camera_position, projection_matrix, view_matrix);

				// Update Specific Forward Plus Uniforms
				shader->SetInt("u_TilesX", FP_Data.workGroupsX);
				shader->SetInt("u_ShowLightComplexity", FP_Data.ShowLightComplexity);

				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, FP_Data.PL_Shadow_CubeMap_Array);
				shader->SetInt("u_PL_ShadowCubeMapArray", 4);

				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D_ARRAY, FP_Data.DL_Shadow_Texture_Array);
				shader->SetInt("u_DL_ShadowMapArray", 5);

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
							const auto& transform = scene_ref->FindEntityByUUID(entity).GetComponent<TransformComponent>().GetGlobalTransform();
							transforms.push_back(transform);
						}

						Renderer::DrawInstancedSubMesh(sub_mesh, transforms);

					}
					else {
						const auto& transform = scene_ref->FindEntityByUUID(entities[0]).GetComponent<TransformComponent>().GetGlobalTransform();
						shader->SetMat4("u_VertexIn.Model", transform);
						Renderer::DrawSubMesh(sub_mesh);
					}
				}
			}
		}

		auto skyboxView = scene_ref->GetAllEntitiesWith<CameraComponent, SkyboxComponent>();
		if (skyboxView.begin() != skyboxView.end()) {

			for (const auto& entity : skyboxView) {
				auto [scene_camera, skybox] = skyboxView.get<CameraComponent, SkyboxComponent>(entity);

				// Only draw the skybox for the primary camera
				if (scene_camera.Primary && scene_camera.ClearFlags == CameraClearFlags::SKYBOX) {

					if (auto mat_ref = Project::GetStaticEditorAssetManager()->GetAsset<SkyboxMaterial>(skybox.SkyboxMaterialAssetHandle); mat_ref){

						skybox.Bind();
						if (mat_ref->Bind()) {
							CameraBase temp_camera(projection_matrix, view_matrix);
							mat_ref->UpdateUniforms(camera_position, projection_matrix, view_matrix);
							Renderer::DrawSkybox(skybox);
							mat_ref->UnBind();
						}
						skybox.UnBind();
					}
				}
			}
		}

		// RENDER DEBUG VIEW FOR OCTREE
		if (auto octree_ref = scene_ref->GetOctree().lock(); octree_ref && scene_ref->GetDisplayOctree()) {

			auto& debug_line_shader = Engine::Get().GetShaderLibrary().GetShader("Debug_Line_Draw");

			// Draw Octree
			if (debug_line_shader) {
				debug_line_shader->Bind();
				debug_line_shader->SetVec4("u_LineColor", { 1.0f, 0.0f, 0.0f, 1.0f });
				debug_line_shader->SetMat4("u_VertexIn.Proj", projection_matrix);
				debug_line_shader->SetMat4("u_VertexIn.View", view_matrix);
				debug_line_shader->SetBool("u_UseInstanceData", true);
			}

			Renderer::DrawInstancedDebugCube(octree_ref->GetAllOctreeBoundsMat4());

			if (debug_line_shader) {
				debug_line_shader->Bind();
				debug_line_shader->SetVec4("u_LineColor", { 0.0f, 1.0f, 0.0f, 1.0f });
				debug_line_shader->SetMat4("u_VertexIn.Proj", projection_matrix);
				debug_line_shader->SetMat4("u_VertexIn.View", view_matrix);
				debug_line_shader->SetBool("u_UseInstanceData", true);
			}

			// Draw All Bounds of Data Sources in Octree
			static std::vector<glm::mat4> bounds_matricies{};
			if(bounds_matricies.capacity() == 0) bounds_matricies.reserve(1024);
			bounds_matricies.clear();

			for (auto& entity : FP_Data.RenderableEntities) {

				if (!scene_ref->ValidEntity(entity)) continue;

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

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

		if (!FP_Data.Screen_Quad_VAO) {
			return;
		}

		std::shared_ptr<Shader> shader = Engine::Get().GetShaderLibrary().GetShader("FBO Texture Shader");
		if (shader)
		{
			shader->Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, scene_ref->GetSceneFrameBuffer()->GetTexture(FrameBufferTexture::ColourTexture));
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

	void DeferredPipeline::OnUpdate(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix) {

	}

	void DeferredPipeline::OnStartPipeline(std::shared_ptr<Louron::Scene> scene) {

	}

	void DeferredPipeline::OnStopPipeline() {

		Renderer::CleanupRenderData();
	}

#pragma endregion

}
