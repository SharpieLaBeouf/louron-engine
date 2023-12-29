#include "Scene.h"
#include "Entity.h"

#include "Components.h"
#include "Light.h"
#include "MeshRenderer.h"
#include "Camera.h"

namespace Louron {
	
	// Creates Entity in Scene
	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	// Duplicates Entity in Scene
	Entity Scene::DuplicateEntity(Entity entity) {
		return Entity();
	}

	// Destroys Entity in Scene
	void Scene::DestroyEntity(Entity entity) {
		m_Registry.destroy(entity);
	}

	// Returns Entity within Scene on Tag Name
	Entity Scene::FindEntityByName(std::string_view name) {
		
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view) {
			const TagComponent& tag = view.get<TagComponent>(entity);
			if (tag.Tag == name)
				return Entity{ entity, this };
		}
		
		return {};
	}

	// Returns Primary Camera Entity
	Entity Scene::GetPrimaryCameraEntity() {

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			const CameraComponent& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}

		return {};
	}

	void Scene::OnStart() {

		m_IsRunning = true;

	}
	
	// TODO: Forward+ Rendering
	
	// Updates all Entities within the Scene, this is where most of the rendering pipeline will take place.
	void Scene::OnUpdate() {

		if (!m_IsPaused) {

			// Get Primary Camera Data
			Camera* mainCamera = nullptr;
			glm::mat4 cameraTransform;
			{
				auto view = m_Registry.view<TransformComponent, CameraComponent>();
				for (auto entity : view) {
					auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

					if (camera.Primary) {
						mainCamera = camera.Camera;
						cameraTransform = transform.GetTransform();
						break;
					}
				}
			}

			if (mainCamera) {

				// Call Renderer for all Meshes
				{
					auto view = m_Registry.view<TransformComponent, MeshRendererComponent>();

					OldLight tempLight;
					tempLight.position = { 0.0f, 10.0f, 0.0f };

					for (auto entity : view) {
						auto [transform, mesh] = view.get<TransformComponent, MeshRendererComponent>(entity);

						mesh.renderEntireMesh(mainCamera, &tempLight);
					}
				}

			}

		}

	}

	void Scene::OnStop() {

		m_IsRunning = false;

	}
}