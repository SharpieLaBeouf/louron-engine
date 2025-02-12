#include "RendererPipeline.h"

// Louron Core Headers
#include "Renderer.h"

#include "../Asset/Asset Manager API.h"

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
			GLuint shadowLightIndex = 0;

			GLfloat m_Padding1 = 0.0f;

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

		L_PROFILE_SCOPE("Forward Plus - Overall");

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
			FP_Data.PLEntitiesInFrustum.clear();
			FP_Data.SLEntitiesInFrustum.clear();
			FP_Data.DLEntities.clear();
			ConductLightFrustumCull();

			// Gather All Meshes Visible in Camera Frustum
			ConductRenderableFrustumCull(camera_position, projection_matrix);

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
							if (!entity.HasComponent<MeshFilterComponent>() || !entity.HasComponent<MeshRendererComponent>()) {
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
					auto mesh_view = oct_scene_ref->GetAllEntitiesWith<MeshRendererComponent, MeshFilterComponent>();
					for (const auto& entity_handle : mesh_view) {

						auto& component = mesh_view.get<MeshFilterComponent>(entity_handle);

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
			L_PROFILE_SCOPE("Forward Plus - Total Rendering");

			ConductShadowMapping(camera_position, projection_matrix, view_matrix);

			UpdateSSBOData();

			// Bind FBO and clear color and depth buffers for the new frame
			scene_ref->GetSceneFrameBuffer()->Bind();
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				
			glPolygonMode(GL_FRONT_AND_BACK, FP_Data.Debug_ShowWireframe ? GL_LINE : GL_FILL);

			ConductDepthPass(camera_position, projection_matrix, view_matrix);

			// Do After Depth Pass - everything in frustum should be drawn and queried in 
			// depth pass, we use query information from depth pass to drive occlusion 
			// culling for colour pass.
			ConductRenderableOcclusionCull();

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

		// Shadow SSBO
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Shadow_LightSpaceMatrix_Buffer); // SPOT
		glBufferData(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Shadow_Max_Maps * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Shadow_LightSpaceMatrix_Buffer); // DIRECTIONAL
		glBufferData(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Shadow_Max_Maps * sizeof(glm::mat4) * 5, nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// POINT SHADOWS - Cube Map Array
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

		// POINT SHADOWS - Frame Buffer
		glGenFramebuffers(1, &FP_Data.PL_Shadow_FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.PL_Shadow_FrameBuffer);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FP_Data.PL_Shadow_CubeMap_Array, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// SPOT SHADOWS - Texture Array
		glGenTextures(1, &FP_Data.SL_Shadow_Texture_Array);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, FP_Data.SL_Shadow_Texture_Array);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, FP_Data.SL_Shadow_Map_Res, FP_Data.SL_Shadow_Map_Res, FP_Data.SL_Shadow_Max_Maps);

		// SPOT SHADOWS - Frame Buffer
		glGenFramebuffers(1, &FP_Data.SL_Shadow_FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.SL_Shadow_FrameBuffer);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FP_Data.SL_Shadow_Texture_Array, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// DIRECTIONAL SHADOWS - Texture Array
		glGenTextures(1, &FP_Data.DL_Shadow_Texture_Array);
		glBindTexture(GL_TEXTURE_2D_ARRAY, FP_Data.DL_Shadow_Texture_Array);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, FP_Data.DL_Shadow_Map_Res, FP_Data.DL_Shadow_Map_Res, FP_Data.DL_Shadow_Max_Maps * 5); // Max 25 shadow maps, 5 cascades per light, max 5 shadow directional lights

		// DIRECTIONAL SHADOWS - Frame Buffer
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

		FP_Data.EntityOcclusionQueries = {};
		FP_Data.RenderableEntitiesInFrustum.reserve(1024);
		FP_Data.OctreeEntitiesInCamera.reserve(1024);

		FP_Data.PLEntitiesInFrustum.reserve(MAX_POINT_LIGHTS);
		FP_Data.SLEntitiesInFrustum.reserve(MAX_SPOT_LIGHTS);
		FP_Data.DLEntities.reserve(MAX_DIRECTIONAL_LIGHTS);

		FP_Data.OpaqueRenderables = {};
		FP_Data.TransparentRenderables = {};
	}

	/// <summary>
	/// Reset OpenGL state configuration required by renderer and clean FP_Data and Light SSBOs.
	/// </summary>
	void ForwardPlusPipeline::OnStopPipeline() {
		
		if (FP_Data.OctreeUpdateThread.joinable())
			FP_Data.OctreeUpdateThread.join();

		if (FP_Data.RenderQueueSortingThread.joinable())
			FP_Data.RenderQueueSortingThread.join();

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		
		glDeleteBuffers(1, &FP_Data.PL_Buffer);
		glDeleteBuffers(1, &FP_Data.PL_Indices_Buffer);

		glDeleteBuffers(1, &FP_Data.SL_Buffer);
		glDeleteBuffers(1, &FP_Data.SL_Indices_Buffer);
		glDeleteBuffers(1, &FP_Data.SL_Shadow_LightSpaceMatrix_Buffer);

		glDeleteBuffers(1, &FP_Data.DL_Buffer);
		glDeleteBuffers(1, &FP_Data.DL_Shadow_LightSpaceMatrix_Buffer);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &FP_Data.PL_Shadow_FrameBuffer);
		glDeleteTextures(1, &FP_Data.PL_Shadow_CubeMap_Array);

		glDeleteFramebuffers(1, &FP_Data.SL_Shadow_FrameBuffer);
		glDeleteTextures(1, &FP_Data.SL_Shadow_Texture_Array);

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
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_POINT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_SPOT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

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
				for (auto& entity : FP_Data.PLEntitiesInFrustum) {
					
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
				for (auto& entity : FP_Data.SLEntitiesInFrustum) {

					if (s_SpotLightVector.size() >= MAX_POINT_LIGHTS)
						break;

					auto& spot_light = entity.GetComponent<SpotLightComponent>();
					if (spot_light.Active)
					{
						s_SpotLightVector.push_back({ spot_light, entity.GetComponent<TransformComponent>() });

						if (FP_Data.SL_Shadow_LightIndexMap.find(entity.GetUUID()) != FP_Data.SL_Shadow_LightIndexMap.end()) {
							s_SpotLightVector.back().shadowLightIndex = FP_Data.SL_Shadow_LightIndexMap.at(entity.GetUUID());
						}
					}
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

			if (FP_Data.PLEntitiesInFrustum.size() >= MAX_POINT_LIGHTS)
				break;

			if (!pl_view.get<PointLightComponent>(entity_handle).Active)
				continue;

			Entity entity = { entity_handle, scene_ref.get() };
			if (IsSphereInsideFrustum({ entity.GetComponent<TransformComponent>().GetGlobalPosition(), entity.GetComponent<PointLightComponent>().Radius }, FP_Data.Camera_Frustum)) {
				FP_Data.PLEntitiesInFrustum.push_back(entity);
			}

		}

		auto sl_view = scene_ref->GetAllEntitiesWith<SpotLightComponent>();
		for (const auto& entity_handle : sl_view) {

			if (FP_Data.SLEntitiesInFrustum.size() >= MAX_SPOT_LIGHTS)
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
				FP_Data.SLEntitiesInFrustum.push_back(entity);
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
	void ForwardPlusPipeline::ConductRenderableFrustumCull(const glm::vec3& camera_position, const glm::mat4& projection_matrix) {

		L_PROFILE_SCOPE("Forward Plus - Frustum Culling Octree Query");

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

		size_t entity_counter{};
		if (auto oct_ref = scene_ref->GetOctree().lock(); oct_ref) {

			std::unique_lock lock(oct_ref->GetOctreeMutex());
			const auto& query_vec = oct_ref->Query(FP_Data.Camera_Frustum);

			entity_counter = oct_ref->TotalCount();

			if (query_vec.size() > FP_Data.OctreeEntitiesInCamera.capacity())
				FP_Data.OctreeEntitiesInCamera.reserve(FP_Data.OctreeEntitiesInCamera.capacity() * 2);
			
			FP_Data.OctreeEntitiesInCamera.insert(FP_Data.OctreeEntitiesInCamera.begin(), query_vec.begin(), query_vec.end());
		}

		std::unique_lock lock(FP_Data.RenderSortingMutex);

		// Transfer Data Over because we don't want 
		// to hold the shared_ptr's to OctreeData
		FP_Data.RenderableEntitiesInFrustum.clear();

		if (FP_Data.OctreeEntitiesInCamera.size() > FP_Data.RenderableEntitiesInFrustum.capacity())
			FP_Data.RenderableEntitiesInFrustum.reserve(FP_Data.RenderableEntitiesInFrustum.capacity() * 2);

		for (const auto& data : FP_Data.OctreeEntitiesInCamera) 
		{
			if (!data->Data)
				continue;

			auto& component = data->Data.GetComponent<MeshRendererComponent>();
			if (component.Active)
			{
				FP_Data.RenderableEntitiesInFrustum.push_back(data->Data);
			}
		}

		{
			L_PROFILE_SCOPE("Forward Plus - LOD Sorting");

			float A = projection_matrix[2][2];
			float B = projection_matrix[3][2];
			float far_plane = B / (A + 1.0f);

			auto view = scene_ref->GetAllEntitiesWith<LODMeshComponent>();
			for (auto& entity_handle : view)
			{
				Entity lod_entity = { entity_handle, scene_ref.get() };
				auto& lod_component = lod_entity.GetComponent<LODMeshComponent>();

				glm::vec3 position = lod_entity.GetComponent<TransformComponent>().GetGlobalPosition();
				float distance_to_lod_entity = glm::distance(camera_position, position);

				// Normalise the distance within the frustum (0.0 = near plane, 1.0 = far plane)
				float max_distance = (lod_component.MaxDistanceOverFarPlane) ? lod_component.MaxDistance : far_plane;
				float normalised_distance = distance_to_lod_entity / max_distance;

				// Find the correct LOD level to keep based on normalised distance
				int keep_lod_index = -1;
				for (int i = 0; i < lod_component.LOD_Elements.size(); i++)
				{
					if (normalised_distance <= lod_component.LOD_Elements[i].DistanceThresholdNormalised)
					{
						keep_lod_index = i;
						break;
					}
				}

				for (int i = 0; i < lod_component.LOD_Elements.size(); i++)
				{
					if (i == keep_lod_index)
						continue;

					for (const auto& entity_handle : lod_component.LOD_Elements[i].MeshRendererEntities)
					{
						if (entity_handle == NULL_UUID)
							continue;

						FP_Data.RenderableEntitiesInFrustum.erase(std::remove(FP_Data.RenderableEntitiesInFrustum.begin(), FP_Data.RenderableEntitiesInFrustum.end(), scene_ref->FindEntityByUUID(entity_handle)), FP_Data.RenderableEntitiesInFrustum.end());
					}
				}

			}
		}

		FP_Data.OctreeEntitiesInCamera.clear();
		Renderer::s_RenderStats.Entities_Culled_Frustum = static_cast<GLuint>(entity_counter - FP_Data.RenderableEntitiesInFrustum.size());		
	}

	static std::unordered_map<UUID, uint8_t> s_visible_history{};
	void ForwardPlusPipeline::ConductRenderableOcclusionCull()
	{
		L_PROFILE_SCOPE("Forward Plus - Occlusion Culling");

		auto scene_ref = m_Scene.lock();

		if (!scene_ref) {
			L_CORE_ERROR("Invalid Scene!");
			return;
		}

		std::unique_lock lock(FP_Data.RenderSortingMutex);

		
		// Occlusion Culling Checks
		size_t entity_counter = FP_Data.RenderableEntitiesInFrustum.size();
		for (auto it = FP_Data.EntityOcclusionQueries.begin(); it != FP_Data.EntityOcclusionQueries.end();)
		{
			Entity entity = scene_ref->FindEntityByUUID(it->first);
			if (!entity)
			{
				++it;
				continue;
			}

			Query& query = it->second;

			bool result_available = query.IsResultAvailable();
			if (!result_available && s_visible_history[it->first] > 0)
			{
				s_visible_history[it->first] -= 1;
				++it;
				continue;
			}

			bool is_visible = query.GetResult() != GL_FALSE;
			s_visible_history[it->first] = (is_visible) ? 5 : 0; // Result stays visible for atleast 5 frames

			auto find_in_frustum_culled = std::find_if(
				FP_Data.RenderableEntitiesInFrustum.begin(),
				FP_Data.RenderableEntitiesInFrustum.end(),
				[&](Entity& entity_find) { return entity_find == entity; });

			if (find_in_frustum_culled != FP_Data.RenderableEntitiesInFrustum.end())
			{
				if (!is_visible)  // Entity is occluded
					FP_Data.RenderableEntitiesInFrustum.erase(find_in_frustum_culled);
			}
			else
			{
				it = FP_Data.EntityOcclusionQueries.erase(it);
				s_visible_history.erase(it->first);
				continue;
			}
			++it;
		}

		Renderer::s_RenderStats.Entities_Culled_Occlusion = static_cast<GLuint>(entity_counter - FP_Data.RenderableEntitiesInFrustum.size());
		Renderer::s_RenderStats.Entities_Culled_Remaining = static_cast<GLuint>(FP_Data.RenderableEntitiesInFrustum.size());
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

		std::unique_lock lock(FP_Data.RenderSortingMutex);

		scene_ref->GetSceneFrameBuffer()->ClearEntityPixelData(NULL_UUID);

		{
			L_PROFILE_SCOPE("Forward Plus - Depth Pass::Sorting");

			FP_Data.DepthRenderables.clear();

			if (FP_Data.RenderableEntitiesInFrustum.empty())
				return;

			for (auto& entity : FP_Data.RenderableEntitiesInFrustum)
			{
				if (!scene_ref->ValidEntity(entity)) continue;

				const glm::vec3& objectPosition = entity.GetComponent<TransformComponent>().GetGlobalPosition();
				const Bounds_AABB& object_bounds = entity.GetComponent<MeshFilterComponent>().TransformedAABB;

				// Find distance from closest point of AABB from camera_position
				float distance = glm::length(camera_position - object_bounds.ClosestPoint(camera_position));

				auto& material_vector = entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles;
				if (material_vector.empty())
					continue;

				FP_Data.DepthRenderables.emplace_back(distance, entity.GetUUID());
			}

			// Front-to-Back sorting
			std::sort(FP_Data.DepthRenderables.begin(), FP_Data.DepthRenderables.end(), [](const auto& a, const auto& b) {
				return std::get<0>(a) < std::get<0>(b);
			});
		}

		if (!FP_Data.DepthRenderables.empty()) 
		{
			L_PROFILE_SCOPE("Forward Plus - Depth Pass::Rendering");

			if (auto shader = AssetManager::GetInbuiltShader("FP_Depth"); shader)
			{
				shader->Bind();
				scene_ref->GetSceneFrameBuffer()->BindEntitySSBO();
				shader->SetMat4("u_Proj", projection_matrix);
				shader->SetMat4("u_View", view_matrix);

				float A = projection_matrix[2][2];
				float B = projection_matrix[3][2];
				float near_plane = B / (A - 1.0f);
				float far_plane = B / (A + 1.0f);
				shader->SetFloat("u_Near", near_plane);
				shader->SetFloat("u_Far", far_plane);

				shader->SetIntVec2("u_ScreenSize", { scene_ref->GetSceneFrameBuffer()->GetConfig().Width, scene_ref->GetSceneFrameBuffer()->GetConfig().Height });

				for (auto& [distance, entity_uuid] : FP_Data.DepthRenderables) 
				{
					Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
					if (!entity) continue;

					shader->SetMat4("u_Model", scene_ref->FindEntityByUUID(entity_uuid).GetComponent<TransformComponent>().GetGlobalTransform());
					shader->SetUInt("u_EntityID", entity_uuid);

					auto& mesh_filter_component = entity.GetComponent<MeshFilterComponent>();
					auto asset_mesh_handle = mesh_filter_component.MeshFilterAssetHandle;

					// Check if Asset Handle is Valid
					if (!AssetManager::IsAssetHandleValid(asset_mesh_handle))
						continue;

					// Retrieve Cached Mesh Asset
					auto mesh_asset = FP_Data.CachedMeshAssets[asset_mesh_handle].lock();

					// Check if Loaded
					if (!mesh_asset)
					{
						// If Not Loaded, Call GetAsset to Load
						FP_Data.CachedMeshAssets[asset_mesh_handle] = AssetManager::GetAsset<AssetMesh>(asset_mesh_handle);
						mesh_asset = FP_Data.CachedMeshAssets[asset_mesh_handle].lock();
						
						// If Failed to Load - Continue
						if (!mesh_asset)
							continue;
					}

					auto& material_vector = entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles;
					if (material_vector.empty())
						continue;

					if (FP_Data.EntityOcclusionQueries.count(entity_uuid) == 0)
						FP_Data.EntityOcclusionQueries[entity_uuid] = Query(Query::Type::AnySamplesPassed);

					if (s_visible_history.count(entity_uuid) == 0)
						s_visible_history[entity_uuid] = 5;

					bool conduct_query = !FP_Data.EntityOcclusionQueries[entity_uuid].IsProcessing();
					if (conduct_query) 
						FP_Data.EntityOcclusionQueries[entity_uuid].Begin();
					
					// Should I draw this to the depth map? How can I determine 
					// whether to draw this to the depth map or not?
					for (int i = 0; i < mesh_asset->SubMeshes.size(); i++)
					{
						auto asset_material = i < material_vector.size() ? AssetManager::GetAsset<PBRMaterial>(material_vector[i].first) : AssetManager::GetAsset<PBRMaterial>(material_vector.back().first);

						if (!asset_material || asset_material->GetRenderType() == RenderType::L_MATERIAL_TRANSPARENT)
							continue;

						Renderer::DrawSubMesh(mesh_asset->SubMeshes[i], true);
					}
										
					// Render AABB bounding box to determine visibility for occlusion query
					if (conduct_query)
						FP_Data.EntityOcclusionQueries[entity_uuid].End();

				}
				scene_ref->GetSceneFrameBuffer()->UnBindEntitySSBO();
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
		std::shared_ptr<Shader> lightCull = AssetManager::GetInbuiltShader("FP_Light_Culling", true);
		if (lightCull) {

			lightCull->Bind();

			lightCull->SetMat4("u_View", view_matrix);
			lightCull->SetMat4("u_Proj", projection_matrix);
			
			auto& size = scene_ref->GetSceneFrameBuffer()->GetConfig();
			lightCull->SetIntVec2("u_ScreenSize", glm::ivec2(size.Width, size.Height));

			// Bind depth to texture 3 so this does not interfere with any 
			// diffuse, normal, or specular textures used 
			// 
			// Texture binding 3 is dedicated for depth map
 
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, scene_ref->GetSceneFrameBuffer()->GetTexture(FrameBufferTexture::DepthTexture));
			lightCull->SetInt("u_Depth", 3);

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
		L_PROFILE_SCOPE("Forward Plus - Shadow Mapping Total");

		// Calculate every other frame
		static bool conduct_shadow_pass = false;
		conduct_shadow_pass = !conduct_shadow_pass;

		if (!conduct_shadow_pass)
			return;
		
		auto scene_ref = m_Scene.lock();
		if (!scene_ref)
			return;

		const glm::mat4& camera_proj_view = projection_matrix * view_matrix;

		#pragma region Directional Light Shadows

		std::vector<Entity> dl_shadow_casting_vec;
		std::vector<Entity> dl_shadow_renderable_entities;
		std::vector<glm::mat4> dl_shadow_light_space_matricies;

		FP_Data.DL_Shadow_LightSpaceMatrixIndex.clear();
		FP_Data.DL_Shadow_LightShadowCascadeDistances.clear();

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

		if(!dl_shadow_casting_vec.empty())
		{

			// 2. Calculate Light Space World Space AABB - Get Meshes Intersecting with AABB from Octree
			// TODO: Need to fix this because it is not including objects that are behind camera frustum that 
			// may cast shadow into frustum. Maybe we do this after generating the cascades and use the 
			// world space AABB of the light projection to find our meshes?
			Bounds_Sphere world_light_bounds;
			{
				L_PROFILE_SCOPE("Directional Shadow Mapping 2a. Calculate Light Space Bounds");

				world_light_bounds.BoundsCentre = camera_position;
				world_light_bounds.BoundsRadius = 250.0f; // 500 diameter

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

						auto& component = data->Data.GetComponent<MeshRendererComponent>();
						if (component.Active && component.CastShadows)
							dl_shadow_renderable_entities.push_back(data->Data);
					}
				}

			}

			// 3. Calculate Light Space Matricies Per Light Per Cascade - 40 x glm::mat4's is the max = MAX_DIRECTIONAL_LIGHTS * 4 cascades (per directional light)

			{
				L_PROFILE_SCOPE("Directional Shadow Mapping 3. Calculate Cascade Light Matricies");
				dl_shadow_light_space_matricies.reserve(dl_shadow_casting_vec.size() * 5);

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

				auto shader = AssetManager::GetInbuiltShader("FP_Shadow_Directional");
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

						std::shared_ptr<AssetMesh> asset_mesh = AssetManager::GetAsset<AssetMesh>(mesh_entity.GetComponent<MeshFilterComponent>().MeshFilterAssetHandle);

						if (asset_mesh)
						{
							for (auto& sub_mesh : asset_mesh->SubMeshes)
								Renderer::DrawSubMesh(sub_mesh);
						}
					}

				}

				shader->UnBind();

				glCullFace(GL_BACK);
			}
		}

		#pragma endregion

		#pragma region Spot Light Shadows

		std::vector<Entity> sl_shadow_casting_vec;
		std::vector<glm::mat4> sl_shadow_light_space_matricies;
		sl_shadow_light_space_matricies.reserve(30);
		std::unordered_map<UUID, std::vector<Entity>> sl_shadow_renderable_entities;

		FP_Data.SL_Shadow_LightIndexMap.clear();

		// 1. Gather Spot Lights

		{
			L_PROFILE_SCOPE("Spot Shadow Mapping 1. Gathering");
			sl_shadow_casting_vec.reserve(FP_Data.SLEntitiesInFrustum.size());
			for (auto& entity : FP_Data.SLEntitiesInFrustum)
				if (entity.GetComponent<SpotLightComponent>().ShadowFlag != ShadowTypeFlag::NoShadows)
				{
					if (sl_shadow_casting_vec.size() >= FP_Data.SL_Shadow_Max_Maps)
						break;

					sl_shadow_casting_vec.push_back(entity);
				}
		}

		// 2. Gather Meshes Inside Spot Light AABB's

		if(!sl_shadow_casting_vec.empty())
		{

			{
				L_PROFILE_SCOPE("Spot Shadow Mapping 2. Calculate ViewProj and Get Meshes in Frustum");
				for (auto& entity : sl_shadow_casting_vec) {

					if (!entity)
						continue;

					auto& transform = entity.GetComponent<TransformComponent>();

					glm::mat4 light_proj = glm::perspective(glm::radians(entity.GetComponent<SpotLightComponent>().Angle), 1.0f, 0.1f, entity.GetComponent<SpotLightComponent>().Range);
					glm::mat4 light_view = glm::lookAt(transform.GetGlobalPosition(), transform.GetGlobalPosition() + transform.GetForwardDirection(), glm::vec3(0.0f, 1.0f, 0.0f));

					sl_shadow_light_space_matricies.push_back(light_proj * light_view);

					Frustum spot_frustum = { sl_shadow_light_space_matricies.back() };

					if (auto oct_ref = scene_ref->GetOctree().lock(); oct_ref) {

						std::unique_lock lock(oct_ref->GetOctreeMutex());

						const auto& query_vec = oct_ref->Query(spot_frustum);

						auto& sl_renderable_entities = sl_shadow_renderable_entities[entity.GetUUID()];
						sl_renderable_entities.reserve(query_vec.size());

						for (const auto& data : query_vec)
						{
							if (!data->Data)
								continue;

							auto& component = data->Data.GetComponent<MeshRendererComponent>();
							if (component.Active && component.CastShadows)
								sl_renderable_entities.push_back(data->Data);
						}
					}

				}

			}


			// 3. Update Light Space Matrix SSBO Data

			{
				L_PROFILE_SCOPE("Spot Shadow Mapping 3. Updating Light Matrix SSBO Data");
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, FP_Data.SL_Shadow_LightSpaceMatrix_Buffer);

				// Update SSBO data with light data
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Shadow_LightSpaceMatrix_Buffer);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sl_shadow_light_space_matricies.size() * sizeof(glm::mat4), sl_shadow_light_space_matricies.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// 4. Render Shadow Map from Light Space Matrix ViewProj

			{
				L_PROFILE_SCOPE("Spot Shadow Mapping 4. Rendering Spot Shadow Maps");
				glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.SL_Shadow_FrameBuffer);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FP_Data.SL_Shadow_Texture_Array, 0);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);

				glCullFace(GL_FRONT);

				glViewport(0, 0, FP_Data.SL_Shadow_Map_Res, FP_Data.SL_Shadow_Map_Res);
				glClear(GL_DEPTH_BUFFER_BIT);

				auto shader = AssetManager::GetInbuiltShader("FP_Shadow_Spot");
				shader->Bind();

				for (int light_index = 0; light_index < sl_shadow_casting_vec.size(); ++light_index) {

					Entity entity = sl_shadow_casting_vec[light_index];
					if (!entity)
						continue;

					FP_Data.SL_Shadow_LightIndexMap[entity.GetUUID()] = light_index;
					shader->SetMat4("u_LightSpaceMatrix", sl_shadow_light_space_matricies[light_index]);
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FP_Data.SL_Shadow_Texture_Array, 0, light_index);

					for (auto& mesh_entity : sl_shadow_renderable_entities[entity.GetUUID()]) {

						if (!mesh_entity)
							continue;

						glm::mat4 transform = mesh_entity.GetComponent<TransformComponent>().GetGlobalTransform();
						shader->SetMat4("u_Model", transform);

						std::shared_ptr<AssetMesh> asset_mesh = AssetManager::GetAsset<AssetMesh>(mesh_entity.GetComponent<MeshFilterComponent>().MeshFilterAssetHandle);
						for (auto& sub_mesh : asset_mesh->SubMeshes)
							Renderer::DrawSubMesh(sub_mesh);
					}

				}

				shader->UnBind();

				glCullFace(GL_BACK);
			}
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
			pl_shadow_casting_vec.reserve(FP_Data.PLEntitiesInFrustum.size());
			for (auto& entity : FP_Data.PLEntitiesInFrustum)
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

						auto& component = data->Data.GetComponent<MeshRendererComponent>();
						if (component.Active && component.CastShadows)
							entities_in_light.push_back(data->Data);
					}
				}

			}

		}

		// 2. Initialise and Draw Shadow CubeMap Array
		if (!pl_shadow_casting_vec.empty()) 
		{
			L_PROFILE_SCOPE("Point Shadow Mapping 2. Drawing");

			glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.PL_Shadow_FrameBuffer);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FP_Data.PL_Shadow_CubeMap_Array, 0);
			glDrawBuffer(GL_DEPTH_ATTACHMENT);
			glReadBuffer(GL_NONE);

			glCullFace(GL_FRONT);

			glViewport(0, 0, FP_Data.PL_Shadow_Map_Res, FP_Data.PL_Shadow_Map_Res);
			glClear(GL_DEPTH_BUFFER_BIT);

			auto shader = AssetManager::GetInbuiltShader("FP_Shadow_Point");
			shader->Bind();

			for (int lightIndex = 0; lightIndex < pl_shadow_casting_vec.size(); ++lightIndex) {

				glm::vec3 light_pos = pl_shadow_casting_vec[lightIndex].GetComponent<TransformComponent>().GetGlobalPosition();

				float near_plane = 0.1f;
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
				shader->SetFloatVec3("u_LightPosition", light_pos);
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

					std::shared_ptr<AssetMesh> asset_mesh = AssetManager::GetAsset<AssetMesh>(entity.GetComponent<MeshFilterComponent>().MeshFilterAssetHandle);
					for (auto& sub_mesh : asset_mesh->SubMeshes)
						Renderer::DrawSubMesh(sub_mesh);
				}
			}

			shader->UnBind();

			glCullFace(GL_BACK);
		}

		#pragma endregion

		if (scene_ref)
			scene_ref->GetSceneFrameBuffer()->Bind();
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

		//// Render Skybox First w/ No Depth Testing
		{
			L_PROFILE_SCOPE("Forward Plus - Render Pass::Skybox");

			auto skyboxView = scene_ref->GetAllEntitiesWith<CameraComponent, SkyboxComponent>();
			if (skyboxView.begin() != skyboxView.end()) {

				for (const auto& entity : skyboxView) {
					auto [scene_camera, skybox] = skyboxView.get<CameraComponent, SkyboxComponent>(entity);

					// Only draw the skybox for the primary camera
					if (scene_camera.Primary && scene_camera.ClearFlags == CameraClearFlags::SKYBOX) {

						if (auto mat_ref = AssetManager::GetAsset<SkyboxMaterial>(skybox.SkyboxMaterialAssetHandle); mat_ref) {

							skybox.Bind();

							// Save the current depth function
							GLenum originalDepthFunc{};
							glGetIntegerv(GL_DEPTH_FUNC, (GLint*)&originalDepthFunc);

							// Change depth function to GL_ALWAYS for the skybox
							glDepthFunc(GL_ALWAYS);

							if (mat_ref->Bind())
							{
								mat_ref->UpdateUniforms(camera_position, projection_matrix, view_matrix);
								Renderer::DrawSkybox(skybox);
								mat_ref->UnBind();
							}
							skybox.UnBind();

							// Restore the original depth function after the skybox is rendered
							glDepthFunc(originalDepthFunc);
						}
					}
				}
			}
		}

		// Wait for Sorting to Finish
		if (FP_Data.RenderQueueSortingThread.joinable()) {
			L_PROFILE_SCOPE("Forward Plus - Render Pass::Renderable Sorting Thread Wait");
			FP_Data.RenderQueueSortingThread.join();
		}
		
		// Rendering
		float A = projection_matrix[2][2];
		float B = projection_matrix[3][2];
		float near_plane = B / (A - 1.0f);
		float far_plane = B / (A + 1.0f);

		// Lets colour in some triangles!
		if (!FP_Data.OpaqueRenderables.empty()) 
		{
			L_PROFILE_SCOPE("Forward Plus - Render Pass::Opaque Pass");
			for (const auto& [material_wrapper_pair, mesh_map] : FP_Data.OpaqueRenderables) 
			{

				const auto& material_asset = material_wrapper_pair.material;

				if (!material_asset)
					continue;

				if (!material_asset->Bind())
					continue;

				auto shader = material_asset->GetShader();
				if (shader->IsValid())
				{
					material_asset->UpdateUniforms(camera_position, projection_matrix, view_matrix, material_wrapper_pair.uniform_block); // Change

					// Update Specific Forward Plus Uniforms
					shader->SetInt("u_TilesX", FP_Data.workGroupsX);
					shader->SetInt("u_ShowLightComplexity", FP_Data.Debug_ShowLightComplexity);

					shader->SetFloat("u_Near", near_plane);
					shader->SetFloat("u_Far", far_plane);

					shader->SetIntVec2("u_ScreenSize", { scene_ref->GetSceneFrameBuffer()->GetConfig().Width, scene_ref->GetSceneFrameBuffer()->GetConfig().Height });

					if (scene_ref->GetSceneFrameBuffer()->IsMultiSampled())
					{
						shader->SetInt("u_Samples", scene_ref->GetSceneFrameBuffer()->GetConfig().Samples);
						glActiveTexture(GL_TEXTURE3);
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, scene_ref->GetSceneFrameBuffer()->GetMultiSampledTexture(FrameBufferTexture::DepthTexture));
						shader->SetInt("u_Depth_MS", 3);
					}
					else
					{
						shader->SetInt("u_Samples", scene_ref->GetSceneFrameBuffer()->GetConfig().Samples);
						glActiveTexture(GL_TEXTURE3);
						glBindTexture(GL_TEXTURE_2D, scene_ref->GetSceneFrameBuffer()->GetTexture(FrameBufferTexture::DepthTexture));
						shader->SetInt("u_Depth", 3);
					}

					// Texture binding 4 is dedicated for Point Light Shadow Cube Map Array
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, FP_Data.PL_Shadow_CubeMap_Array);
					shader->SetInt("u_PL_ShadowCubeMapArray", 4);

					// Texture binding 5 is dedicated for Directional Light Shadow Texture Array
					glActiveTexture(GL_TEXTURE5);
					glBindTexture(GL_TEXTURE_2D_ARRAY, FP_Data.DL_Shadow_Texture_Array);
					shader->SetInt("u_DL_ShadowMapArray", 5);

					// Texture binding 6 is dedicated for Spot Light Shadow Texture Array
					glActiveTexture(GL_TEXTURE6);
					glBindTexture(GL_TEXTURE_2D_ARRAY, FP_Data.SL_Shadow_Texture_Array);
					shader->SetInt("u_SL_ShadowMapArray", 6);

				}
				else
				{
					shader = AssetManager::GetInbuiltShader("Invalid Shader");
					shader->Bind();

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, AssetManager::GetInbuiltAsset<Texture>("Invalid Checkered Texture")->GetID());
					shader->SetInt("u_InvalidTexture", 0);
					shader->SetMat4("u_VertexIn.Proj", projection_matrix);
					shader->SetMat4("u_VertexIn.View", view_matrix);
				}

				for (const auto& [sub_mesh, entities] : mesh_map) 
				{

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
					else 
					{
						const auto& transform = scene_ref->FindEntityByUUID(entities[0]).GetComponent<TransformComponent>().GetGlobalTransform();
						shader->SetMat4("u_VertexIn.Model", transform);
						Renderer::DrawSubMesh(sub_mesh);
					}
				}
			}
		}

		if (!FP_Data.TransparentRenderables.empty())
		{
			L_PROFILE_SCOPE("Forward Plus - Render Pass::Transparent Pass");

			// Disable Depth Writing During Transparent Pass
			// Depth Already Written During Depth/Colour Pass
			// We do not want to override during colour pass...
			glDepthMask(GL_FALSE);
			glDisable(GL_CULL_FACE);

			// Enable Blending During Transparency Pass
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Render Transparent Objects Back to Front One By One....
			for (const auto& [distance, material_wrapper_pair, sub_mesh, entity_uuid] : FP_Data.TransparentRenderables)
			{
				Entity ent = scene_ref->FindEntityByUUID(entity_uuid);
				if (!ent) continue;

				const auto& material_asset = material_wrapper_pair.material;

				if (!material_asset)
					continue;

				if (!material_asset->Bind())
					continue;

				auto shader = material_asset->GetShader();
				if (shader->IsValid())
				{
					material_asset->UpdateUniforms(camera_position, projection_matrix, view_matrix, material_wrapper_pair.uniform_block); // Change

					// Update Specific Forward Plus Uniforms
					shader->SetInt("u_TilesX", FP_Data.workGroupsX);
					shader->SetInt("u_ShowLightComplexity", FP_Data.Debug_ShowLightComplexity);

					shader->SetFloat("u_Near", near_plane);
					shader->SetFloat("u_Far", far_plane);

					shader->SetIntVec2("u_ScreenSize", { scene_ref->GetSceneFrameBuffer()->GetConfig().Width, scene_ref->GetSceneFrameBuffer()->GetConfig().Height });

					// Texture binding 3 is dedicated for depth map
					if (scene_ref->GetSceneFrameBuffer()->IsMultiSampled())
					{
						shader->SetInt("u_Samples", scene_ref->GetSceneFrameBuffer()->GetConfig().Samples);
						glActiveTexture(GL_TEXTURE3);
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, scene_ref->GetSceneFrameBuffer()->GetMultiSampledTexture(FrameBufferTexture::DepthTexture));
						shader->SetInt("u_Depth_MS", 3);
					}
					else
					{
						shader->SetInt("u_Samples", scene_ref->GetSceneFrameBuffer()->GetConfig().Samples);
						glActiveTexture(GL_TEXTURE3);
						glBindTexture(GL_TEXTURE_2D, scene_ref->GetSceneFrameBuffer()->GetTexture(FrameBufferTexture::DepthTexture));
						shader->SetInt("u_Depth", 3);
					}

					// Texture binding 4 is dedicated for Point Light Shadow Cube Map Array
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, FP_Data.PL_Shadow_CubeMap_Array);
					shader->SetInt("u_PL_ShadowCubeMapArray", 4);

					// Texture binding 5 is dedicated for Directional Light Shadow Texture Array
					glActiveTexture(GL_TEXTURE5);
					glBindTexture(GL_TEXTURE_2D_ARRAY, FP_Data.DL_Shadow_Texture_Array);
					shader->SetInt("u_DL_ShadowMapArray", 5);

					// Texture binding 6 is dedicated for Spot Light Shadow Texture Array
					glActiveTexture(GL_TEXTURE6);
					glBindTexture(GL_TEXTURE_2D_ARRAY, FP_Data.SL_Shadow_Texture_Array);
					shader->SetInt("u_SL_ShadowMapArray", 6);

				}
				else
				{
					shader = AssetManager::GetInbuiltShader("Invalid Shader");
					shader->Bind();

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, AssetManager::GetInbuiltAsset<Texture>("Invalid Checkered Texture")->GetID());
					shader->SetInt("u_InvalidTexture", 0);
					shader->SetMat4("u_VertexIn.Proj", projection_matrix);
					shader->SetMat4("u_VertexIn.View", view_matrix);
				}

				shader->SetBool("u_UseInstanceData", false);

				const auto& transform = ent.GetComponent<TransformComponent>().GetGlobalTransform();
				shader->SetMat4("u_VertexIn.Model", transform);
				Renderer::DrawSubMesh(sub_mesh);
			}

			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			glDisable(GL_BLEND);
		}

		// Sorting
		FP_Data.RenderQueueSortingThread = std::thread([&]() -> void 
		{

			L_PROFILE_SCOPE("Forward Plus - Render Pass::Renderable Sorting");

			auto thread_scene_ref = m_Scene.lock();

			if (!thread_scene_ref) {
				L_CORE_ERROR("Invalid Scene!");
				return;
			}

			FP_Data.OpaqueRenderables.clear();
			FP_Data.TransparentRenderables.clear();
			FP_Data.Debug_RenderAABB.clear();

			std::unique_lock lock(FP_Data.RenderSortingMutex);
			if (FP_Data.RenderableEntitiesInFrustum.empty())
				return;

			for (auto& entity : FP_Data.RenderableEntitiesInFrustum)
			{
				if (!thread_scene_ref->ValidEntity(entity))
					continue;

				auto& mesh_filter_component = entity.GetComponent<MeshFilterComponent>();

				// Check if Asset Handle is Valid
				if (!AssetManager::IsAssetHandleValid(mesh_filter_component.MeshFilterAssetHandle))
					continue;

				// Retrieve Cached Mesh Asset
				auto mesh_asset = FP_Data.CachedMeshAssets[mesh_filter_component.MeshFilterAssetHandle].lock();

				// Check if Loaded
				if (!mesh_asset)
				{
					// If Not Loaded, Call GetAsset to Load
					FP_Data.CachedMeshAssets[mesh_filter_component.MeshFilterAssetHandle] = AssetManager::GetAsset<AssetMesh>(mesh_filter_component.MeshFilterAssetHandle);
					mesh_asset = FP_Data.CachedMeshAssets[mesh_filter_component.MeshFilterAssetHandle].lock();

					// If Failed to Load - Continue
					if (!mesh_asset)
						continue;
				}

				// Retrieve Sub Meshes
				auto& sub_meshes = mesh_asset->SubMeshes;

				// Retrieve All MeshMaterialHandles
				auto& material_handles = entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles;

				if (sub_meshes.empty() || material_handles.empty())
					continue;

				// MATERIAL AND MATERIAL UNIFORM BLOCK SORTING
				// Materials will be sorted based on their material, and the uniform 
				// block of an individual material on a MeshRendererComponent.
				int material_index = 0;
				for (int i = 0; i < sub_meshes.size(); ++i)
				{
					// Material Handle + Uniform Group in Mesh Renderer Component
					// Nullptr means there is no custom uniform block
					auto& mesh_renderer_material_pair = material_handles[material_index];

					// Retrieve Cached Mesh Asset
					auto material_asset = FP_Data.CachedMaterialAssets[mesh_renderer_material_pair.first].lock();

					// Check if Loaded
					if (!material_asset)
					{
						// If Not Loaded, Call GetAsset to Load
						FP_Data.CachedMaterialAssets[mesh_renderer_material_pair.first] = AssetManager::GetAsset<PBRMaterial>(mesh_renderer_material_pair.first);
						material_asset = FP_Data.CachedMaterialAssets[mesh_renderer_material_pair.first].lock();

						// If Failed to Load - Continue
						if (!material_asset)
							continue;
					}

					// Opaque Sorting
					if (material_asset->GetRenderType() == RenderType::L_MATERIAL_OPAQUE)
					{
						// Retrieve the Uniform Block Associated w/ This Mesh Renderer Material
						auto uniform_block = (mesh_renderer_material_pair.second) ? mesh_renderer_material_pair.second : material_asset->GetUniformBlock();

						// Key the Material Wrapper to Secure Placement in Opaque Queue
						auto& sub_mesh_map = FP_Data.OpaqueRenderables[{ material_asset, uniform_block}]; // Keys this to the opaque renderables

						// Key the Sub Mesh to Retrieve Vector of Entities in this Rendering State
						auto& entity_vector = sub_mesh_map[sub_meshes[i]];

						// If First - Set Allocation for 8 entities
						if (entity_vector.size() == 0 && entity_vector.capacity() == 0)
							entity_vector.reserve(8);

						// If we need to reallocate, double if capacity 
						// is under 64, if above, we will + 8 only
						if (entity_vector.size() >= entity_vector.capacity())
							entity_vector.reserve(entity_vector.size() < 64 ? entity_vector.capacity() * 2 : entity_vector.capacity() + 8);

						entity_vector.push_back(entity.GetUUID());
					}
					// Transparent Sorting
					else if (material_asset->GetRenderType() == RenderType::L_MATERIAL_TRANSPARENT)
					{
						const glm::vec3& objectPosition = entity.GetComponent<TransformComponent>().GetGlobalPosition();
						float distance = glm::length(objectPosition - camera_position);

						// If First - Set Allocation for 128 entities
						// This queue is not batched w/ multiple render
						// states or materials, these all need to be 
						// rendered back to front
						if (FP_Data.TransparentRenderables.size() == 0 && FP_Data.TransparentRenderables.capacity() == 0)
							FP_Data.TransparentRenderables.reserve(128);

						// If we need to reallocate, double if capacity is under 1024, if above, we will + 16 only
						if (FP_Data.TransparentRenderables.size() >= FP_Data.TransparentRenderables.capacity())
							FP_Data.TransparentRenderables.reserve(FP_Data.TransparentRenderables.size() < 1024 ? FP_Data.TransparentRenderables.capacity() * 2 : FP_Data.TransparentRenderables.capacity() + 16);

						// Emplace to Back of TransparentRenderQueue
						FP_Data.TransparentRenderables.emplace_back
						(
							distance,
							_MaterialWrapper{ material_asset, mesh_renderer_material_pair.second ? mesh_renderer_material_pair.second : material_asset->GetUniformBlock() },
							sub_meshes[i],
							entity.GetUUID()
						);
					}

					// Makes sure we don't exceed the maximum materials, if we do, then we will 
					// just continue using the last material in the Mesh Renderer materials vector
					if (material_index < material_handles.size() - 1)
						material_index++;
				}

				// Set Option for Debug Draw Cube for AABB
				if (mesh_filter_component.GetShouldDisplayDebugLines())
					FP_Data.Debug_RenderAABB.push_back(mesh_filter_component.TransformedAABB.GetGlobalBoundsMat4());
			}

			// Back-to-Front Sorting - Transparent Objects
			std::sort(FP_Data.TransparentRenderables.begin(), FP_Data.TransparentRenderables.end(), [](const auto& a, const auto& b) {
				return std::get<0>(a) > std::get<0>(b);
			});
		});

		// Debug Rendering
		{
			L_PROFILE_SCOPE("Forward Plus - Render Pass::Draw Debug Lines");

			static std::vector<glm::mat4> bounds_matricies{};
			if (bounds_matricies.capacity() == 0) 
				bounds_matricies.reserve(1024);
			else
				bounds_matricies.clear();

			// Octree Display Enabled
			if (auto octree_ref = scene_ref->GetOctree().lock(); octree_ref && scene_ref->GetDisplayOctree()) {

				// Draw Octree
				auto debug_line_shader = AssetManager::GetInbuiltShader("Debug_Line_Draw");
				if (debug_line_shader) {
					debug_line_shader->Bind();
					debug_line_shader->SetFloatVec4("u_LineColor", { 1.0f, 0.0f, 0.0f, 1.0f });
					debug_line_shader->SetMat4("u_VertexIn.Proj", projection_matrix);
					debug_line_shader->SetMat4("u_VertexIn.View", view_matrix);
					debug_line_shader->SetBool("u_UseInstanceData", true);

					Renderer::DrawInstancedDebugCube(octree_ref->GetAllOctreeBoundsMat4());

					debug_line_shader->Bind();
					debug_line_shader->SetFloatVec4("u_LineColor", { 0.0f, 1.0f, 0.0f, 1.0f });
					debug_line_shader->SetMat4("u_VertexIn.Proj", projection_matrix);
					debug_line_shader->SetMat4("u_VertexIn.View", view_matrix);
					debug_line_shader->SetBool("u_UseInstanceData", true);

					// Draw All Bounds of Data Sources in Octree
					for (auto& entity : FP_Data.RenderableEntitiesInFrustum) {

						if (!scene_ref->ValidEntity(entity)) continue;

						if (entity.HasComponent<MeshFilterComponent>())
							bounds_matricies.push_back(entity.GetComponent<MeshFilterComponent>().TransformedAABB.GetGlobalBoundsMat4());
					}

					Renderer::DrawInstancedDebugCube(bounds_matricies);

					debug_line_shader->UnBind();
				}
			}
			// Only Draw Debug MeshFilter AABB's
			else if (!FP_Data.Debug_RenderAABB.empty())
			{
				auto debug_line_shader = AssetManager::GetInbuiltShader("Debug_Line_Draw");
				if (debug_line_shader)
				{
					debug_line_shader->Bind();
					debug_line_shader->SetFloatVec4("u_LineColor", { 0.0f, 1.0f, 0.0f, 1.0f });
					debug_line_shader->SetMat4("u_VertexIn.Proj", projection_matrix);
					debug_line_shader->SetMat4("u_VertexIn.View", view_matrix);
					debug_line_shader->SetBool("u_UseInstanceData", true);

					Renderer::DrawInstancedDebugCube(FP_Data.Debug_RenderAABB);

					debug_line_shader->UnBind();
				}
			}
		}

		// Clean Up Scene Render Pass
		GLint maxTextureUnits = 7;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
		for (int i = 0; i < maxTextureUnits; i++) {
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

		auto shader = AssetManager::GetInbuiltShader("FBO Texture Shader");
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
