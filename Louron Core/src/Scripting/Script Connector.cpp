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
		mono_add_internal_call("Louron.EngineCallbacks::Entity_Instantiate_POS_ROT_SCALE", Entity_Instantiate_POS_ROT_SCALE);

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

		mono_add_internal_call("Louron.EngineCallbacks::Material_Create", Material_Create);
		mono_add_internal_call("Louron.EngineCallbacks::Material_SetShader", Material_SetShader);
		mono_add_internal_call("Louron.EngineCallbacks::Material_Destroy", Material_Destroy);
		
		mono_add_internal_call("Louron.EngineCallbacks::MeshRendererComponent_GetMaterial", MeshRendererComponent_GetMaterial);
		mono_add_internal_call("Louron.EngineCallbacks::MeshRendererComponent_SetMaterial", MeshRendererComponent_SetMaterial);
		mono_add_internal_call("Louron.EngineCallbacks::MeshRendererComponent_GetMaterials", MeshRendererComponent_GetMaterials);
		mono_add_internal_call("Louron.EngineCallbacks::MeshRendererComponent_SetMaterials", MeshRendererComponent_SetMaterials);
		
		mono_add_internal_call("Louron.EngineCallbacks::MeshRenderer_EnableUniformBlock", MeshRenderer_EnableUniformBlock);
		mono_add_internal_call("Louron.EngineCallbacks::MeshRenderer_GetUniformBlock", MeshRenderer_GetUniformBlock);
		mono_add_internal_call("Louron.EngineCallbacks::MeshRenderer_DisableUniformBlock", MeshRenderer_DisableUniformBlock);
		mono_add_internal_call("Louron.EngineCallbacks::MeshRenderer_EnableAllUniformBlocks", MeshRenderer_EnableAllUniformBlocks);
		mono_add_internal_call("Louron.EngineCallbacks::MeshRenderer_DisableAllUniformBlocks", MeshRenderer_DisableAllUniformBlocks);

		mono_add_internal_call("Louron.EngineCallbacks::MaterialUniformBlock_SetUniform", MaterialUniformBlock_SetUniform);
		
		mono_add_internal_call("Louron.EngineCallbacks::MaterialUniformBlock_OverrideAlbedoMap", MaterialUniformBlock_OverrideAlbedoMap);
		mono_add_internal_call("Louron.EngineCallbacks::MaterialUniformBlock_OverrideMetallicMap", MaterialUniformBlock_OverrideMetallicMap);
		mono_add_internal_call("Louron.EngineCallbacks::MaterialUniformBlock_OverrideNormalMap", MaterialUniformBlock_OverrideNormalMap);
		mono_add_internal_call("Louron.EngineCallbacks::MaterialUniformBlock_OverrideAlbedoTint", MaterialUniformBlock_OverrideAlbedoTint);
		mono_add_internal_call("Louron.EngineCallbacks::MaterialUniformBlock_OverrideMetallic", MaterialUniformBlock_OverrideMetallic);
		mono_add_internal_call("Louron.EngineCallbacks::MaterialUniformBlock_OverrideRoughness", MaterialUniformBlock_OverrideRoughness);
		
		mono_add_internal_call("Louron.EngineCallbacks::Texture2D_Create", Texture2D_Create);
		mono_add_internal_call("Louron.EngineCallbacks::Texture2D_CreateWithData", Texture2D_CreateWithData);
		mono_add_internal_call("Louron.EngineCallbacks::Texture2D_SetPixel", Texture2D_SetPixel);
		mono_add_internal_call("Louron.EngineCallbacks::Texture2D_SetPixelData", Texture2D_SetPixelData);
		mono_add_internal_call("Louron.EngineCallbacks::Texture2D_SubmitTextureChanges", Texture2D_SubmitTextureChanges);
		mono_add_internal_call("Louron.EngineCallbacks::Texture2D_Destroy", Texture2D_Destroy);

		mono_add_internal_call("Louron.EngineCallbacks::Mesh_CreateNewMesh",				Mesh_CreateNewMesh);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_SubmitChanges",				Mesh_SubmitChanges);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_CopyBufferDataToCPU",			Mesh_CopyBufferDataToCPU);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_ClearBufferDataFromCPU",		Mesh_ClearBufferDataFromCPU);
		
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_SetTriangles",					Mesh_SetTriangles);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_GetTriangles",					Mesh_GetTriangles);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_RecalculateNormals",			Mesh_RecalculateNormals);
		
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_SetVertices",					Mesh_SetVertices);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_SetNormals",					Mesh_SetNormals);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_SetTextureCoords",				Mesh_SetTextureCoords);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_SetTangents",					Mesh_SetTangents);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_SetBitangents",				Mesh_SetBitangents);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_GetVertices",					Mesh_GetVertices);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_GetNormals",					Mesh_GetNormals);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_GetTextureCoords",				Mesh_GetTextureCoords);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_GetTangents",					Mesh_GetTangents);
		mono_add_internal_call("Louron.EngineCallbacks::Mesh_GetBitangents",				Mesh_GetBitangents);
		mono_add_internal_call("Louron.EngineCallbacks::MeshFilterComponent_SharedMesh",	MeshFilterComponent_SharedMesh);
		mono_add_internal_call("Louron.EngineCallbacks::MeshFilterComponent_CopyMesh",		MeshFilterComponent_CopyMesh);
		mono_add_internal_call("Louron.EngineCallbacks::MeshFilterComponent_SetMesh",		MeshFilterComponent_SetMesh);
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
		if (component_name == "Louron.MeshFilterComponent") {
			entity.AddComponent<MeshFilterComponent>();
		}
		if (component_name == "Louron.MeshRendererComponent") {
			auto& component = entity.AddComponent<MeshRendererComponent>();
			component.MeshRendererMaterialHandles.push_back({ AssetManager::GetInbuiltAsset<Material>("Default_Material", AssetType::Material_Standard)->Handle, nullptr });
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

	void ScriptConnector::Entity_Instantiate_POS_ROT_SCALE(UUID entityID, uint32_t* handle, glm::vec3* position, glm::vec3* rotation, glm::vec3* scale, uint32_t* prefab_clone_uuid)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity)
			return;

		auto prefab_asset = AssetManager::GetAsset<Prefab>(*handle);

		TransformComponent transform{};
		transform.m_Position = *position;
		transform.m_Rotation = *rotation;
		transform.m_Scale = *scale;

		Entity prefab_clone = scene->InstantiatePrefab(prefab_asset, transform);
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

		TransformComponent& entity_transform = entity.GetTransform();
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

		TransformComponent& entity_transform = entity.GetTransform();
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

		TransformComponent& entity_transform = entity.GetTransform();

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

		TransformComponent& entity_transform = entity.GetTransform();

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

		TransformComponent& entity_transform = entity.GetTransform();

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

		TransformComponent& entity_transform = entity.GetTransform();

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

		TransformComponent& entity_transform = entity.GetTransform();

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

		TransformComponent& entity_transform = entity.GetTransform();

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

		TransformComponent& entity_transform = entity.GetTransform();

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

		TransformComponent& entity_transform = entity.GetTransform();

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

		TransformComponent& entity_transform = entity.GetTransform();

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

		TransformComponent& entity_transform = entity.GetTransform();

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
			shader->GetShader()->SetFloatVec2(ScriptingUtils::MonoStringToString(name).c_str(), value);
		}
	}

	void ScriptConnector::ComputeShader_SetVector3(AssetHandle asset_handle, MonoString* name, glm::vec3 value)
	{
		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
		{
			shader->GetShader()->Bind();
			shader->GetShader()->SetFloatVec3(ScriptingUtils::MonoStringToString(name).c_str(), value);
		}
	}

	void ScriptConnector::ComputeShader_SetVector4(AssetHandle asset_handle, MonoString* name, glm::vec4 value)
	{
		if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
		{
			shader->GetShader()->Bind();
			shader->GetShader()->SetFloatVec4(ScriptingUtils::MonoStringToString(name).c_str(), value);
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

#pragma region Material

	AssetHandle ScriptConnector::Material_Create(MonoString* name)
	{
		std::shared_ptr<Material> material = std::make_shared<Material>();
		return AssetManager::AddRuntimeAsset<Material>(material, ScriptingUtils::MonoStringToString(name));
	}

	void ScriptConnector::Material_SetShader(AssetHandle asset_handle, AssetHandle shader_handle)
	{
		AssetManager::GetAsset<Material>(asset_handle)->SetShader(shader_handle);
	}

	void ScriptConnector::Material_Destroy(AssetHandle asset_handle)
	{
		AssetManager::RemoveRuntimeAsset(asset_handle);
	}

#pragma endregion

#pragma region MeshRendererComponent


	uint32_t ScriptConnector::MeshRendererComponent_GetMaterial(UUID entityID)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return NULL_UUID;

		if (!entity.HasComponent<MeshRendererComponent>())
			return NULL_UUID;

		return entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles.back().first;
	}

	void ScriptConnector::MeshRendererComponent_SetMaterial(UUID entityID, AssetHandle material_handle)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<MeshRendererComponent>() || // If No Component
			 entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles.back().first == material_handle) // If Material Handle Already Set
			return;

		if (AssetManager::IsAssetHandleValid(material_handle)) // Check validity of asset
		{
			auto& material_pair = entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles.back();

			material_pair.first = material_handle;

			material_pair.second.reset();   // Clear Material Uniform Block
			material_pair.second = nullptr; // Clear Material Uniform Block
		}

		return;
	}

	uint32_t* ScriptConnector::MeshRendererComponent_GetMaterials(UUID entityID)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return new uint32_t[1](NULL_UUID);

		if (!entity.HasComponent<MeshRendererComponent>())
			return new uint32_t[1](NULL_UUID);

		auto& component = entity.GetComponent<MeshRendererComponent>();

		const size_t size = component.MeshRendererMaterialHandles.size();
		uint32_t* material_array = new uint32_t[size](NULL_UUID);
		for (int i = 0; i < size; i++)
			material_array[i] = (uint32_t)component.MeshRendererMaterialHandles[i].first;

		return material_array;
	}

	void ScriptConnector::MeshRendererComponent_SetMaterials(UUID entityID, uint32_t* material_handles, uint32_t num_elements)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<MeshRendererComponent>())
			return;

		auto& component = entity.GetComponent<MeshRendererComponent>();

		std::vector<std::pair<AssetHandle, std::shared_ptr<MaterialUniformBlock>>> new_handle_vector;
		
		for (uint32_t i = 0; i < num_elements; i++)
		{
			// Default to nullptr
			std::shared_ptr<MaterialUniformBlock> uniform_block = nullptr;

			// Find existing material uniform block if it exists
			auto it = std::find_if(component.MeshRendererMaterialHandles.begin(),
				component.MeshRendererMaterialHandles.end(),
				[&](const std::pair<AssetHandle, std::shared_ptr<MaterialUniformBlock>>& pair)
				{ return pair.first == material_handles[i]; });

			if (it != component.MeshRendererMaterialHandles.end())
				uniform_block = it->second;

			// Add to new vector
			new_handle_vector.emplace_back(material_handles[i], uniform_block);
		}

		component.MeshRendererMaterialHandles = std::move(new_handle_vector);
	}

	void ScriptConnector::MeshRenderer_EnableUniformBlock(UUID entityID, uint32_t material_index)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<MeshRendererComponent>())
			return;

		auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles;

		if (material_index == -1)
			material_index = (uint32_t)material_handle_vector.size() - 1;

		if (material_index >= material_handle_vector.size())
			return;

		auto material_asset = AssetManager::GetAsset<Material>(material_handle_vector[material_index].first);

		if (!material_asset)
			return;

		if (!material_handle_vector[material_index].second)
		{
			material_handle_vector[material_index].second = std::make_shared<MaterialUniformBlock>(*material_asset->GetUniformBlock());
			material_handle_vector[material_index].second->GenerateNewBlockID();
		}

		return;
	}

	MaterialUniformBlock* ScriptConnector::MeshRenderer_GetUniformBlock(UUID entityID, uint32_t material_index)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return nullptr;

		if (!entity.HasComponent<MeshRendererComponent>())
			return nullptr;

		auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles;

		if (material_index == -1)
			material_index = (uint32_t)material_handle_vector.size() - 1;

		if (material_index >= material_handle_vector.size())
			return nullptr;

		return material_handle_vector[material_index].second.get();
	}

	void ScriptConnector::MeshRenderer_DisableUniformBlock(UUID entityID, uint32_t material_index)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<MeshRendererComponent>())
			return;

		auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles;

		if (material_index == -1)
			material_index = (uint32_t)material_handle_vector.size() - 1;

		if (material_index >= material_handle_vector.size())
			return;
		
		material_handle_vector[material_index].second.reset();
		material_handle_vector[material_index].second = nullptr;
	}

	void ScriptConnector::MeshRenderer_EnableAllUniformBlocks(UUID entityID)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<MeshRendererComponent>())
			return;

		auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles;

		for(auto& [material_handle, uniform_block] : material_handle_vector)
		{
			auto material_asset = AssetManager::GetAsset<Material>(material_handle);

			if (!material_asset)
				continue;

			if (!uniform_block)
				uniform_block = std::make_shared<MaterialUniformBlock>(*material_asset->GetUniformBlock());
		}

		return;
	}

	void ScriptConnector::MeshRenderer_DisableAllUniformBlocks(UUID entityID)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");

		Entity entity = scene->FindEntityByUUID(entityID);
		if (!entity) return;

		if (!entity.HasComponent<MeshRendererComponent>())
			return;

		auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MeshRendererMaterialHandles;

		for (auto& [material_handle, uniform_block] : material_handle_vector)
		{
			uniform_block.reset();
			uniform_block = nullptr;
		}

		return;
	}

#pragma endregion

#pragma region MaterialUniformBlock

	void ScriptConnector::MaterialUniformBlock_SetUniform(MaterialUniformBlock* uniform_block, MonoString* uniform_name, uint32_t type, void* value)
	{
		if (!uniform_block || !value)
			return;

		switch (type)
		{
			// Bool
			case (uint32_t)GLSLType::Bool:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Bool,			*static_cast<bool*>(value)); return;
			case (uint32_t)GLSLType::BVec2:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::BVec2,		*static_cast<glm::bvec2*>(value)); return;
			case (uint32_t)GLSLType::BVec3:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::BVec3,		*static_cast<glm::bvec3*>(value)); return;
			case (uint32_t)GLSLType::BVec4:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::BVec4,		*static_cast<glm::bvec4*>(value)); return;

			// Int
			case (uint32_t)GLSLType::Int:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Int,			*static_cast<int*>(value)); return;
			case (uint32_t)GLSLType::IVec2:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::IVec2,		*static_cast<glm::ivec2*>(value)); return;
			case (uint32_t)GLSLType::IVec3:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::IVec3,		*static_cast<glm::ivec3*>(value)); return;
			case (uint32_t)GLSLType::IVec4:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::IVec4,		*static_cast<glm::ivec4*>(value)); return;

			// Int
			case (uint32_t)GLSLType::Uint:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Uint,			*static_cast<unsigned int*>(value)); return;
			case (uint32_t)GLSLType::UVec2:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::UVec2,		*static_cast<glm::uvec2*>(value)); return;
			case (uint32_t)GLSLType::UVec3:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::UVec3,		*static_cast<glm::uvec3*>(value)); return;
			case (uint32_t)GLSLType::UVec4:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::UVec4,		*static_cast<glm::uvec4*>(value)); return;

			// Float
			case (uint32_t)GLSLType::Float:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Float,		*static_cast<float*>(value)); return;
			case (uint32_t)GLSLType::Vec2:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Vec2,			*static_cast<glm::vec2*>(value)); return;
			case (uint32_t)GLSLType::Vec3:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Vec3,			*static_cast<glm::vec3*>(value)); return;
			case (uint32_t)GLSLType::Vec4:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Vec4,			*static_cast<glm::vec4*>(value)); return;

			// Double
			case (uint32_t)GLSLType::Double:	uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Double,		*static_cast<double*>(value)); return;
			case (uint32_t)GLSLType::DVec2:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::DVec2,		*static_cast<glm::dvec2*>(value)); return;
			case (uint32_t)GLSLType::DVec3:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::DVec3,		*static_cast<glm::dvec3*>(value)); return;
			case (uint32_t)GLSLType::DVec4:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::DVec4,		*static_cast<glm::dvec4*>(value)); return;


			case (uint32_t)GLSLType::Mat2:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Mat2,			*static_cast<glm::mat2*>(value)); return;
			case (uint32_t)GLSLType::Mat3:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Mat3,			*static_cast<glm::mat3*>(value)); return;
			case (uint32_t)GLSLType::Mat4:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Mat4,			*static_cast<glm::mat4*>(value)); return;

			case (uint32_t)GLSLType::Sampler1D:					uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Sampler1D,				*static_cast<AssetHandle*>(value)); return;
			case (uint32_t)GLSLType::Sampler1DArray:			uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Sampler1DArray,			*static_cast<AssetHandle*>(value)); return;
			case (uint32_t)GLSLType::Sampler1DShadow:			uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Sampler1DShadow,			*static_cast<AssetHandle*>(value)); return;
			case (uint32_t)GLSLType::Sampler1DArrayShadow:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Sampler1DArrayShadow,		*static_cast<AssetHandle*>(value)); return;

			case (uint32_t)GLSLType::Sampler2D:					uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Sampler2D,				*static_cast<AssetHandle*>(value)); return;
			case (uint32_t)GLSLType::Sampler2DArray:			uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Sampler2DArray,			*static_cast<AssetHandle*>(value)); return;
			case (uint32_t)GLSLType::Sampler2DShadow:			uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Sampler2DShadow,			*static_cast<AssetHandle*>(value)); return;
			case (uint32_t)GLSLType::Sampler2DArrayShadow:		uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Sampler2DArrayShadow,		*static_cast<AssetHandle*>(value)); return;

			case (uint32_t)GLSLType::Sampler3D:					uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::Sampler3D,				*static_cast<AssetHandle*>(value)); return;

			case (uint32_t)GLSLType::SamplerCube:				uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::SamplerCube,				*static_cast<AssetHandle*>(value)); return;
			case (uint32_t)GLSLType::SamplerCubeArray:			uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::SamplerCubeArray,			*static_cast<AssetHandle*>(value)); return;
			case (uint32_t)GLSLType::SamplerCubeShadow:			uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::SamplerCubeShadow,		*static_cast<AssetHandle*>(value)); return;
			case (uint32_t)GLSLType::SamplerCubeArrayShadow:	uniform_block->SetUniform(ScriptingUtils::MonoStringToString(uniform_name), GLSLType::SamplerCubeArrayShadow,	*static_cast<AssetHandle*>(value)); return;

		}
		return;
	}

	void ScriptConnector::MaterialUniformBlock_OverrideAlbedoMap(MaterialUniformBlock* uniform_block, AssetHandle value)
	{
		if (!uniform_block)
			return;

		uniform_block->OverrideAlbedoMap(value);
	}

	void ScriptConnector::MaterialUniformBlock_OverrideMetallicMap(MaterialUniformBlock* uniform_block, AssetHandle value)
	{
		if (!uniform_block)
			return;

		uniform_block->OverrideMetallicMap(value);
	}

	void ScriptConnector::MaterialUniformBlock_OverrideNormalMap(MaterialUniformBlock* uniform_block, AssetHandle value)
	{
		if (!uniform_block)
			return;

		uniform_block->OverrideNormalMap(value);
	}

	void ScriptConnector::MaterialUniformBlock_OverrideAlbedoTint(MaterialUniformBlock* uniform_block, glm::vec4 value)
	{
		if (!uniform_block)
			return;

		uniform_block->OverrideAlbedoTint(value);
	}

	void ScriptConnector::MaterialUniformBlock_OverrideMetallic(MaterialUniformBlock* uniform_block, float value)
	{
		if (!uniform_block)
			return;

		uniform_block->OverrideMetallic(value);
	}

	void ScriptConnector::MaterialUniformBlock_OverrideRoughness(MaterialUniformBlock* uniform_block, float value)
	{
		if (!uniform_block)
			return;

		uniform_block->OverrideRoughness(value);
	}

#pragma endregion

#pragma region Texture2D

	uint32_t ScriptConnector::Texture2D_Create(int width, int height, Texture2D::TextureFormat internal_format)
	{
		std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(width, height, internal_format, true);
		return AssetManager::AddRuntimeAsset(texture, "Runtime Texture");
	}

	uint32_t ScriptConnector::Texture2D_CreateWithData(unsigned char* data, int width, int height, uint8_t internal_format, uint8_t data_format)
	{
		std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(data, width, height, static_cast<Texture2D::TextureFormat>(internal_format), static_cast<Texture2D::TextureFormat>(data_format), true);
		return AssetManager::AddRuntimeAsset(texture, "Runtime Texture");
	}

	void ScriptConnector::Texture2D_SetPixel(uint32_t asset_handle, glm::vec4 colour, glm::ivec2 pixel_coord)
	{
		if (!AssetManager::IsAssetHandleValid(asset_handle))
			return;

		const auto& texture_ref = AssetManager::GetAsset<Texture2D>(asset_handle);
		if (!texture_ref)
			return;

		texture_ref->SetPixel(colour, pixel_coord);
	}

	void ScriptConnector::Texture2D_SetPixelData(uint32_t asset_handle, unsigned char* pixel_data, int pixel_data_size, uint8_t pixel_data_format)
	{
		if (!AssetManager::IsAssetHandleValid(asset_handle))
			return;

		const auto& texture_ref = AssetManager::GetAsset<Texture2D>(asset_handle);
		if (!texture_ref)
			return;

		texture_ref->SetPixelData(pixel_data, pixel_data_size, static_cast<Texture2D::TextureFormat>(pixel_data_format));
	}

	void ScriptConnector::Texture2D_SubmitTextureChanges(uint32_t asset_handle)
	{
		if (!AssetManager::IsAssetHandleValid(asset_handle))
			return;

		const auto& texture_ref = AssetManager::GetAsset<Texture2D>(asset_handle);
		if (!texture_ref)
			return;

		texture_ref->SubmitTextureChanges();
	}

	void ScriptConnector::Texture2D_Destroy(AssetHandle handle)
	{
		AssetManager::RemoveRuntimeAsset(handle);
	}

#pragma endregion

#pragma region Mesh & MeshFilter

	uint32_t ScriptConnector::Mesh_CreateNewMesh()
	{

		std::shared_ptr<SubMesh> sub_mesh = std::make_shared<SubMesh>();

		sub_mesh->SetVAO(std::make_unique<VertexArray>());

		VertexBuffer* vbo_verts = new VertexBuffer(3);
		vbo_verts->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aPos"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_verts);

		VertexBuffer* vbo_norms = new VertexBuffer(3);
		vbo_norms->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aNormal"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_norms);

		VertexBuffer* vbo_texcoords = new VertexBuffer(2);
		vbo_texcoords->SetLayout(BufferLayout{ {ShaderDataType::Float2, "aTexCoord"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_texcoords);

		VertexBuffer* vbo_tangents = new VertexBuffer(3);
		vbo_tangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aTangent"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_tangents);

		VertexBuffer* vbo_bitangents = new VertexBuffer(3);
		vbo_bitangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aBitangent"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_bitangents);

		IndexBuffer* ebo = new IndexBuffer();
		sub_mesh->GetVAO()->SetIndexBuffer(ebo);

		std::shared_ptr<AssetMesh> mesh = std::make_shared<AssetMesh>();

		mesh->SubMeshes.push_back(std::move(sub_mesh));

		return AssetManager::AddRuntimeAsset(mesh, "New Runtime Mesh");
	}

	void ScriptConnector::Mesh_SubmitChanges(AssetHandle assetHandle, bool clearCPUData)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;
			
		mesh->SubMeshes.front()->SubmitChangesToGPU(clearCPUData);
	}

	void ScriptConnector::Mesh_CopyBufferDataToCPU(AssetHandle assetHandle)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;
			
		mesh->SubMeshes.front()->CopyGPUData();
	}

	void ScriptConnector::Mesh_ClearBufferDataFromCPU(AssetHandle assetHandle)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;
		
		mesh->SubMeshes.front()->ClearCPUData();
	}

	void ScriptConnector::Mesh_SetVertices(AssetHandle assetHandle, float* data, uint32_t dataLength)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle) || data == nullptr || dataLength == 0)
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);
		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		mesh->SubMeshes.front()->SetVertices(data, dataLength);

		for (uint32_t i = 0; i < dataLength; i += 3) // Assuming (x, y, z) ordering!!
		{
			glm::vec3 v(data[i], data[i + 1], data[i + 2]);
			mesh->MeshBounds.BoundsMin = glm::min(mesh->MeshBounds.BoundsMin, v);
			mesh->MeshBounds.BoundsMax = glm::max(mesh->MeshBounds.BoundsMax, v);
		}

		mesh->ModifiedAABB = true;
	}

	void ScriptConnector::Mesh_SetNormals(AssetHandle assetHandle, float* data, uint32_t dataLength)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;
			
		mesh->SubMeshes.front()->SetNormals(data, dataLength);
	}

	void ScriptConnector::Mesh_SetTextureCoords(AssetHandle assetHandle, float* data, uint32_t dataLength)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		mesh->SubMeshes.front()->SetTextureCoords(data, dataLength);
	}

	void ScriptConnector::Mesh_SetTangents(AssetHandle assetHandle, float* data, uint32_t dataLength)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		mesh->SubMeshes.front()->SetTangents(data, dataLength);
	}

	void ScriptConnector::Mesh_SetBitangents(AssetHandle assetHandle, float* data, uint32_t dataLength)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;
		
		mesh->SubMeshes.front()->SetBiTangents(data, dataLength);
	}

	void ScriptConnector::Mesh_GetVertices(AssetHandle assetHandle, const float** data, int* count)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		size_t temp_count = 0;
		*data = mesh->SubMeshes.front()->GetVertices(&temp_count);
		*count = static_cast<int>(temp_count);
	}

	void ScriptConnector::Mesh_GetNormals(AssetHandle assetHandle, const float** data, int* count)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		size_t temp_count = 0;
		*data = mesh->SubMeshes.front()->GetNormals(&temp_count);
		*count = static_cast<int>(temp_count);
	}

	void ScriptConnector::Mesh_GetTextureCoords(AssetHandle assetHandle, const float** data, int* count)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		size_t temp_count = 0;
		*data = mesh->SubMeshes.front()->GetTextureCoords(&temp_count);
		*count = static_cast<int>(temp_count);
	}

	void ScriptConnector::Mesh_GetTangents(AssetHandle assetHandle, const float** data, int* count)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		size_t temp_count = 0;
		*data = mesh->SubMeshes.front()->GetTangents(&temp_count);
		*count = static_cast<int>(temp_count);
	}

	void ScriptConnector::Mesh_GetBitangents(AssetHandle assetHandle, const float** data, int* count)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		size_t temp_count = 0;
		*data = mesh->SubMeshes.front()->GetBiTangents(&temp_count);
		*count = static_cast<int>(temp_count);
	}

	uint32_t ScriptConnector::MeshFilterComponent_SharedMesh(uint32_t entityUUID)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityUUID);
		if (!entity) return NULL_UUID;

		if (!entity.HasComponent<MeshFilterComponent>())
			return NULL_UUID;

		AssetHandle handle = entity.GetComponent<MeshFilterComponent>().MeshFilterAssetHandle;
		if (!AssetManager::IsAssetHandleValid(handle))
			return NULL_UUID;

		return handle;
	}

	uint32_t ScriptConnector::MeshFilterComponent_CopyMesh(uint32_t entityUUID)
	{
		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityUUID);
		if (!entity) return NULL_UUID;

		if (!entity.HasComponent<MeshFilterComponent>())
			return NULL_UUID;

		auto asset_mesh = AssetManager::GetAsset<AssetMesh>(entity.GetComponent<MeshFilterComponent>().MeshFilterAssetHandle);

		if (asset_mesh && asset_mesh->SubMeshes.size() >= 1 && asset_mesh->SubMeshes.front())
		{
			auto copy_asset_mesh = std::make_shared<AssetMesh>();
			copy_asset_mesh->SubMeshes.push_back(std::make_shared<SubMesh>(*asset_mesh->SubMeshes.front()));
			copy_asset_mesh->MeshBounds = asset_mesh->MeshBounds;

			return AssetManager::AddRuntimeAsset<AssetMesh>(copy_asset_mesh, "New Runtime Mesh");
		}

		std::shared_ptr<SubMesh> sub_mesh = std::make_shared<SubMesh>();

		sub_mesh->SetVAO(std::make_unique<VertexArray>());

		VertexBuffer* vbo_verts = new VertexBuffer(3);
		vbo_verts->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aPos"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_verts);

		VertexBuffer* vbo_norms = new VertexBuffer(3);
		vbo_norms->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aNormal"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_norms);

		VertexBuffer* vbo_texcoords = new VertexBuffer(2);
		vbo_texcoords->SetLayout(BufferLayout{ {ShaderDataType::Float2, "aTexCoord"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_texcoords);

		VertexBuffer* vbo_tangents = new VertexBuffer(3);
		vbo_tangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aTangent"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_tangents);

		VertexBuffer* vbo_bitangents = new VertexBuffer(3);
		vbo_bitangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aBitangent"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_bitangents);

		IndexBuffer* ebo = new IndexBuffer();
		sub_mesh->GetVAO()->SetIndexBuffer(ebo);

		asset_mesh = std::make_shared<AssetMesh>();

		asset_mesh->SubMeshes.push_back(std::move(sub_mesh));

		AssetHandle handle = AssetManager::AddRuntimeAsset(asset_mesh, "New Runtime Mesh");
		entity.GetComponent<MeshFilterComponent>().MeshFilterAssetHandle = handle;

		return handle;
	}

	void ScriptConnector::MeshFilterComponent_SetMesh(uint32_t entityUUID, uint32_t assetHandle)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		Scene* scene = ScriptManager::GetSceneContext();
		L_CORE_ASSERT(scene, "Scene Not Valid.");
		Entity entity = scene->FindEntityByUUID(entityUUID);
		if (!entity) return;

		if (!entity.HasComponent<MeshFilterComponent>())
			return;

		entity.GetComponent<MeshFilterComponent>().MeshFilterAssetHandle = assetHandle;
	}

	void ScriptConnector::Mesh_SetTriangles(AssetHandle assetHandle, uint32_t* data, uint32_t dataLength)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		mesh->SubMeshes.front()->SetTriangles(data, dataLength);
	}

	void ScriptConnector::Mesh_GetTriangles(AssetHandle assetHandle, const uint32_t** data, int* count)
	{
		if (!AssetManager::IsAssetHandleValid(assetHandle))
			return;

		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		size_t temp_count = 0;
		*data = mesh->SubMeshes.front()->GetTriangles(&temp_count);
		*count = static_cast<int>(temp_count);
	}

	void ScriptConnector::Mesh_RecalculateNormals(AssetHandle assetHandle)
	{
		const auto& mesh = AssetManager::GetAsset<AssetMesh>(assetHandle);

		if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

		mesh->SubMeshes.front()->RecalculateNormals();
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

