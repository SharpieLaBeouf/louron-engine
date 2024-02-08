#include "Scene.h"

#include "Mesh.h"
#include "Entity.h"
#include "Components.h"

#include "../Renderer/Renderer.h"
#include "../Renderer/RendererPipeline.h"

#include <glm/gtc/quaternion.hpp>

#include <imgui/imgui.h>

#include <iomanip>

// Scene Management
namespace Louron {
	Scene::Scene(const std::string& sceneName, std::shared_ptr<RenderPipeline> pipeline) {
		m_SceneConfig->Name = sceneName;
		m_SceneConfig->AssetDirectory = "Assets/";
		m_SceneConfig->ScenePipeline = pipeline;
		m_SceneConfig->ResourceManager = std::make_shared<ResourceManager>();
	}

	// Creates Entity in Scene
	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<Transform>();

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

	bool Scene::HasEntity(const std::string& name)
	{
		return (FindEntityByName(name)) ? true : false;
	}

	void Scene::OnStart() {

		m_IsRunning = true;

		m_SceneConfig->ScenePipeline->OnStartPipeline();

	}
	
	void Scene::OnUpdate() {

		if (!m_IsPaused) {
			m_SceneConfig->ScenePipeline->OnUpdate(this);
		}
	}

	void Scene::OnUpdateGUI() {



	}

	void Scene::OnStop() {

		m_IsRunning = false;

		m_SceneConfig->ScenePipeline->OnStopPipeline();
	}
}