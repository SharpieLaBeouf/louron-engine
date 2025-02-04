#include "Script Connector.h"

// Louron Core Headers
#include "../Asset/Asset Manager API.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

	namespace ScriptingUtils {

		static std::string MonoStringToString(MonoString* string)
		{
			char* cStr = mono_string_to_utf8(string);
			std::string str(cStr);
			mono_free(cStr);
			return str;
		}

	}

#pragma region Function Register

	void ScriptConnector::RegisterFunctions() {

		mono_add_internal_call("Louron.EngineCallbacks::Debug_LogMessage", Debug_LogMessage);

		mono_add_internal_call("Louron.EngineCallbacks::Entity_DestroyEntity", Entity_DestroyEntity);
		mono_add_internal_call("Louron.EngineCallbacks::Entity_AddComponent", Entity_AddComponent);
		mono_add_internal_call("Louron.EngineCallbacks::Entity_RemoveComponent", Entity_RemoveComponent);
		mono_add_internal_call("Louron.EngineCallbacks::Entity_HasComponent", Entity_HasComponent);

		mono_add_internal_call("Louron.EngineCallbacks::Entity_Instantiate", Entity_Instantiate);

		mono_add_internal_call("Louron.EngineCallbacks::Entity_GetParent", Entity_GetParent);
		mono_add_internal_call("Louron.EngineCallbacks::Entity_SetParent", Entity_SetParent);
		
		mono_add_internal_call("Louron.EngineCallbacks::Input_GetKey", Input_GetKey);
		mono_add_internal_call("Louron.EngineCallbacks::Input_GetKeyDown", Input_GetKeyDown);
		mono_add_internal_call("Louron.EngineCallbacks::Input_GetKeyUp", Input_GetKeyUp);
		mono_add_internal_call("Louron.EngineCallbacks::Input_GetMouseButton", Input_GetMouseButton);
		mono_add_internal_call("Louron.EngineCallbacks::Input_GetMouseButtonDown", Input_GetMouseButtonDown);
		mono_add_internal_call("Louron.EngineCallbacks::Input_GetMouseButtonUp", Input_GetMouseButtonUp);
		mono_add_internal_call("Louron.EngineCallbacks::Input_GetMousePosition", Input_GetMousePosition);

		mono_add_internal_call("Louron.EngineCallbacks::Time_GetDeltaTime", Time_GetDeltaTime);
		mono_add_internal_call("Louron.EngineCallbacks::Time_GetCurrentTime", Time_GetCurrentTime);

		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_GetTransform", TransformComponent_GetTransform);
		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_SetTransform", TransformComponent_SetTransform);

		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_GetPosition", TransformComponent_GetPosition);
		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_SetPosition", TransformComponent_SetPosition);

		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_GetRotation", TransformComponent_GetRotation);
		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_SetRotation", TransformComponent_SetRotation);

		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_GetScale", TransformComponent_GetScale);
		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_SetScale", TransformComponent_SetScale);

		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_SetFront", TransformComponent_SetFront);
		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_GetFront", TransformComponent_GetFront);
		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_GetUp", TransformComponent_GetUp);
		mono_add_internal_call("Louron.EngineCallbacks::TransformComponent_GetRight", TransformComponent_GetRight);

		mono_add_internal_call("Louron.EngineCallbacks::TagComponent_GetTag", Tag_GetTag);
		mono_add_internal_call("Louron.EngineCallbacks::TagComponent_SetTag", Tag_SetTag);

		//mono_add_internal_call("Louron.EngineCallbacks::ScriptComponent_GetScript", Script_GetScriptName);
		//mono_add_internal_call("Louron.EngineCallbacks::ScriptComponent_SetScript", Script_SetScriptName);

		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_GetActive", PointLightComponent_GetActive);
		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_SetActive", PointLightComponent_SetActive);
		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_GetColour", PointLightComponent_GetColour);
		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_SetColour", PointLightComponent_SetColour);
		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_GetRadius", PointLightComponent_GetRadius);
		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_SetRadius", PointLightComponent_SetRadius);
		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_GetIntensity", PointLightComponent_GetIntensity);
		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_SetIntensity", PointLightComponent_SetIntensity);
		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_GetShadowFlag", PointLightComponent_GetShadowFlag);
		mono_add_internal_call("Louron.EngineCallbacks::PointLightComponent_SetShadowFlag", PointLightComponent_SetShadowFlag);

		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_GetActive", SpotLightComponent_GetActive);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_SetActive", SpotLightComponent_SetActive);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_GetColour", SpotLightComponent_GetColour);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_SetColour", SpotLightComponent_SetColour);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_GetRange", SpotLightComponent_GetRange);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_SetRange", SpotLightComponent_SetRange);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_GetAngle", SpotLightComponent_GetAngle);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_SetAngle", SpotLightComponent_SetAngle);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_GetIntensity", SpotLightComponent_GetIntensity);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_SetIntensity", SpotLightComponent_SetIntensity);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_GetShadowFlag", SpotLightComponent_GetShadowFlag);
		mono_add_internal_call("Louron.EngineCallbacks::SpotLightComponent_SetShadowFlag", SpotLightComponent_SetShadowFlag);

		mono_add_internal_call("Louron.EngineCallbacks::DirectionalLightComponent_GetActive", DirectionalLightComponent_GetActive);
		mono_add_internal_call("Louron.EngineCallbacks::DirectionalLightComponent_SetActive", DirectionalLightComponent_SetActive);
		mono_add_internal_call("Louron.EngineCallbacks::DirectionalLightComponent_GetColour", DirectionalLightComponent_GetColour);
		mono_add_internal_call("Louron.EngineCallbacks::DirectionalLightComponent_SetColour", DirectionalLightComponent_SetColour);
		mono_add_internal_call("Louron.EngineCallbacks::DirectionalLightComponent_GetIntensity", DirectionalLightComponent_GetIntensity);
		mono_add_internal_call("Louron.EngineCallbacks::DirectionalLightComponent_SetIntensity", DirectionalLightComponent_SetIntensity);
		mono_add_internal_call("Louron.EngineCallbacks::DirectionalLightComponent_GetShadowFlag", DirectionalLightComponent_GetShadowFlag);
		mono_add_internal_call("Louron.EngineCallbacks::DirectionalLightComponent_SetShadowFlag", DirectionalLightComponent_SetShadowFlag);

		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_GetMass",					Rigidbody_GetMass);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_SetMass",					Rigidbody_SetMass);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_GetDrag",					Rigidbody_GetDrag);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_SetDrag",					Rigidbody_SetDrag);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_GetAngularDrag",				Rigidbody_GetAngularDrag);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_SetAngularDrag",				Rigidbody_SetAngularDrag);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_GetAutomaticCentreOfMass",	Rigidbody_GetAutomaticCentreOfMass);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_SetAutomaticCentreOfMass",	Rigidbody_SetAutomaticCentreOfMass);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_GetUseGravity",				Rigidbody_GetUseGravity);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_SetUseGravity",				Rigidbody_SetUseGravity);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_GetIsKinematic",				Rigidbody_GetIsKinematic);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_SetIsKinematic",				Rigidbody_SetIsKinematic);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_GetPositionConstraint",		Rigidbody_GetPositionConstraint);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_SetPositionConstraint",		Rigidbody_SetPositionConstraint);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_GetRotationConstraint",		Rigidbody_GetRotationConstraint);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_SetRotationConstraint",		Rigidbody_SetRotationConstraint);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_ApplyForce",					Rigidbody_ApplyForce);
		mono_add_internal_call("Louron.EngineCallbacks::RigidbodyComponent_ApplyTorque",				Rigidbody_ApplyTorque);

		mono_add_internal_call("Louron.EngineCallbacks::BoxColliderComponent_GetIsTrigger",		BoxColliderComponent_GetIsTrigger);
		mono_add_internal_call("Louron.EngineCallbacks::BoxColliderComponent_SetIsTrigger",		BoxColliderComponent_SetIsTrigger);
		mono_add_internal_call("Louron.EngineCallbacks::BoxColliderComponent_GetCentre",		BoxColliderComponent_GetCentre);
		mono_add_internal_call("Louron.EngineCallbacks::BoxColliderComponent_SetCentre",		BoxColliderComponent_SetCentre);
		mono_add_internal_call("Louron.EngineCallbacks::BoxColliderComponent_GetSize",			BoxColliderComponent_GetSize);
		mono_add_internal_call("Louron.EngineCallbacks::BoxColliderComponent_SetSize",			BoxColliderComponent_SetSize);
		mono_add_internal_call("Louron.EngineCallbacks::BoxColliderComponent_GetMaterial",		BoxColliderComponent_GetMaterial);
		mono_add_internal_call("Louron.EngineCallbacks::BoxColliderComponent_SetMaterial",		BoxColliderComponent_SetMaterial);

		mono_add_internal_call("Louron.EngineCallbacks::SphereColliderComponent_GetIsTrigger",	SphereColliderComponent_GetIsTrigger);
		mono_add_internal_call("Louron.EngineCallbacks::SphereColliderComponent_SetIsTrigger",	SphereColliderComponent_SetIsTrigger);
		mono_add_internal_call("Louron.EngineCallbacks::SphereColliderComponent_GetCentre",		SphereColliderComponent_GetCentre);
		mono_add_internal_call("Louron.EngineCallbacks::SphereColliderComponent_SetCentre",		SphereColliderComponent_SetCentre);
		mono_add_internal_call("Louron.EngineCallbacks::SphereColliderComponent_GetRadius",		SphereColliderComponent_GetRadius);
		mono_add_internal_call("Louron.EngineCallbacks::SphereColliderComponent_SetRadius",		SphereColliderComponent_SetRadius);
		mono_add_internal_call("Louron.EngineCallbacks::SphereColliderComponent_GetMaterial",	SphereColliderComponent_GetMaterial);
		mono_add_internal_call("Louron.EngineCallbacks::SphereColliderComponent_SetMaterial",	SphereColliderComponent_SetMaterial);

		mono_add_internal_call("Louron.EngineCallbacks::ComputeShader_Dispatch",	ComputeShader_Dispatch);	
		mono_add_internal_call("Louron.EngineCallbacks::ComputeShader_SetBuffer",	ComputeShader_SetBuffer);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeShader_SetBool",		ComputeShader_SetBool);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeShader_SetInt",		ComputeShader_SetInt);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeShader_SetUInt",		ComputeShader_SetUInt);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeShader_SetFloat",	ComputeShader_SetFloat);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeShader_SetVector2",	ComputeShader_SetVector2);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeShader_SetVector3",	ComputeShader_SetVector3);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeShader_SetVector4",	ComputeShader_SetVector4);
		
		mono_add_internal_call("Louron.EngineCallbacks::ComputeBuffer_Create",	ComputeBuffer_Create);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeBuffer_SetData", ComputeBuffer_SetData);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeBuffer_GetData", ComputeBuffer_GetData);
		mono_add_internal_call("Louron.EngineCallbacks::ComputeBuffer_Release", ComputeBuffer_Release);

	}

#pragma endregion

	void ScriptConnector::Debug_LogMessage(int type, MonoString* message) {

		switch (type) {
			case 0:		L_CORE_INFO(ScriptingUtils::MonoStringToString(message));	break;
			case 1:		L_CORE_WARN(ScriptingUtils::MonoStringToString(message));	break;
			case 2:		L_CORE_ERROR(ScriptingUtils::MonoStringToString(message));	break;
			case 3:		L_CORE_FATAL(ScriptingUtils::MonoStringToString(message));	break;
			default:	L_CORE_INFO(ScriptingUtils::MonoStringToString(message));	break;
		}
	}

#pragma region Entity

	void ScriptConnector::Entity_DestroyEntity(UUID entityID) {

		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		scene->DestroyEntity(entity);
	}

	void ScriptConnector::Entity_AddComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		L_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end(), "Component Not Setup in Louron Engine.");

		std::string component_name = mono_type_get_name(managedType);

		if (component_name == "Louron.TransformComponent") {
			entity.AddComponent<TransformComponent>();
		}
		if (component_name == "Louron.TagComponent") {
			entity.AddComponent<TagComponent>();
		}
		if (component_name == "Louron.ScriptComponent") {
			entity.AddComponent<ScriptComponent>();
		}
		if (component_name == "Louron.PointLightComponent") {
			entity.AddComponent<PointLightComponent>();
		}
		if (component_name == "Louron.SpotLightComponent") {
			entity.AddComponent<SpotLightComponent>();
		}
		if (component_name == "Louron.DirectionalLightComponent") {
			entity.AddComponent<DirectionalLightComponent>();
		}
		if (component_name == "Louron.RigidbodyComponent") {
			entity.AddComponent<RigidbodyComponent>();
		}
		if (component_name == "Louron.BoxColliderComponent") {
			entity.AddComponent<BoxColliderComponent>();
		}
		if (component_name == "Louron.SphereColliderComponent") {
			entity.AddComponent<SphereColliderComponent>();
		}
	}

	void ScriptConnector::Entity_RemoveComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		L_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end(), "Component Not Setup in Louron Engine.");

		std::string component_name = mono_type_get_name(managedType);

		if (component_name == "Louron.TransformComponent") {
			entity.RemoveComponent<TransformComponent>();
		}
		if (component_name == "Louron.TagComponent") {
			entity.RemoveComponent<TagComponent>();
		}
		if (component_name == "Louron.ScriptComponent") {
			entity.RemoveComponent<ScriptComponent>();
		}
		if (component_name == "Louron.PointLightComponent") {
			entity.RemoveComponent<PointLightComponent>();
		}
		if (component_name == "Louron.SpotLightComponent") {
			entity.RemoveComponent<SpotLightComponent>();
		}
		if (component_name == "Louron.DirectionalLightComponent") {
			entity.RemoveComponent<DirectionalLightComponent>();
		}
		if (component_name == "Louron.Rigidbody") {
			entity.RemoveComponent<RigidbodyComponent>();
		}
		if (component_name == "Louron.BoxCollider") {
			entity.RemoveComponent<BoxColliderComponent>();
		}
		if (component_name == "Louron.SphereCollider") {
			entity.RemoveComponent<SphereColliderComponent>();
		}
	}

	bool ScriptConnector::Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return false;

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		L_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end(), "Could Not Find Component Function.");
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	void ScriptConnector::Entity_Instantiate(UUID entityID, uint32_t* handle, uint32_t* prefab_clone_uuid) {

		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity)
			return;

		auto prefab_asset = AssetManager::GetAsset<Prefab>(*handle);

		Entity prefab_clone = scene->InstantiatePrefab(prefab_asset);
		if (prefab_clone) {
			prefab_clone.GetComponent<TagComponent>().SetUniqueName(prefab_asset->GetPrefabName());
			*prefab_clone_uuid = prefab_clone.GetUUID();
		}
	}

	UUID ScriptConnector::Entity_GetParent(UUID entityID) {

		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity)
			return NULL_UUID;

		return entity.GetComponent<HierarchyComponent>().GetParentID();
	}

	void ScriptConnector::Entity_SetParent(UUID entityID, UUID parentID) {

		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		Entity parent = scene->FindEntityByUUID(parentID);
		if (!entity || !parent)
			return;

		entity.GetComponent<HierarchyComponent>().AttachParent(parentID);
	}

#pragma endregion

#pragma region Input

	bool ScriptConnector::Input_GetKey(KeyCode keyCode) {
		return Engine::Get().GetInput().GetKey(keyCode);
	}

	bool ScriptConnector::Input_GetKeyDown(KeyCode keyCode) {
		return Engine::Get().GetInput().GetKeyDown(keyCode);
	}

	bool ScriptConnector::Input_GetKeyUp(KeyCode keyCode) {
		return Engine::Get().GetInput().GetKeyUp(keyCode);
	}

	bool ScriptConnector::Input_GetMouseButton(MouseButtonCode buttonCode) {
		return Engine::Get().GetInput().GetMouseButton(buttonCode);
	}

	bool ScriptConnector::Input_GetMouseButtonDown(MouseButtonCode buttonCode) {
		return Engine::Get().GetInput().GetMouseButtonDown(buttonCode);
	}

	bool ScriptConnector::Input_GetMouseButtonUp(MouseButtonCode buttonCode) {
		return Engine::Get().GetInput().GetMouseButtonUp(buttonCode);
	}

	void ScriptConnector::Input_GetMousePosition(glm::vec2* mousePos) {
		*mousePos = Engine::Get().GetInput().GetMousePosition();
	}

#pragma endregion

#pragma region Time

	void ScriptConnector::Time_GetDeltaTime(float* out_float) {
		*out_float = Time::GetDeltaTime();
	}

	void ScriptConnector::Time_GetCurrentTime(float* out_float) {
		*out_float = static_cast<float>(Time::Get().GetCurrTime());
	}

#pragma endregion

#pragma region TransformComponent

	void ScriptConnector::TransformComponent_GetTransform(UUID entityID, _Transform* out_transform)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();
		_Transform trans{};
		trans.position = entity_transform.GetGlobalPosition();
		trans.rotation = entity_transform.GetGlobalRotation();
		trans.scale = entity_transform.GetGlobalScale();
		*out_transform = trans;
	}

	void ScriptConnector::TransformComponent_SetTransform(UUID entityID, _Transform* transform)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();
		entity_transform.SetGlobalPosition(transform->position);
		entity_transform.SetGlobalRotation(transform->rotation);
		entity_transform.SetGlobalScale(transform->scale);
	}

	void ScriptConnector::TransformComponent_GetPosition(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		*ref = entity_transform.GetGlobalPosition();
	}

	void ScriptConnector::TransformComponent_SetPosition(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		entity_transform.SetGlobalPosition(*ref);
	}


	void ScriptConnector::TransformComponent_GetRotation(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		*ref = entity_transform.GetGlobalRotation();
	}

	void ScriptConnector::TransformComponent_SetRotation(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		entity_transform.SetGlobalRotation(*ref);
	}

	void ScriptConnector::TransformComponent_GetScale(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		*ref = entity_transform.GetGlobalScale();
	}

	void ScriptConnector::TransformComponent_SetScale(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		entity_transform.SetGlobalScale(*ref);
	}

	void ScriptConnector::TransformComponent_SetFront(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		entity_transform.SetForwardDirection(*ref);
	}

	void ScriptConnector::TransformComponent_GetFront(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		*ref = entity_transform.GetForwardDirection();
	}

	void ScriptConnector::TransformComponent_GetUp(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		*ref = entity_transform.GetUpDirection();
	}

	void ScriptConnector::TransformComponent_GetRight(UUID entityID, glm::vec3* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TransformComponent>())
			return;

		TransformComponent& entity_transform = entity.GetComponent<TransformComponent>();

		*ref = entity_transform.GetRightDirection();
	}

#pragma endregion

#pragma region Tag Component

	MonoString* ScriptConnector::Tag_GetTag(UUID entityID)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity)
			return mono_string_new(mono_domain_get(), "");

		if (!entity.HasComponent<TagComponent>())
			return mono_string_new(mono_domain_get(), "");

		auto& component = entity.GetComponent<TagComponent>();
		return mono_string_new(mono_domain_get(), component.Tag.c_str());
	}

	void ScriptConnector::Tag_SetTag(UUID entityID, MonoString* ref)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<TagComponent>())
			return;

		auto& component = entity.GetComponent<TagComponent>();
		component.Tag = ScriptingUtils::MonoStringToString(ref);
	}

#pragma endregion

#pragma region Script Component

	//MonoString* ScriptConnector::Script_GetScriptName(UUID entityID)
	//{
	//	Scene* scene = ScriptManager::GetSceneContext();
	//	L_CORE_ASSERT(scene, "Scene Not Valid.");

	//	Entity entity = scene->FindEntityByUUID(entityID);
	//	if (!entity) return;

	//	if (!entity.HasComponent<ScriptComponent>())
	//		return mono_string_new(mono_domain_get(), "");

	//	auto& component = entity.GetComponent<ScriptComponent>();
	//	return mono_string_new(mono_domain_get(), component.ScriptName.c_str());
	//}

	//void ScriptConnector::Script_SetScriptName(UUID entityID, MonoString* ref)
	//{
	//	Scene* scene = ScriptManager::GetSceneContext();
	//	L_CORE_ASSERT(scene, "Scene Not Valid.");

	//	Entity entity = scene->FindEntityByUUID(entityID);
	//	if (!entity) return;

	//	if (!entity.HasComponent<ScriptComponent>())
	//		return;

	//	auto& component = entity.GetComponent<ScriptComponent>();
	//	component.ScriptName = ScriptingUtils::MonoStringToString(ref);
	//}

#pragma endregion

#pragma region Point Light Component

	void ScriptConnector::PointLightComponent_GetActive(UUID entityID, bool* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		*out = entity.GetComponent<PointLightComponent>().Active;
	}

	void ScriptConnector::PointLightComponent_SetActive(UUID entityID, bool* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity)
			return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		entity.GetComponent<PointLightComponent>().Active = *ref;
	}

	void ScriptConnector::PointLightComponent_GetColour(UUID entityID, glm::vec4* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		*out = entity.GetComponent<PointLightComponent>().Colour;
	}

	void ScriptConnector::PointLightComponent_SetColour(UUID entityID, glm::vec4* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		entity.GetComponent<PointLightComponent>().Colour = *ref;
	}

	void ScriptConnector::PointLightComponent_GetRadius(UUID entityID, float* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		*out = entity.GetComponent<PointLightComponent>().Radius;
	}

	void ScriptConnector::PointLightComponent_SetRadius(UUID entityID, float* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		entity.GetComponent<PointLightComponent>().Radius = *ref;
	}

	void ScriptConnector::PointLightComponent_GetIntensity(UUID entityID, float* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		*out = entity.GetComponent<PointLightComponent>().Intensity;
	}

	void ScriptConnector::PointLightComponent_SetIntensity(UUID entityID, float* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		entity.GetComponent<PointLightComponent>().Intensity = *ref;
	}

	void ScriptConnector::PointLightComponent_GetShadowFlag(UUID entityID, uint8_t* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		*out = static_cast<uint8_t>(entity.GetComponent<PointLightComponent>().ShadowFlag);
	}

	void ScriptConnector::PointLightComponent_SetShadowFlag(UUID entityID, uint8_t* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<PointLightComponent>())
			return;

		entity.GetComponent<PointLightComponent>().ShadowFlag = static_cast<ShadowTypeFlag>(*ref);
	}

#pragma endregion

#pragma region Spot Light Component

	void ScriptConnector::SpotLightComponent_GetActive(UUID entityID, bool* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		*out = entity.GetComponent<SpotLightComponent>().Active;
	}

	void ScriptConnector::SpotLightComponent_SetActive(UUID entityID, bool* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		entity.GetComponent<SpotLightComponent>().Active = *ref;
	}

	void ScriptConnector::SpotLightComponent_GetColour(UUID entityID, glm::vec4* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		*out = entity.GetComponent<SpotLightComponent>().Colour;
	}

	void ScriptConnector::SpotLightComponent_SetColour(UUID entityID, glm::vec4* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		entity.GetComponent<SpotLightComponent>().Colour = *ref;
	}

	void ScriptConnector::SpotLightComponent_GetRange(UUID entityID, float* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		*out = entity.GetComponent<SpotLightComponent>().Range;
	}

	void ScriptConnector::SpotLightComponent_SetRange(UUID entityID, float* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		entity.GetComponent<SpotLightComponent>().Range = *ref;
	}

	void ScriptConnector::SpotLightComponent_GetAngle(UUID entityID, float* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		*out = entity.GetComponent<SpotLightComponent>().Angle;
	}

	void ScriptConnector::SpotLightComponent_SetAngle(UUID entityID, float* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		entity.GetComponent<SpotLightComponent>().Angle = *ref;
	}

	void ScriptConnector::SpotLightComponent_GetIntensity(UUID entityID, float* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		*out = entity.GetComponent<SpotLightComponent>().Intensity;
	}

	void ScriptConnector::SpotLightComponent_SetIntensity(UUID entityID, float* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		entity.GetComponent<SpotLightComponent>().Intensity = *ref;
	}

	void ScriptConnector::SpotLightComponent_GetShadowFlag(UUID entityID, ShadowTypeFlag* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		*out = entity.GetComponent<SpotLightComponent>().ShadowFlag;
	}

	void ScriptConnector::SpotLightComponent_SetShadowFlag(UUID entityID, ShadowTypeFlag* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SpotLightComponent>())
			return;

		entity.GetComponent<SpotLightComponent>().ShadowFlag = *ref;
	}

#pragma endregion

#pragma region Directional Light Component

	void ScriptConnector::DirectionalLightComponent_GetActive(UUID entityID, bool* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<DirectionalLightComponent>())
			return;

		*out = entity.GetComponent<DirectionalLightComponent>().Active;
	}

	void ScriptConnector::DirectionalLightComponent_SetActive(UUID entityID, bool* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<DirectionalLightComponent>())
			return;

		entity.GetComponent<DirectionalLightComponent>().Active = *ref;
	}

	void ScriptConnector::DirectionalLightComponent_GetColour(UUID entityID, glm::vec4* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<DirectionalLightComponent>())
			return;

		*out = entity.GetComponent<DirectionalLightComponent>().Colour;
	}

	void ScriptConnector::DirectionalLightComponent_SetColour(UUID entityID, glm::vec4* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<DirectionalLightComponent>())
			return;

		entity.GetComponent<DirectionalLightComponent>().Colour = *ref;
	}

	void ScriptConnector::DirectionalLightComponent_GetIntensity(UUID entityID, float* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<DirectionalLightComponent>())
			return;

		*out = entity.GetComponent<DirectionalLightComponent>().Intensity;
	}

	void ScriptConnector::DirectionalLightComponent_SetIntensity(UUID entityID, float* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<DirectionalLightComponent>())
			return;

		entity.GetComponent<DirectionalLightComponent>().Intensity = *ref;
	}

	void ScriptConnector::DirectionalLightComponent_GetShadowFlag(UUID entityID, ShadowTypeFlag* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<DirectionalLightComponent>())
			return;

		*out = entity.GetComponent<DirectionalLightComponent>().ShadowFlag;
	}

	void ScriptConnector::DirectionalLightComponent_SetShadowFlag(UUID entityID, ShadowTypeFlag* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<DirectionalLightComponent>())
			return;

		entity.GetComponent<DirectionalLightComponent>().ShadowFlag = *ref;
	}

#pragma endregion

#pragma region RigidbodyComponent Component

	void ScriptConnector::Rigidbody_GetMass(UUID entityID, float* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		*out = entity.GetComponent<RigidbodyComponent>().GetMass();
	}

	void ScriptConnector::Rigidbody_SetMass(UUID entityID, float* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().SetMass(*ref);
	}

	void ScriptConnector::Rigidbody_GetDrag(UUID entityID, float* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		*out = entity.GetComponent<RigidbodyComponent>().GetDrag();
	}

	void ScriptConnector::Rigidbody_SetDrag(UUID entityID, float* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().SetDrag(*ref);
	}

	void ScriptConnector::Rigidbody_GetAngularDrag(UUID entityID, float* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		*out = entity.GetComponent<RigidbodyComponent>().GetAngularDrag();
	}

	void ScriptConnector::Rigidbody_SetAngularDrag(UUID entityID, float* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().SetAngularDrag(*ref);
	}

	void ScriptConnector::Rigidbody_GetAutomaticCentreOfMass(UUID entityID, bool* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		*out = entity.GetComponent<RigidbodyComponent>().IsAutomaticCentreOfMassEnabled();
	}

	void ScriptConnector::Rigidbody_SetAutomaticCentreOfMass(UUID entityID, bool* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().SetAutomaticCentreOfMass(*ref);
	}

	void ScriptConnector::Rigidbody_GetUseGravity(UUID entityID, bool* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		*out = entity.GetComponent<RigidbodyComponent>().IsGravityEnabled();
	}

	void ScriptConnector::Rigidbody_SetUseGravity(UUID entityID, bool* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().SetGravity(*ref);
	}

	void ScriptConnector::Rigidbody_GetIsKinematic(UUID entityID, bool* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		*out = entity.GetComponent<RigidbodyComponent>().IsKinematicEnabled();
	}

	void ScriptConnector::Rigidbody_SetIsKinematic(UUID entityID, bool* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().SetKinematic(*ref);
	}

	void ScriptConnector::Rigidbody_GetPositionConstraint(UUID entityID, glm::bvec3* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		*out = entity.GetComponent<RigidbodyComponent>().GetPositionConstraint();
	}

	void ScriptConnector::Rigidbody_SetPositionConstraint(UUID entityID, glm::bvec3* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().SetPositionConstraint(*ref);
	}

	void ScriptConnector::Rigidbody_GetRotationConstraint(UUID entityID, glm::bvec3* out) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		*out = entity.GetComponent<RigidbodyComponent>().GetRotationConstraint();
	}

	void ScriptConnector::Rigidbody_SetRotationConstraint(UUID entityID, glm::bvec3* ref) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().SetRotationConstraint(*ref);
	}

	void ScriptConnector::Rigidbody_ApplyForce(UUID entityID, glm::vec3* force, PxForceMode::Enum* forceMode) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().ApplyForce(*force, *forceMode);
	}

	void ScriptConnector::Rigidbody_ApplyTorque(UUID entityID, glm::vec3* torque) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<RigidbodyComponent>())
			return;

		entity.GetComponent<RigidbodyComponent>().ApplyTorque(*torque);
	}

#pragma endregion

#pragma region Box Collider Component

	void ScriptConnector::BoxColliderComponent_GetIsTrigger(UUID entityID, bool* result) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<BoxColliderComponent>())
			return;

		*result = entity.GetComponent<BoxColliderComponent>().IsTrigger();
	}

	void ScriptConnector::BoxColliderComponent_SetIsTrigger(UUID entityID, bool* reference) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<BoxColliderComponent>())
			return;

		entity.GetComponent<BoxColliderComponent>().SetIsTrigger(*reference);
	}

	void ScriptConnector::BoxColliderComponent_GetCentre(UUID entityID, glm::vec3* result) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<BoxColliderComponent>())
			return;

		*result = entity.GetComponent<BoxColliderComponent>().GetCentre();
	}

	void ScriptConnector::BoxColliderComponent_SetCentre(UUID entityID, glm::vec3* reference) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<BoxColliderComponent>())
			return;

		entity.GetComponent<BoxColliderComponent>().SetCentre(*reference);
	}

	void ScriptConnector::BoxColliderComponent_GetSize(UUID entityID, glm::vec3* result) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<BoxColliderComponent>())
			return;

		*result = entity.GetComponent<BoxColliderComponent>().GetSize();
	}

	void ScriptConnector::BoxColliderComponent_SetSize(UUID entityID, glm::vec3* reference) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<BoxColliderComponent>())
			return;

		entity.GetComponent<BoxColliderComponent>().SetSize(*reference);
	}

	void ScriptConnector::BoxColliderComponent_GetMaterial(UUID entityID, _PhysicsMaterial* result) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<BoxColliderComponent>())
			return;

		auto material = entity.GetComponent<BoxColliderComponent>().GetMaterial();
		result->m_DynamicFriction = material->GetDynamicFriction();
		result->m_StaticFriction = material->GetStaticFriction();
		result->m_Bounciness = material->GetBounciness();
	}

	void ScriptConnector::BoxColliderComponent_SetMaterial(UUID entityID, _PhysicsMaterial* reference) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<BoxColliderComponent>())
			return;

		auto material = entity.GetComponent<BoxColliderComponent>().GetMaterial();
		material->SetDynamicFriction(reference->m_DynamicFriction);
		material->SetStaticFriction(reference->m_StaticFriction);
		material->SetBounciness(reference->m_Bounciness);
	}


#pragma endregion

#pragma region Sphere Collider Component

	void ScriptConnector::SphereColliderComponent_GetIsTrigger(UUID entityID, bool* result) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SphereColliderComponent>())
			return;

		*result = entity.GetComponent<SphereColliderComponent>().IsTrigger();
	}

	void ScriptConnector::SphereColliderComponent_SetIsTrigger(UUID entityID, bool* reference) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SphereColliderComponent>())
			return;

		entity.GetComponent<SphereColliderComponent>().SetIsTrigger(*reference);
	}

	void ScriptConnector::SphereColliderComponent_GetCentre(UUID entityID, glm::vec3* result) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SphereColliderComponent>())
			return;

		*result = entity.GetComponent<SphereColliderComponent>().GetCentre();
	}

	void ScriptConnector::SphereColliderComponent_SetCentre(UUID entityID, glm::vec3* reference) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SphereColliderComponent>())
			return;

		entity.GetComponent<SphereColliderComponent>().SetCentre(*reference);
	}

	void ScriptConnector::SphereColliderComponent_GetRadius(UUID entityID, float* result) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SphereColliderComponent>())
			return;

		*result = entity.GetComponent<SphereColliderComponent>().GetRadius();
	}

	void ScriptConnector::SphereColliderComponent_SetRadius(UUID entityID, float* reference) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SphereColliderComponent>())
			return;

		entity.GetComponent<SphereColliderComponent>().SetRadius(*reference);
	}

	void ScriptConnector::SphereColliderComponent_GetMaterial(UUID entityID, _PhysicsMaterial* result) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SphereColliderComponent>())
			return;

		auto material = entity.GetComponent<SphereColliderComponent>().GetMaterial();
		result->m_DynamicFriction = material->GetDynamicFriction();
		result->m_StaticFriction = material->GetStaticFriction();
		result->m_Bounciness = material->GetBounciness();
	}

	void ScriptConnector::SphereColliderComponent_SetMaterial(UUID entityID, _PhysicsMaterial* reference) {
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<SphereColliderComponent>())
			return;

		auto material = entity.GetComponent<SphereColliderComponent>().GetMaterial();
		material->SetDynamicFriction(reference->m_DynamicFriction);
		material->SetStaticFriction(reference->m_StaticFriction);
		material->SetBounciness(reference->m_Bounciness);
	}

#pragma endregion

#pragma region Compute Shader

	void ScriptConnector::ComputeShader_SetBuffer(AssetHandle asset_handle, ComputeBuffer* buffer, uint32_t binding_index)
	{
		if (!buffer)
			return;

		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
			shader->SetBuffer(buffer, binding_index);
	}

	void ScriptConnector::ComputeShader_Dispatch(AssetHandle asset_handle, uint32_t x, uint32_t y, uint32_t z)
	{
		if (x <= 0) x = 1;
		if (y <= 0) y = 1;
		if (z <= 0) z = 1;

		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
			shader->Dispatch(x, y, z);
	}

	void ScriptConnector::ComputeShader_SetBool(AssetHandle asset_handle, MonoString* name, bool value)
	{
		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
		{
			shader->GetShader()->Bind();
			shader->GetShader()->SetBool(ScriptingUtils::MonoStringToString(name).c_str(), value);
		}
	}

	void ScriptConnector::ComputeShader_SetInt(AssetHandle asset_handle, MonoString* name, int32_t value)
	{
		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
		{
			shader->GetShader()->Bind();
			shader->GetShader()->SetInt(ScriptingUtils::MonoStringToString(name).c_str(), value);
		}
	}

	void ScriptConnector::ComputeShader_SetUInt(AssetHandle asset_handle, MonoString* name, uint32_t value)
	{
		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
		{
			shader->GetShader()->Bind();
			shader->GetShader()->SetUInt(ScriptingUtils::MonoStringToString(name).c_str(), value);
		}
	}

	void ScriptConnector::ComputeShader_SetFloat(AssetHandle asset_handle, MonoString* name, float value)
	{
		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
		{
			shader->GetShader()->Bind();
			shader->GetShader()->SetFloat(ScriptingUtils::MonoStringToString(name).c_str(), value);
		}
	}

	void ScriptConnector::ComputeShader_SetVector2(AssetHandle asset_handle, MonoString* name, glm::vec2 value)
	{
		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
		{
			shader->GetShader()->Bind();
			shader->GetShader()->SetVec2(ScriptingUtils::MonoStringToString(name).c_str(), value);
		}
	}

	void ScriptConnector::ComputeShader_SetVector3(AssetHandle asset_handle, MonoString* name, glm::vec3 value)
	{
		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
		{
			shader->GetShader()->Bind();
			shader->GetShader()->SetVec3(ScriptingUtils::MonoStringToString(name).c_str(), value);
		}
	}

	void ScriptConnector::ComputeShader_SetVector4(AssetHandle asset_handle, MonoString* name, glm::vec4 value)
	{
		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
		{
			shader->GetShader()->Bind();
			shader->GetShader()->SetVec4(ScriptingUtils::MonoStringToString(name).c_str(), value);
		}
	}

#pragma endregion

#pragma region Compute Buffer

	ComputeBuffer* ScriptConnector::ComputeBuffer_Create(int element_count, int element_size)
	{
		return new ComputeBuffer(element_count, element_size);
	}

	void ScriptConnector::ComputeBuffer_SetData(ComputeBuffer* buffer, void* data, int element_count, int element_size) 
	{
		if (!buffer)
			return;

		buffer->SetData(data, element_count, element_size);
	}

	void ScriptConnector::ComputeBuffer_GetData(ComputeBuffer* buffer, void* output, int element_count, int element_size)
	{
		if (!buffer)
			return;

		buffer->GetData(output, element_count, element_size);
	}

	void ScriptConnector::ComputeBuffer_Release(ComputeBuffer* buffer) 
	{
		if (!buffer)
			return;

		buffer->Release();
		delete buffer;  // Delete the buffer object
	}

#pragma endregion

#pragma region Component Register

	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypename = fmt::format("Louron.{}", structName);

				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptManager::GetCoreAssemblyImage());
				if (!managedType)
				{
					L_CORE_WARN("Could not find component type {}", managedTypename);
					return;
				}
				s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };

			}(), ...);
	}

	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptConnector::RegisterComponents() {

		s_EntityHasComponentFuncs.clear();
		RegisterComponent(AllComponents{});
	}

#pragma endregion


}

