using System;
using System.Runtime.CompilerServices;

namespace Louron
{

    public static class EngineCallbacks
    {

        #region Entity

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_DestroyEntity(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_AddComponent(uint entityID, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_RemoveComponent(uint entityID, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(uint entityID, Type componentType);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_Instantiate(uint entityID, ref uint prefab_asset_handle, out uint prefab_clone_entity_id); // Returns UUID of new entity!

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Entity_GetParent(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetParent(uint entityID, uint parentID);

        #endregion

        #region Input

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_GetKey(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_GetKeyDown(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_GetKeyUp(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_GetMouseButton(MouseButton keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_GetMouseButtonDown(MouseButton keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_GetMouseButtonUp(MouseButton keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMousePosition(out Vector2 mousePos);
        
        #endregion

        #region Components

        #region Transform Component

        // Transform
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTransform(uint entityID, ref Entity._Transform reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTransform(uint entityID, ref Entity._Transform reference);

        // Position
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetPosition(uint entityID, ref Vector3 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetPosition(uint entityID, ref Vector3 reference);

        // Rotation
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(uint entityID, ref Vector3 rereferencesult);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(uint entityID, ref Vector3 reference);

        // Scale
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetScale(uint entityID, ref Vector3 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetScale(uint entityID, ref Vector3 reference);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetFront(uint entityID, ref Vector3 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetUp(uint entityID, ref Vector3 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRight(uint entityID, ref Vector3 reference);

        #endregion

        #region Tag Component
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TagComponent_GetTag(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TagComponent_SetTag(uint entityID, string reference);
        #endregion

        #region Script Component
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ScriptComponent_GetScript(uint entityID, out string result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ScriptComponent_SetScript(uint entityID, ref string reference);
        #endregion

        #region Point Light
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_GetActive(uint entityID, out bool result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_SetActive(uint entityID, ref bool reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_GetColour(uint entityID, out Vector4 result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_SetColour(uint entityID, ref Vector4 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_GetRadius(uint entityID, out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_SetRadius(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_GetIntensity(uint entityID, out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_SetIntensity(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_GetShadowFlag(uint entityID, out ShadowTypeFlag result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PointLightComponent_SetShadowFlag(uint entityID, ref ShadowTypeFlag reference);
        #endregion

        #region Spot Light
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_GetActive(uint entityID, out bool result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_SetActive(uint entityID, ref bool reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_GetColour(uint entityID, out Vector4 result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_SetColour(uint entityID, ref Vector4 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_GetRange(uint entityID, out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_SetRange(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_GetAngle(uint entityID, out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_SetAngle(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_GetIntensity(uint entityID, out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_SetIntensity(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_GetShadowFlag(uint entityID, out ShadowTypeFlag result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpotLightComponent_SetShadowFlag(uint entityID, ref ShadowTypeFlag reference);
        #endregion

        #region Directional Light
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_GetActive(uint entityID, out bool result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_SetActive(uint entityID, ref bool reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_GetColour(uint entityID, out Vector4 result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_SetColour(uint entityID, ref Vector4 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_GetIntensity(uint entityID, out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_SetIntensity(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_GetShadowFlag(uint entityID, out ShadowTypeFlag result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_SetShadowFlag(uint entityID, ref ShadowTypeFlag reference);
        #endregion

        #region Rigidbody

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_GetMass(uint entityID, out float result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetMass(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_GetDrag(uint entityID, out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetDrag(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_GetAngularDrag(uint entityID, out float result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetAngularDrag(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_GetAutomaticCentreOfMass(uint entityID, out bool result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetAutomaticCentreOfMass(uint entityID, ref bool reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_GetUseGravity(uint entityID, out bool result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetUseGravity(uint entityID, ref bool reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_GetIsKinematic(uint entityID, out bool result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetIsKinematic(uint entityID, ref bool reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_GetPositionConstraint(uint entityID, out BVector3 result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetPositionConstraint(uint entityID, ref BVector3 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_GetRotationConstraint(uint entityID, out BVector3 result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_SetRotationConstraint(uint entityID, ref BVector3 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_ApplyForce(uint entityID, ref Vector3 force, ref ForceMode forceMode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidbodyComponent_ApplyTorque(uint entityID, ref Vector3 torque);

        #endregion

        #region Box Collider

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_GetIsTrigger(uint entityID, out bool result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_SetIsTrigger(uint entityID, ref bool reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_GetCentre(uint entityID, out Vector3 result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_SetCentre(uint entityID, ref Vector3 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_GetSize(uint entityID, out Vector3 result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_SetSize(uint entityID, ref Vector3 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_GetMaterial(uint entityID, out PhysicsMaterial result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxColliderComponent_SetMaterial(uint entityID, ref PhysicsMaterial reference);

        #endregion

        #region Sphere Collider

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_GetIsTrigger(uint entityID, out bool result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_SetIsTrigger(uint entityID, ref bool reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_GetCentre(uint entityID, out Vector3 result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_SetCentre(uint entityID, ref Vector3 reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_GetRadius(uint entityID, out float result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_SetRadius(uint entityID, ref float reference);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_GetMaterial(uint entityID, out PhysicsMaterial result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SphereColliderComponent_SetMaterial(uint entityID, ref PhysicsMaterial reference);

        #endregion

        #endregion

        #region Debug

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Debug_LogMessage(Debug.LogType type, string message);

        #endregion

        #region Time

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetDeltaTime(out float deltaTime);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetCurrentTime(out float currentTime);

        #endregion

    }
}
