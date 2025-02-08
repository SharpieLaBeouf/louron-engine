#include "Scene.h"

// Louron Core Headers
#include "Entity.h"
#include "Prefab.h"
#include "Scene Serializer.h"
#include "OctreeBounds.h"

#include "Components/Components.h"
#include "Components/Light.h"
#include "Components/Mesh.h"
#include "Components/UUID.h"
#include "Components/Skybox.h"

#include "Components/Physics/Collider.h"
#include "Components/Physics/Rigidbody.h"
#include "Components/Physics/PhysicsWrappers.h"
#include "Components/Physics/CollisionCallback.h"

#include "Scene Systems/Physics System.h"

#include "../Debug/Profiler.h"

#include "../Renderer/Camera.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/RendererPipeline.h"

#include "../Core/Time.h"
#include "../Core/Input.h"

#include "../OpenGL/Framebuffer.h"

#include "../Scripting/Script Manager.h"
#include "../Scripting/Script Connector.h"

#include "../Project/Project.h"

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
		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_TOUCH_LOST;
		return PxFilterFlag::eDEFAULT;
	}

	Scene::Scene() {

		m_SceneConfig.SceneFilePath = "Scenes/Untitled Scene.lscene";

		m_SceneConfig.Name = m_SceneConfig.SceneFilePath.filename().replace_extension().string();
		m_SceneConfig.AssetDirectory = "Assets/";

		m_SceneConfig.ScenePipelineType = L_RENDER_PIPELINE::FORWARD_PLUS;
		m_SceneConfig.ScenePipeline = std::make_shared<ForwardPlusPipeline>();
	}

	Scene::Scene(L_RENDER_PIPELINE pipeline) {

		m_SceneConfig.SceneFilePath = "Scenes/Untitled Scene.lscene";

		m_SceneConfig.Name = m_SceneConfig.SceneFilePath.filename().replace_extension().string();
		m_SceneConfig.AssetDirectory = "Assets/";
		m_SceneConfig.ScenePipelineType = pipeline;
		
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
				m_SceneConfig.SceneFilePath = std::filesystem::relative(outFilePath, Project::GetActiveProject()->GetProjectDirectory());
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

		std::unique_lock<std::mutex> lock;

		if (m_Octree) 
			lock = std::unique_lock<std::mutex>(m_Octree->GetOctreeMutex());
		
		while (true) {

			if (m_EntityMap->find(uuid) == m_EntityMap->end())
				break;

			uuid = UUID();
		}

		Entity entity = { m_Registry.create(), this };

		// 1. Add UUID Component
		auto& temp = entity.AddComponent<IDComponent>(uuid);

		// 2. Add Transform Component
		entity.AddComponent<TransformComponent>();

		// 3. Add Tag Component
		std::string uniqueName = name.empty() ? "Untitled Entity" : name;
		int suffix = 1;
		std::string baseName = uniqueName;

		// Ensure the name is unique by appending a numeric suffix		
		auto check_tags = [&](const char* name) -> bool {

			auto view = m_Registry.view<TagComponent>();
			for (auto entity : view) {
				const TagComponent& tag = view.get<TagComponent>(entity);
				if (tag.Tag == name)
					return true;
			}

			return false;
		};

		while (check_tags(uniqueName.c_str())) {
			uniqueName = baseName + " (" + std::to_string(suffix++) + ")";
		}
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = uniqueName;

		// 4. Add Hierarchy Component
		entity.AddComponent<HierarchyComponent>();

		m_EntityMap->emplace(uuid, entity);

		return entity;
	}

	#pragma region Component Copying

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap, Scene* scene_ref)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					UUID dest_id = src.get<IDComponent>(srcEntity).ID;
					entt::entity dstEntity = enttMap.at(dest_id);

					auto& srcComponent = src.get<Component>(srcEntity);
					
					// Store entity uuid and scene of source component
					UUID source_uuid = srcComponent.entity_uuid;
					Scene* source_scene = srcComponent.scene;

					// Change source component uuid and scene to new id and scene
					srcComponent.entity_uuid = dest_id;
					srcComponent.scene = scene_ref;

					// Copy construct new component with new id and scene
					auto& dstComponent = dst.emplace_or_replace<Component>(dstEntity, srcComponent);
					
					// Ensure new component has the correct id and scene ref
					dstComponent.entity_uuid = dest_id;
					dstComponent.scene = scene_ref;

					// Revert source component to its normal state
					srcComponent.entity_uuid = source_uuid;
					srcComponent.scene = source_scene;
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap, Scene* scene_ref)
	{
		CopyComponent<Component...>(dst, src, enttMap, scene_ref);
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
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap, this);
		return true;
	}

	std::shared_ptr<Scene> Scene::Copy(std::shared_ptr<Scene> source_scene)
	{
		L_RENDER_PIPELINE pipeline = source_scene->GetConfig().ScenePipelineType;

		std::shared_ptr<Scene> dest_scene = std::make_shared<Scene>(pipeline);

		dest_scene->m_DisplayOctree = source_scene->m_DisplayOctree;
		dest_scene->m_IsPaused = source_scene->m_IsPaused;
		dest_scene->m_IsRunning = source_scene->m_IsRunning;
		dest_scene->m_IsPhysicsCalculating = source_scene->m_IsPhysicsCalculating;

		dest_scene->m_SceneConfig.AssetDirectory = source_scene->m_SceneConfig.AssetDirectory;
		dest_scene->m_SceneConfig.Name = source_scene->m_SceneConfig.Name;
		dest_scene->m_SceneConfig.SceneFilePath = source_scene->m_SceneConfig.SceneFilePath;
		dest_scene->m_SceneConfig.ScenePipelineType = source_scene->m_SceneConfig.ScenePipelineType;

		dest_scene->CopyRegistry(source_scene);

		// Generate Octree for New Scene
		{
			// Calculate Overall Scene Octree
			OctreeBoundsConfig octree_config{};

			std::vector<OctreeBounds<Entity>::OctreeData> data_sources;

			auto bounds_view_mesh = dest_scene->GetAllEntitiesWith<MeshFilterComponent, MeshRendererComponent>();
			for (const auto& entity_handle : bounds_view_mesh) {
				auto& mesh_filter = bounds_view_mesh.get<MeshFilterComponent>(entity_handle);

				// Ensure the AABB is up to date
				mesh_filter.UpdateTransformedAABB();

				const auto& aabb = mesh_filter.TransformedAABB;

				data_sources.push_back(std::make_shared<OctreeDataSource<Entity>>(mesh_filter.GetEntity(), aabb));
			}

			octree_config.Looseness = 1.25f;
			octree_config.PreferredDataSourceLimit = 8;

			// Create the octree and insert data sources
			dest_scene->m_Octree = std::make_shared<OctreeBounds<Entity>>(octree_config, data_sources);
		}

		auto view = dest_scene->GetAllEntitiesWith<IDComponent>();
		for (auto& entity : view) {
			Entity ent = { entity, dest_scene.get() };
		}

		return dest_scene;
	}

	#pragma endregion

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

	// FBO Stuff
	void Scene::CreateSceneFrameBuffer(const FrameBufferConfig& framebuffer_config) { m_SceneFrameBuffer = std::make_shared<FrameBuffer>(framebuffer_config); }
	void Scene::SetSceneFrameBuffer(std::shared_ptr<FrameBuffer> framebuffer) { m_SceneFrameBuffer = framebuffer; }
	std::shared_ptr<FrameBuffer> Scene::GetSceneFrameBuffer() const { return m_SceneFrameBuffer; }

	// TODO: Duplicates Entity in Scene
	Entity Scene::DuplicateEntity(Entity entity) {
		return Entity();
	}

	// Destroys Entity in Scene
	void Scene::DestroyEntity(Entity entity, std::unique_lock<std::mutex>* parent_lock) {

		// Need to lock the octree because it may be trying to 
		// get things from scene as it's being deleted!
		std::unique_lock<std::mutex> octree_lock;
		if (!parent_lock && m_Octree)
			octree_lock = std::unique_lock<std::mutex>(m_Octree->GetOctreeMutex());

		// 1. Check if entity is valid
		if (!entity) {
			L_CORE_WARN("Attempted to Destroy Null Entity.");
			return;
		}

		// 2. Check If Entity is Part of the Scene
		if (!HasEntity(entity.GetUUID())) {
			L_CORE_WARN("Attempted to Destroy an Entity Not In The Scene.");
			return;
		}

		if (m_IsRunning && entity.HasComponent<ScriptComponent>())
			ScriptManager::OnDestroyEntity(entity);

		// 3. Call Physics System Remove Methods
		if (entity.HasAnyComponent<RigidbodyComponent, SphereColliderComponent, BoxColliderComponent>()) {

			if (entity.HasComponent<RigidbodyComponent>())
			{
				entity.GetComponent<RigidbodyComponent>().Shutdown();
				PhysicsSystem::RemoveRigidBody(entity, this);
			}

			if (entity.HasComponent<SphereColliderComponent>())
			{
				entity.GetComponent<SphereColliderComponent>().Shutdown();
				PhysicsSystem::RemoveCollider(entity, this, PxGeometryType::eSPHERE);
			}
			if (entity.HasComponent<BoxColliderComponent>())
			{
				entity.GetComponent<BoxColliderComponent>().Shutdown();
				PhysicsSystem::RemoveCollider(entity, this, PxGeometryType::eBOX);
			}
		}

		// 4. Remove From Parent and Destroy All Children
		if (entity.HasComponent<HierarchyComponent>()) {

			auto& component = entity.GetComponent<HierarchyComponent>();

			if (component.HasParent()) 
				component.DetachParent();
			
			// Make a copy of the child list so we don't invalidate 
			// the iterator whilst destroying children
			std::vector<UUID> children_vec = component.GetChildren();
			for (const auto& children_uuid : children_vec) 
				DestroyEntity(FindEntityByUUID(children_uuid), &octree_lock);
		}

		// 5. Remove the Entity from the Scene Entity Map
		m_EntityMap->erase(entity.GetUUID());

		// 6. Destroy the Entity and Components from the ENTT Registry
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
		return Entity{ entt::null, nullptr };
	}

	bool Scene::HasEntityByUUID(UUID uuid)
	{
		return m_EntityMap->find(uuid) != m_EntityMap->end();
	}

	Entity Scene::FindEntityByUUID(UUID uuid)
	{
		if (m_EntityMap->find(uuid) != m_EntityMap->end()) {
			return Entity{ m_EntityMap->at(uuid), this };
		}

		L_CORE_WARN("Entity UUID not found in scene: {0}", std::to_string(uuid));
		return Entity{ entt::null, nullptr };
	}

	// Returns Primary Camera Entity
	Entity Scene::GetPrimaryCameraEntity() {

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			const CameraComponent& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}

		L_CORE_WARN("Scene Does Not Have an Entity with a Camera Component.");
		return Entity{ entt::null, nullptr };
	}

	bool Scene::HasEntity(const Entity& entity)
	{
		if(ValidEntity(entity))
			return m_Registry.has(entity);
		return false;
	}
	bool Scene::HasEntity(const std::string& name)
	{
		return (FindEntityByName(name)) ? true : false;
	}
	bool Scene::HasEntity(const UUID& uuid)
	{
		return (FindEntityByUUID(uuid)) ? true : false;
	}

	bool Scene::ValidEntity(const Entity& entity)
	{
		return m_Registry.valid(entity);
	}

	Entity Scene::InstantiatePrefab(std::shared_ptr<Prefab> prefab, std::optional<TransformComponent> transform, const UUID& parent_uuid)
	{
		if (!prefab)
			return {};

		entt::registry* prefab_registry = &prefab->m_PrefabRegistry;
		entt::entity prefab_root_entity = prefab->m_RootEntity;
		std::string prefab_name = prefab->m_PrefabName;

		std::vector<Entity> LODMeshEntities; // So we can easily resolve the prefab handles to entity UUIDs
		std::unordered_map<Louron::UUID, Louron::UUID> PrefabUUID_To_EntityUUID{}; // Key == Prefab entt::entity, Value == instantiated_entity.GetUUID()

		std::function<Entity(entt::entity, const UUID&)> copy_prefab_entity = [&](entt::entity start_prefab_entity, const UUID& parent_uuid) -> Entity {

			Entity instantiated_entity = this->CreateEntity("");

			PrefabUUID_To_EntityUUID[(uint32_t)start_prefab_entity] = instantiated_entity.GetUUID();

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
				if (prefab_registry->has<TransformComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<TransformComponent>(start_prefab_entity);
					auto& transform_component = instantiated_entity.GetComponent<TransformComponent>();
					
					if (parent_uuid == NULL_UUID) { 
						// If we are the root entity, we check if the transform 
						// passed has a value, if not, standard copy from prefab
						if (transform.has_value())
						{
							transform_component.SetPosition(transform.value().GetLocalPosition());
							transform_component.SetRotation(transform.value().GetLocalRotation());
							transform_component.SetScale(transform.value().GetLocalScale());
						}
						else
						{
							transform_component.SetPosition(component.GetLocalPosition());
							transform_component.SetRotation(component.GetLocalRotation());
							transform_component.SetScale(component.GetLocalScale());
						}
					}
					else {
						transform_component.SetPosition(component.GetLocalPosition());
						transform_component.SetRotation(component.GetLocalRotation());
						transform_component.SetScale(component.GetLocalScale());
					}
				}

				// 1.g. MeshFilter
				if (prefab_registry->has<MeshFilterComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<MeshFilterComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<MeshFilterComponent>(component);
				}

				// 1.h. MeshRenderer
				if (prefab_registry->has<MeshRendererComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<MeshRendererComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<MeshRendererComponent>(component);
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
				if (prefab_registry->has<RigidbodyComponent>(start_prefab_entity)) {
					RigidbodyComponent component = prefab_registry->get<RigidbodyComponent>(start_prefab_entity);
					auto& ent_rb_component = instantiated_entity.AddComponent<RigidbodyComponent>(); 
					ent_rb_component = component;

					if (IsRunning() || IsSimulating())
						instantiated_entity.GetComponent<RigidbodyComponent>().Init(&instantiated_entity.GetComponent<TransformComponent>(), m_PhysxScene);
				}

				// 1.n. Sphere Collider
				if (prefab_registry->has<SphereColliderComponent>(start_prefab_entity)) {
					SphereColliderComponent component = prefab_registry->get<SphereColliderComponent>(start_prefab_entity);
					auto& ent_sc_component = instantiated_entity.AddComponent<SphereColliderComponent>();
					ent_sc_component = component;


					if (IsRunning() || IsSimulating())
						instantiated_entity.GetComponent<SphereColliderComponent>().Init();
				}

				// 1.o. Box Collider
				if (prefab_registry->has<BoxColliderComponent>(start_prefab_entity)) {
					BoxColliderComponent component = prefab_registry->get<BoxColliderComponent>(start_prefab_entity);
					auto& ent_bc_component = instantiated_entity.AddComponent<BoxColliderComponent>();
					ent_bc_component = std::move(component);

					if (IsRunning() || IsSimulating())
						instantiated_entity.GetComponent<BoxColliderComponent>().Init();
				}

				// 1.p. Script Component
				if (prefab_registry->has<ScriptComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<ScriptComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<ScriptComponent>(component);

					// TODO: Add functionality to move script and script fields from prefabs into new components
					// basically just need to copy the ScriptFieldInstances from the prefabs entries in the scriptmanager
					// to this new entity! easy peasy...

				}

				// 1.q. LOD Component
				if (prefab_registry->has<LODMeshComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<LODMeshComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<LODMeshComponent>(component);
					LODMeshEntities.push_back(instantiated_entity);
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

		// Resolve prefab handles to scene uuids
		for (auto& entity : LODMeshEntities)
		{
			if (!entity || !entity.HasComponent<LODMeshComponent>())
				continue;

			auto& component = entity.GetComponent<LODMeshComponent>();
			for (auto& element : component.LOD_Elements)
			{
				for (auto& entity_handle : element.MeshRendererEntities)
				{
					entity_handle = PrefabUUID_To_EntityUUID[entity_handle];
				}
			}
		}

		return instantiated_entity;
	}

#pragma endregion

#pragma region Scene Logic
	
	// SCENE HARD START & STOP
	// All scenes are started when they are created, not when we 
	// are playing. This is to setup required things such as 
	// collision callbacks and rendering pipeline
	void Scene::OnStart() {

		m_SceneConfig.ScenePipeline->OnStartPipeline(std::static_pointer_cast<Scene>(shared_from_this()));
	}

	void Scene::OnStop() {

		if (m_IsRunning)
			OnRuntimeStop();

		if (m_IsSimulating)
			OnSimulationStop();

		m_IsRunning = false;
		m_IsSimulating = false;

		m_SceneConfig.ScenePipeline->OnStopPipeline();
	}

	// RUNTIME
	void Scene::OnRuntimeStart() {

		m_IsRunning = true;

		// Scripting
		{
			ScriptManager::OnRuntimeStart(std::static_pointer_cast<Scene>(shared_from_this()));
			// Instantiate all script entities

			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				ScriptManager::OnCreateEntity(entity);
			}
		}

		OnPhysicsStart();
	}
	
	void Scene::OnRuntimeStop() {

		m_IsRunning = false;
		m_IsSimulating = false;

		ScriptManager::OnRuntimeStop();
		OnPhysicsStop();
	}

	// PHYSICS SIMULATION
	void Scene::OnSimulationStart() { 

		m_IsSimulating = true;

		OnPhysicsStart();
	}

	void Scene::OnSimulationStop() {

		m_IsRunning = false;
		m_IsSimulating = false;

		OnPhysicsStop();
	}

	void Scene::OnPhysicsStart() {

		// 1. Create new PhysX scene
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

		if (!m_CollisionCallback) {
			m_CollisionCallback = std::make_unique<CollisionCallback>(std::static_pointer_cast<Scene>(shared_from_this()));

			m_PhysxScene->setSimulationEventCallback(m_CollisionCallback.get());
		}

		// 2. Get All Entities with physics components
		std::unordered_map<UUID, Entity> entities;
		{
			auto rb_view = GetAllEntitiesWith<RigidbodyComponent>();
			for (auto& entt_id : rb_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}
			
			auto bc_view = GetAllEntitiesWith<BoxColliderComponent>();
			for (auto& entt_id : bc_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}

			auto sc_view = GetAllEntitiesWith<SphereColliderComponent>();
			for (auto& entt_id : sc_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}
		}

		// 3. Initialise All Physics Components
		for (auto& [uuid, entity] : entities) {

			if (entity.HasComponent<RigidbodyComponent>())
				entity.GetComponent<RigidbodyComponent>().Init(&entity.GetComponent<TransformComponent>(), m_PhysxScene);
			
			if (entity.HasComponent<BoxColliderComponent>())
				entity.GetComponent<BoxColliderComponent>().Init();
			
			if (entity.HasComponent<SphereColliderComponent>())
				entity.GetComponent<SphereColliderComponent>().Init();
			
		}
	}

	void Scene::OnPhysicsStop() {

		// 1. Get All Entities with physics components

		std::unordered_map<UUID, Entity> entities;
		{
			auto rb_view = GetAllEntitiesWith<RigidbodyComponent>();
			for (auto& entt_id : rb_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}

			auto bc_view = GetAllEntitiesWith<BoxColliderComponent>();
			for (auto& entt_id : bc_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}

			auto sc_view = GetAllEntitiesWith<SphereColliderComponent>();
			for (auto& entt_id : sc_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}
		}

		// 2. Shutdown All Physics Components
		for (auto& [uuid, entity] : entities) {

			if (entity.HasComponent<RigidbodyComponent>())
				entity.GetComponent<RigidbodyComponent>().Shutdown();

			if (entity.HasComponent<BoxColliderComponent>())
				entity.GetComponent<BoxColliderComponent>().Shutdown();

			if (entity.HasComponent<SphereColliderComponent>())
				entity.GetComponent<SphereColliderComponent>().Shutdown();
		}

		m_PhysxScene->release();
		m_PhysxScene = nullptr;
	}

	// UPDATE
	void Scene::OnUpdate(EditorCamera* editor_camera) {

		// Physics
		if (!m_IsPaused && (m_IsRunning || m_IsSimulating)) {
			PhysicsSystem::UpdatePhysicsObjects(std::static_pointer_cast<Scene>(shared_from_this()));
		}

		// Scripts - only if running
		if (!m_IsPaused && m_IsRunning) {

			// See if any entities that have inactive scripts have recently become active
			ScriptManager::CheckInactiveScriptsOnEntities();

			auto script_entities = m_Registry.view<ScriptComponent>();
			for (auto script_entity : script_entities)
				ScriptManager::OnUpdateEntity({ script_entity, this });
			
		}

		CameraBase* camera = nullptr;
		Entity camera_entity = GetPrimaryCameraEntity();
		if (camera_entity && !editor_camera)
			camera = camera_entity.GetComponent<CameraComponent>().CameraInstance.get();
		else if (editor_camera)
			camera = reinterpret_cast<CameraBase*>(editor_camera);

		if (camera) {

			static glm::vec3 camera_position{};
			static glm::mat4 projection_matrix{};
			static glm::mat4 view_matrix{};

			switch (camera->GetCameraType()) {

				case Camera_Type::None:
				{
					camera_position = {};
					projection_matrix = glm::mat4(1.0f);
					view_matrix = glm::mat4(1.0f);
					break;
				}

				case Camera_Type::SceneCamera:
				{
					camera_position = GetPrimaryCameraEntity().GetComponent<TransformComponent>().GetGlobalPosition();
					projection_matrix = camera->GetProjection();
					// If we are using a scene camera which is attached to a 
					// camera compoennt, it is simple to get the view matrix 
					// by simply inverting the global transform matrix
					view_matrix = glm::inverse(GetPrimaryCameraEntity().GetComponent<TransformComponent>().GetGlobalTransform());
					break;
				}

				case Camera_Type::EditorCamera:
				{
					camera_position = editor_camera->GetPosition();
					projection_matrix = camera->GetProjection();
					view_matrix = camera->GetViewMatrix();
					break;
				}
			}

			// Always Render
			m_SceneFrameBuffer->Bind();
			m_SceneConfig.ScenePipeline->OnUpdate(camera_position, projection_matrix, view_matrix);
			m_SceneFrameBuffer->Unbind();

			// Clear the standard OpenGL back buffer
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (m_SceneFrameBuffer->GetConfig().RenderToScreen)
				m_SceneConfig.ScenePipeline->RenderFBOQuad();
		}
		else {
			L_CORE_WARN("No Primary Camera Found in Scene");
			m_SceneFrameBuffer->Bind();
			Renderer::ClearColour({ 99.0f , 99.0f, 99.0f, 1.0f });
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_SceneFrameBuffer->Unbind();
		}

	}

	void Scene::OnUpdateGUI() {



	}

	void Scene::OnFixedUpdate() {

		// Scripts - only if running
		if (!m_IsPaused && m_IsRunning) {

			auto script_entities = m_Registry.view<ScriptComponent>();
			for (auto script_entity : script_entities)
				ScriptManager::OnFixedUpdateEntity({ script_entity, this });

		}

		// Physics
		if (!m_IsPaused && (m_IsRunning || m_IsSimulating)) {

			m_IsPhysicsCalculating = true;

			PhysicsSystem::Update(std::static_pointer_cast<Scene>(shared_from_this()));

			// Handle persistent collision triggers as trigger callback is not called when contact is persistent
			for (const auto& pair : CollisionCallback::s_ActiveTriggers) {
				Entity triggerEntity = FindEntityByUUID(pair.first);
				Entity otherEntity = FindEntityByUUID(pair.second);

				if (triggerEntity && otherEntity && triggerEntity.HasComponent<ScriptComponent>()) {
					ScriptManager::OnCollideEntity(triggerEntity, otherEntity, _Collision_Type::TriggerStay);
				}
			}

			m_IsPhysicsCalculating = false;
		}

	}

	void Scene::OnViewportResize(const glm::ivec2& new_size) {

		glm::ivec2 final_size = new_size;

		if (final_size.x <= 0 || final_size.y <= 0) {
			L_CORE_ERROR("Attempted to Resize FrameBuffer to Invalid Size: X({0}), Y({1}).", new_size.x, new_size.y);
			L_CORE_ERROR("Setting New Viewport Size to: X(1), Y(1).");
			final_size = { 1,1 };
		}

		if (m_SceneFrameBuffer) {

			if (m_SceneFrameBuffer->GetConfig().Width == final_size.x && m_SceneFrameBuffer->GetConfig().Height == final_size.y)
				return;

			// Update Render Pipeline Data
			m_SceneFrameBuffer->Resize(final_size);
			m_SceneConfig.ScenePipeline->OnViewportResize();

		}

	}


#pragma endregion

}