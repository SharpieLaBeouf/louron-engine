#include "Scene.h"

// Louron Core Headers
#include "Entity.h"
#include "Prefab.h"
#include "Scene Serializer.h"
#include "Resource Manager.h"

#include "Components/Camera.h"
#include "Components/Components.h"
#include "Components/Light.h"
#include "Components/Mesh.h"
#include "Components/UUID.h"
#include "Components/Skybox.h"

#include "Components/Physics/Collider.h"
#include "Components/Physics/Rigidbody.h"
#include "Components/Physics/PhysicsWrappers.h"

#include "Scene Systems/Physics System.h"

#include "../Debug/Profiler.h"

#include "../Renderer/Renderer.h"
#include "../Renderer/RendererPipeline.h"

#include "../Core/Time.h"
#include "../Core/Input.h"

// C++ Standard Library Headers
#include <iomanip>

// External Vendor Library Headers
#include <glm/gtc/quaternion.hpp>
#include <imgui/imgui.h>

namespace Louron {

#pragma region Initialisation and ECS

	static PxFilterFlags CustomFilterShader(
		PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize) {
		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND;
		return PxFilterFlag::eDEFAULT;
	}

	Scene::Scene() {
		// Create PhysX Scene
		PxSceneDesc sceneDesc(PxGetPhysics().getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
		sceneDesc.filterShader = CustomFilterShader;
		m_PhysxScene = PxGetPhysics().createScene(sceneDesc);

#ifdef _DEBUG
		PxPvdSceneClient* pvdClient = m_PhysxScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
#endif

		m_SceneFilePath = "Scenes/Untitled Scene.lscene";

		m_SceneConfig.Name = m_SceneFilePath.filename().replace_extension().string();
		m_SceneConfig.AssetDirectory = "Assets/";
		m_SceneConfig.ScenePipeline = std::make_shared<RenderPipeline>();
		m_SceneConfig.SceneResourceManager = std::make_shared<ResourceManager>();
		
		m_SceneConfig.ScenePipeline = std::make_shared<ForwardPlusPipeline>();
	}

	Scene::Scene(L_RENDER_PIPELINE pipeline) {
		// Create PhysX Scene
		PxSceneDesc sceneDesc(PxGetPhysics().getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
		sceneDesc.filterShader = CustomFilterShader;
		m_PhysxScene = PxGetPhysics().createScene(sceneDesc);

#ifdef _DEBUG
		PxPvdSceneClient* pvdClient = m_PhysxScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
#endif

		m_SceneFilePath = "Scenes/Untitled Scene.lscene";

		m_SceneConfig.Name = m_SceneFilePath.filename().replace_extension().string();
		m_SceneConfig.AssetDirectory = "Assets/";
		m_SceneConfig.ScenePipeline = std::make_shared<RenderPipeline>();
		m_SceneConfig.SceneResourceManager = std::make_shared<ResourceManager>();

		switch (pipeline) {
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

	Scene::~Scene() {


		m_PhysxScene->release();

	}

	/// <summary>
	/// Once the Scene has been initialised, call this to load the scene from file.
	/// </summary>
	/// <returns>Returns true if the SceneFile was loaded successfully, returns false if not.</returns>
	bool Scene::LoadSceneFile(const std::filesystem::path& sceneFilePath) {

		std::filesystem::path outFilePath = sceneFilePath;

		// Check if Scene File Path is Empty.
		if (outFilePath.empty()) {
			L_CORE_ERROR("Scene File Path Empty - Cannot Load Scene Data from File!");
			return false;
		}

		// Check if Scene File Extension is Incompatible.
		if (outFilePath.extension() != ".lscene") {
			L_CORE_ERROR("Scene File Path Extension Incompatible - Cannot Load Scene Data from File!");
			return false;
		}

		// Load Existing Scene File or Create New Scene.
		if (std::filesystem::exists(outFilePath)) {

			SceneSerializer serializer(std::static_pointer_cast<Scene>(shared_from_this()));
			if (serializer.Deserialize(outFilePath)) {
				m_SceneFilePath = outFilePath;
				return true;
			}

			L_CORE_ERROR("Scene File Path Does Not Exist - Cannot Load Scene Data from File!");
		}
		return false;
	}

	/// <summary>
	/// Creates Entity in Scene and Generates New UUID
	/// </summary>
	Entity Scene::CreateEntity(const std::string& name) {
		return CreateEntity(UUID(), name);
	}

	/// <summary>
	/// Create Entity in Scene with UUID
	/// </summary>
	Entity Scene::CreateEntity(UUID uuid, const std::string& name) {

		while (true) {

			if (m_EntityMap->find(uuid) == m_EntityMap->end())
				break;

			uuid = UUID();
		}

		Entity entity = { m_Registry.create(), this };

		// 1. Add UUID Component
		entity.AddComponent<IDComponent>(uuid);

		// 2. Add Transform Component
		entity.AddComponent<Transform>();

		// 3. Add Tag Component
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Untitled Entity" : name;

		// 4. Add Hierarchy Component
		entity.AddComponent<HierarchyComponent>();

		m_EntityMap->emplace(uuid, entity);

		return entity;
	}

	#pragma region Component Copying

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

	#pragma endregion

	/// <summary>
	/// Copy Constructor and Operator Deleted in ENTT for Registry.
	/// Have to Manually Copy Over Data.
	/// </summary>
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

	void Scene::SetPhysScene(PxScene* physScene) {

		// Release PhysX Scene
		if (m_PhysxScene) {
			m_PhysxScene->release();
			m_PhysxScene = physScene;
			return;
		}

		if (physScene) {
			m_PhysxScene = physScene;
			return;
		}

		L_CORE_WARN("Physics Scene Not Valid.");
		m_PhysxScene = nullptr;
	}


	// TODO: Duplicates Entity in Scene
	Entity Scene::DuplicateEntity(Entity entity) {
		return Entity();
	}

	// Destroys Entity in Scene
	void Scene::DestroyEntity(Entity entity) {

		// 1. Check for children and Destroy them too
		if (!entity) {
			L_CORE_WARN("Attempted to Destroy Null Entity.");
			return;
		}

		// Check If Entity is Part of the Scene
		if (!HasEntity(entity.GetUUID())) {
			L_CORE_WARN("Attempted to Destroy an Entity Not In The Scene.");
			return;
		}

		// Call Physics System Remove Methods
		PhysicsSystem::RemoveRigidBody(entity, this);

		if (entity.HasAnyComponent<SphereCollider, BoxCollider>()) {

			if (entity.HasComponent<SphereCollider>())
				PhysicsSystem::RemoveCollider(entity, this, PxGeometryType::eSPHERE);

			if (entity.HasComponent<BoxCollider>())
				PhysicsSystem::RemoveCollider(entity, this, PxGeometryType::eBOX);
		}

		// 2. Remove Entity from Scene

		// Remove the Entity from the Scene Entity Map
		m_EntityMap->erase(entity.GetUUID());

		// Destroy the Entity and Components from the ENTT Registry
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

		L_CORE_ERROR("Scene Does Not Have an Entity Named: {0}", name);
		return {};
	}

	Entity Scene::FindEntityByUUID(UUID uuid)
	{
		if (m_EntityMap->find(uuid) != m_EntityMap->end()) {
			return Entity{ m_EntityMap->at(uuid), this };
		}

		L_CORE_ERROR("Entity UUID not found in scene");
		return Entity{};
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
	bool Scene::HasEntity(const UUID& uuid)
	{
		return (FindEntityByUUID(uuid)) ? true : false;
	}

	Entity Scene::InstantiatePrefab(std::shared_ptr<Prefab> prefab, std::optional<Transform> transform, const UUID& parent_uuid)
	{
		entt::registry* prefab_registry = &prefab->m_PrefabRegistry;
		entt::entity prefab_root_entity = prefab->m_RootEntity;
		std::string prefab_name = prefab->m_PrefabName;

		std::function<Entity(entt::entity, const UUID&)> copy_prefab_entity = [&](entt::entity start_prefab_entity, const UUID& parent_uuid) -> Entity {

			Entity instantiated_entity = this->CreateEntity("");

			// 1. Copy Data in All Components
			{
				// 1.a. Tag Component
				if (prefab_registry->has<TagComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<TagComponent>(start_prefab_entity);
					instantiated_entity.GetComponent<TagComponent>().Tag = component.Tag;
				}

				// 1.b. Hierarchy Component
				if (prefab_registry->has<HierarchyComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<HierarchyComponent>(start_prefab_entity);
					instantiated_entity.GetComponent<HierarchyComponent>().AttachParent(parent_uuid);
				}

				// 1.c. Camera Component
				if (prefab_registry->has<CameraComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<CameraComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<CameraComponent>(component);
				}

				// 1.d. Audio Listener
				if (prefab_registry->has<AudioListener>(start_prefab_entity)) {
					auto& component = prefab_registry->get<AudioListener>(start_prefab_entity);
					instantiated_entity.AddComponent<AudioListener>(component);
				}

				// 1.e. Audio Emitter
				if (prefab_registry->has<AudioEmitter>(start_prefab_entity)) {
					auto& component = prefab_registry->get<AudioEmitter>(start_prefab_entity);
					instantiated_entity.AddComponent<AudioEmitter>(component);
				}

				// 1.f. Transform Component
				if (prefab_registry->has<Transform>(start_prefab_entity)) {
					auto& component = prefab_registry->get<Transform>(start_prefab_entity);
					instantiated_entity.RemoveComponent<Transform>(); // Remove so we can use copy constructor for transform
					
					if (parent_uuid == NULL_UUID) { 
						// If we are the root entity, we check if the transform 
						// passed has a value, if not, standard copy from prefab
						if (transform.has_value())
							instantiated_entity.AddComponent<Transform>(transform.value());
						else
							instantiated_entity.AddComponent<Transform>(component);
					}
					else {
						instantiated_entity.AddComponent<Transform>(component);
					}
				}

				// 1.g. MeshFilter
				if (prefab_registry->has<MeshFilter>(start_prefab_entity)) {
					auto& component = prefab_registry->get<MeshFilter>(start_prefab_entity);
					instantiated_entity.AddComponent<MeshFilter>(component);
				}

				// 1.h. MeshRenderer
				if (prefab_registry->has<MeshRenderer>(start_prefab_entity)) {
					auto& component = prefab_registry->get<MeshRenderer>(start_prefab_entity);
					instantiated_entity.AddComponent<MeshRenderer>(component);
				}

				// 1.i. PointLight Component
				if (prefab_registry->has<PointLightComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<PointLightComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<PointLightComponent>(component);
				}

				// 1.j. SpotLight Component
				if (prefab_registry->has<SpotLightComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<SpotLightComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<SpotLightComponent>(component);
				}

				// 1.k. DirectionalLight Component
				if (prefab_registry->has<DirectionalLightComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<DirectionalLightComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<DirectionalLightComponent>(component);
				}

				// 1.l. Skybox Component
				if (prefab_registry->has<SkyboxComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<SkyboxComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<SkyboxComponent>(component);
				}

				// 1.m. Rigidbody Component
				if (prefab_registry->has<Rigidbody>(start_prefab_entity)) {
					auto& component = prefab_registry->get<Rigidbody>(start_prefab_entity);
					instantiated_entity.AddComponent<Rigidbody>() = component;
				}

				// 1.n. Sphere Collider
				if (prefab_registry->has<SphereCollider>(start_prefab_entity)) {
					auto& component = prefab_registry->get<SphereCollider>(start_prefab_entity);
					instantiated_entity.AddComponent<SphereCollider>() = component;
				}

				// 1.o. Box Collider
				if (prefab_registry->has<BoxCollider>(start_prefab_entity)) {
					auto& component = prefab_registry->get<BoxCollider>(start_prefab_entity);
					instantiated_entity.AddComponent<BoxCollider>() = component;
				}
			}

			// 2. Recurse Children
			if (prefab_registry->has<HierarchyComponent>(start_prefab_entity)) {

				for (const auto& child_uuid : prefab_registry->get<HierarchyComponent>(start_prefab_entity).GetChildren())
				{
					Entity child_entity = copy_prefab_entity(prefab->FindEntityByUUID(child_uuid), instantiated_entity.GetUUID());

				}

			}

			return instantiated_entity;

		};

		Entity instantiated_entity = copy_prefab_entity(prefab_root_entity, parent_uuid);

		prefab->m_InstantiationCount++;

		instantiated_entity.GetComponent<TagComponent>().Tag = instantiated_entity.GetComponent<TagComponent>().Tag + "_" + std::to_string(prefab->m_InstantiationCount);

		return instantiated_entity;
	}

#pragma endregion

#pragma region Scene Logic

	void Scene::OnStart() {

		m_IsRunning = true;

		if (!m_CollisionCallback) {
			m_CollisionCallback = std::make_unique<CollisionCallback>(std::static_pointer_cast<Scene>(shared_from_this()));

			m_PhysxScene->setSimulationEventCallback(m_CollisionCallback.get());
		}

		m_SceneConfig.ScenePipeline->OnStartPipeline(std::static_pointer_cast<Scene>(shared_from_this()));
	}

	void Scene::OnUpdate() {

		if (!m_IsPaused && m_IsRunning) {

			PhysicsSystem::UpdatePhysicsObjects(std::static_pointer_cast<Scene>(shared_from_this()));

			m_SceneConfig.ScenePipeline->OnUpdate();

		}
	}

	void Scene::OnUpdateGUI() {



	}

	void Scene::OnFixedUpdate() {
		m_IsSimulatingPhysics = true;

		PhysicsSystem::Update(std::static_pointer_cast<Scene>(shared_from_this()));

		m_IsSimulatingPhysics = false;
	}

	void Scene::OnFixedUpdateGUI() {

	}

	void Scene::OnStop() {

		m_IsRunning = false;

		m_SceneConfig.ScenePipeline->OnStopPipeline();
	}

#pragma endregion

}