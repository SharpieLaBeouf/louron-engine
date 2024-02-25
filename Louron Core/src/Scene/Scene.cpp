#include "Scene.h"

// Louron Core Headers
#include "Entity.h"
#include "Scene Serializer.h"

#include "Components/Mesh.h"
#include "Components/Components.h"

#include "../Renderer/Renderer.h"

// C++ Standard Library Headers
#include <iomanip>

// External Vendor Library Headers
#include <glm/gtc/quaternion.hpp>

#include <imgui/imgui.h>

namespace Louron {

	Scene::Scene() {
		m_SceneFilePath = "Scenes/Untitled Scene.lscene";

		m_SceneConfig.Name = m_SceneFilePath.filename().replace_extension().string();
		m_SceneConfig.AssetDirectory = "Assets/";
		m_SceneConfig.ScenePipeline = std::make_shared<RenderPipeline>();
		m_SceneConfig.SceneResourceManager = std::make_shared<ResourceManager>();

		m_SceneConfig.ScenePipelineType = L_RENDER_PIPELINE::FORWARD;

	}

	Scene::Scene(const std::filesystem::path& sceneFilePath, L_RENDER_PIPELINE pipelineType) {

		std::filesystem::path outFilePath = sceneFilePath;

		// Check if Scene File Path is Empty.
		if (outFilePath.empty()) {
			// TODO: Add Log Here
			outFilePath = "Scenes/Untitled Scene.lscene";
		}

		// Check if Scene File Extension is Incompatible.
		if (outFilePath.extension() != ".lscene") {
			// TODO: Add Log Here
			outFilePath.replace_extension();
			outFilePath = outFilePath.string() + ".lscene";
		}

		// Load Existing Scene File or Create New Scene.
		if (std::filesystem::exists(outFilePath)) {

			std::shared_ptr<Scene> scene = std::make_shared<Scene>();

			SceneSerializer serializer(scene);
			if (serializer.Deserialize(outFilePath)) {
				m_SceneFilePath = outFilePath;
				m_SceneConfig = std::move(scene->m_SceneConfig);
				CopyRegistry(scene);

				return;
			}

			// TODO: Add Log Here - Could Not Load Scene File.
		}

		// If Existing Scene File Load Unsuccessful, Set Default Values
		m_SceneFilePath = outFilePath;

		m_SceneConfig.Name = outFilePath.filename().replace_extension().string();
		m_SceneConfig.AssetDirectory = "Assets/";
		m_SceneConfig.ScenePipelineType = pipelineType;
		m_SceneConfig.SceneResourceManager = std::make_shared<ResourceManager>();

		switch (pipelineType) {
			case L_RENDER_PIPELINE::FORWARD:
				m_SceneConfig.ScenePipeline = std::make_shared<RenderPipeline>();
			break;
			case L_RENDER_PIPELINE::FORWARD_PLUS:
				m_SceneConfig.ScenePipeline = std::make_shared<ForwardPlusPipeline>();
			break;
			case L_RENDER_PIPELINE::DEFERRED:
				m_SceneConfig.ScenePipeline = std::make_shared<DeferredPipeline>();
			break;
		}

	}

	/// <summary>
	/// Creates Entity in Scene and Generates New UUID
	/// </summary>
	Entity Scene::CreateEntity(const std::string& name) {
		return CreateEntity(UUID(), name);
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	/// <summary>
	/// Copy Constructor and Operator Deleted in ENTT for Registry.
	/// Have to Manually Copy Over Data.
	/// </summary>
	/// <param name="otherScene"></param>
	/// <returns></returns>
	bool Scene::CopyRegistry(std::shared_ptr<Scene> otherScene)
	{
		auto& srcSceneRegistry = otherScene->m_Registry;
		auto& dstSceneRegistry = m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = CreateEntity(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);
		return true;
	}

	/// <summary>
	/// Create Entity in Scene with UUID
	/// </summary>
	Entity Scene::CreateEntity(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<Transform>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Untitled Entity" : name;

		m_EntityMap[uuid] = entity;

		return entity;
	}

	// TODO: Duplicates Entity in Scene
	Entity Scene::DuplicateEntity(Entity entity) {
		return Entity();
	}

	// Destroys Entity in Scene
	void Scene::DestroyEntity(Entity entity) {
		m_EntityMap.erase(entity.GetUUID());
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

		L_CORE_WARN("Scene Does Not Have an Entity Named: {0}", name);
		return {};
	}

	Entity Scene::FindEntityByUUID(UUID uuid)
	{
		L_CORE_ASSERT(m_EntityMap.find(uuid) != m_EntityMap.end(), "Entity UUID not found in scene!");
		return { m_EntityMap.at(uuid), this };
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

		m_SceneConfig.ScenePipeline->OnStartPipeline(shared_from_this());
	}
	
	void Scene::OnUpdate() {

		if (!m_IsPaused) {
			m_SceneConfig.ScenePipeline->OnUpdate();
		}
	}

	void Scene::OnUpdateGUI() {



	}

	void Scene::OnStop() {

		m_IsRunning = false;

		m_SceneConfig.ScenePipeline->OnStopPipeline();
	}
}