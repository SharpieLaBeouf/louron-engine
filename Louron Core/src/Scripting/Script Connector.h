#pragma once

// Louron Core Headers

#include "../Core/Logging.h"
#include "../Core/Time.h"
#include "../Core/Input.h"

#include "../Debug/Assert.h"

#include "../OpenGL/Compute Shader Asset.h"

#include "../Project/Project.h"

#include "../Scene/Scene.h"
#include "../Scene/Entity.h"
#include "../Scene/Prefab.h"
#include "../Scene/Components/Components.h"
#include "../Scene/Components/Light.h"
#include "../Scene/Components/Skybox.h"


#include "Script Manager.h"

// C++ Standard Library Headers
#include <typeindex>

// External Vendor Library Headers
#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

namespace Louron {

	class ScriptConnector {
	public:

		static void RegisterComponents();
		static void RegisterFunctions();
	
	private:

		static void Debug_LogMessage(int type, MonoString* message);


#pragma region Entity

		static void Entity_DestroyEntity(UUID entityID);
		static void Entity_AddComponent(UUID entityID, MonoReflectionType* componentType);
		static void Entity_RemoveComponent(UUID entityID, MonoReflectionType* componentType);
		static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType);
		static void Entity_Instantiate(UUID entityID, uint32_t* handle, uint32_t* prefab_clone_uuid);
		static UUID Entity_GetParent(UUID entityID);
		static void Entity_SetParent(UUID entityID, UUID parentID);

#pragma endregion

#pragma region Input

		static bool Input_GetKey(KeyCode keyCode);
		static bool Input_GetKeyDown(KeyCode keyCode);
		static bool Input_GetKeyUp(KeyCode keyCode);
		static bool Input_GetMouseButton(MouseButtonCode buttonCode);
		static bool Input_GetMouseButtonDown(MouseButtonCode buttonCode);
		static bool Input_GetMouseButtonUp(MouseButtonCode buttonCode);
		static void Input_GetMousePosition(glm::vec2* mousePos);

#pragma endregion

#pragma region Time

		static void Time_GetDeltaTime(float* out_float);
		static void Time_GetCurrentTime(float* out_float);

#pragma endregion

#pragma region TransformComponent

		struct _Transform {
			glm::vec3 position;
			glm::vec3 rotation;
			glm::vec3 scale;
		};

		static void TransformComponent_GetTransform(UUID entityID, _Transform* out_transform);
		static void TransformComponent_SetTransform(UUID entityID, _Transform* transform);
		static void TransformComponent_GetPosition(UUID entityID, glm::vec3* ref);
		static void TransformComponent_SetPosition(UUID entityID, glm::vec3* ref);

		static void TransformComponent_GetRotation(UUID entityID, glm::vec3* ref);
		static void TransformComponent_SetRotation(UUID entityID, glm::vec3* ref);
		static void TransformComponent_GetScale(UUID entityID, glm::vec3* ref);
		static void TransformComponent_SetScale(UUID entityID, glm::vec3* ref);
		static void TransformComponent_SetFront(UUID entityID, glm::vec3* ref);
		static void TransformComponent_GetFront(UUID entityID, glm::vec3* ref);
		static void TransformComponent_GetUp(UUID entityID, glm::vec3* ref);
		static void TransformComponent_GetRight(UUID entityID, glm::vec3* ref);

#pragma endregion

#pragma region Tag Component

		static MonoString* Tag_GetTag(UUID entityID);
		static void Tag_SetTag(UUID entityID, MonoString* ref);

#pragma endregion

#pragma region Script Component

		//static MonoString* Script_GetScriptName(UUID entityID)
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

		//static void Script_SetScriptName(UUID entityID, MonoString* ref)
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

		static void PointLightComponent_GetActive(UUID entityID, bool* out);
		static void PointLightComponent_SetActive(UUID entityID, bool* ref);
		static void PointLightComponent_GetColour(UUID entityID, glm::vec4* out);
		static void PointLightComponent_SetColour(UUID entityID, glm::vec4* ref);
		static void PointLightComponent_GetRadius(UUID entityID, float* out);
		static void PointLightComponent_SetRadius(UUID entityID, float* ref);
		static void PointLightComponent_GetIntensity(UUID entityID, float* out);
		static void PointLightComponent_SetIntensity(UUID entityID, float* ref);
		static void PointLightComponent_GetShadowFlag(UUID entityID, uint8_t* out);
		static void PointLightComponent_SetShadowFlag(UUID entityID, uint8_t* ref);

#pragma endregion

#pragma region Spot Light Component

		static void SpotLightComponent_GetActive(UUID entityID, bool* out);
		static void SpotLightComponent_SetActive(UUID entityID, bool* ref);
		static void SpotLightComponent_GetColour(UUID entityID, glm::vec4* out);
		static void SpotLightComponent_SetColour(UUID entityID, glm::vec4* ref);
		static void SpotLightComponent_GetRange(UUID entityID, float* out);
		static void SpotLightComponent_SetRange(UUID entityID, float* ref);
		static void SpotLightComponent_GetAngle(UUID entityID, float* out);
		static void SpotLightComponent_SetAngle(UUID entityID, float* ref);
		static void SpotLightComponent_GetIntensity(UUID entityID, float* out);
		static void SpotLightComponent_SetIntensity(UUID entityID, float* ref);
		static void SpotLightComponent_GetShadowFlag(UUID entityID, ShadowTypeFlag* out);
		static void SpotLightComponent_SetShadowFlag(UUID entityID, ShadowTypeFlag* ref);

#pragma endregion

#pragma region Directional Light Component

		static void DirectionalLightComponent_GetActive(UUID entityID, bool* out);
		static void DirectionalLightComponent_SetActive(UUID entityID, bool* ref);
		static void DirectionalLightComponent_GetColour(UUID entityID, glm::vec4* out);
		static void DirectionalLightComponent_SetColour(UUID entityID, glm::vec4* ref);
		static void DirectionalLightComponent_GetIntensity(UUID entityID, float* out);
		static void DirectionalLightComponent_SetIntensity(UUID entityID, float* ref);
		static void DirectionalLightComponent_GetShadowFlag(UUID entityID, ShadowTypeFlag* out);
		static void DirectionalLightComponent_SetShadowFlag(UUID entityID, ShadowTypeFlag* ref);

#pragma endregion

#pragma region Physics

		struct _PhysicsMaterial {
			float m_DynamicFriction;
			float m_StaticFriction;
			float m_Bounciness;
		};

#pragma region RigidbodyComponent Component

		static void Rigidbody_GetMass(UUID entityID, float* out);
		static void Rigidbody_SetMass(UUID entityID, float* ref);
		static void Rigidbody_GetDrag(UUID entityID, float* out);
		static void Rigidbody_SetDrag(UUID entityID, float* ref);
		static void Rigidbody_GetAngularDrag(UUID entityID, float* out);
		static void Rigidbody_SetAngularDrag(UUID entityID, float* ref);
		static void Rigidbody_GetAutomaticCentreOfMass(UUID entityID, bool* out);
		static void Rigidbody_SetAutomaticCentreOfMass(UUID entityID, bool* ref);
		static void Rigidbody_GetUseGravity(UUID entityID, bool* out);
		static void Rigidbody_SetUseGravity(UUID entityID, bool* ref);
		static void Rigidbody_GetIsKinematic(UUID entityID, bool* out);
		static void Rigidbody_SetIsKinematic(UUID entityID, bool* ref);
		static void Rigidbody_GetPositionConstraint(UUID entityID, glm::bvec3* out);
		static void Rigidbody_SetPositionConstraint(UUID entityID, glm::bvec3* ref);
		static void Rigidbody_GetRotationConstraint(UUID entityID, glm::bvec3* out);
		static void Rigidbody_SetRotationConstraint(UUID entityID, glm::bvec3* ref);
		static void Rigidbody_ApplyForce(UUID entityID, glm::vec3* force, PxForceMode::Enum* forceMode);
		static void Rigidbody_ApplyTorque(UUID entityID, glm::vec3* torque);

#pragma endregion

#pragma region Box Collider Component

		static void BoxColliderComponent_GetIsTrigger(UUID entityID, bool* result);
		static void BoxColliderComponent_SetIsTrigger(UUID entityID, bool* reference);
		static void BoxColliderComponent_GetCentre(UUID entityID, glm::vec3* result);
		static void BoxColliderComponent_SetCentre(UUID entityID, glm::vec3* reference);
		static void BoxColliderComponent_GetSize(UUID entityID, glm::vec3* result);
		static void BoxColliderComponent_SetSize(UUID entityID, glm::vec3* reference);
		static void BoxColliderComponent_GetMaterial(UUID entityID, _PhysicsMaterial* result);
		static void BoxColliderComponent_SetMaterial(UUID entityID, _PhysicsMaterial* reference);

#pragma endregion

#pragma region Sphere Collider Component

		static void SphereColliderComponent_GetIsTrigger(UUID entityID, bool* result);
		static void SphereColliderComponent_SetIsTrigger(UUID entityID, bool* reference);
		static void SphereColliderComponent_GetCentre(UUID entityID, glm::vec3* result);
		static void SphereColliderComponent_SetCentre(UUID entityID, glm::vec3* reference);
		static void SphereColliderComponent_GetRadius(UUID entityID, float* result);
		static void SphereColliderComponent_SetRadius(UUID entityID, float* reference);
		static void SphereColliderComponent_GetMaterial(UUID entityID, _PhysicsMaterial* result);
		static void SphereColliderComponent_SetMaterial(UUID entityID, _PhysicsMaterial* reference);

#pragma endregion

#pragma endregion


#pragma region Compute Shader

		static void ComputeShader_SetBuffer(AssetHandle asset_handle, ComputeBuffer* buffer, uint32_t binding_index);
		static void ComputeShader_Dispatch(AssetHandle asset_handle, uint32_t x, uint32_t y, uint32_t z);
		static void ComputeShader_SetBool(AssetHandle asset_handle, MonoString* name, bool value);
		static void ComputeShader_SetInt(AssetHandle asset_handle, MonoString* name, int32_t value);
		static void ComputeShader_SetUInt(AssetHandle asset_handle, MonoString* name, uint32_t value);
		static void ComputeShader_SetFloat(AssetHandle asset_handle, MonoString* name, float value);
		static void ComputeShader_SetVector2(AssetHandle asset_handle, MonoString* name, glm::vec2 value);
		static void ComputeShader_SetVector3(AssetHandle asset_handle, MonoString* name, glm::vec3 value);
		static void ComputeShader_SetVector4(AssetHandle asset_handle, MonoString* name, glm::vec4 value);

#pragma endregion

#pragma region Compute Buffer

		static ComputeBuffer* ComputeBuffer_Create(int element_count, int element_size);
		static void ComputeBuffer_SetData(ComputeBuffer* buffer, void* data, int element_count, int element_size);
		static void ComputeBuffer_GetData(ComputeBuffer* buffer, void* output, int element_count, int element_size);
		static void ComputeBuffer_Release(ComputeBuffer* buffer);

#pragma endregion

	};

}