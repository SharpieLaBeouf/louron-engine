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

        #region Compute Shaders

        // Compute Shader
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeShader_Dispatch(uint shader_asset, uint x, uint y, uint z);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeShader_SetBuffer(uint shader_asset, IntPtr buffer, uint binding_index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeShader_SetBool(uint shader_asset, string name, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeShader_SetInt(uint shader_asset, string name, int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeShader_SetUInt(uint shader_asset, string name, uint value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeShader_SetFloat(uint shader_asset, string name, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeShader_SetVector2(uint shader_asset, string name, Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeShader_SetVector3(uint shader_asset, string name, Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeShader_SetVector4(uint shader_asset, string name, Vector4 value);

        // TODO: Implement Matrix data type into Louron Script Core
        //[MethodImpl(MethodImplOptions.InternalCall)]
        //internal extern static void ComputeShader_SetMat2(uint shader_asset, string name, ref Matrix2x2 value);

        //[MethodImpl(MethodImplOptions.InternalCall)]
        //internal extern static void ComputeShader_SetMat3(uint shader_asset, string name, ref Matrix3x3 value);

        //[MethodImpl(MethodImplOptions.InternalCall)]
        //internal extern static void ComputeShader_SetMat4(uint shader_asset, string name, ref Matrix4x4 value);

        // Compute Buffer

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static IntPtr ComputeBuffer_Create(int element_count, int element_size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeBuffer_SetData(IntPtr buffer, IntPtr data, int element_count, int element_size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeBuffer_GetData(IntPtr buffer, IntPtr output, int element_count, int element_size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ComputeBuffer_Release(IntPtr buffer);


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
        internal extern static void TransformComponent_SetFront(uint entityID, ref Vector3 reference);

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

        #region Material

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Material_Create(string material_name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Material_SetShader(uint asset_handle, uint shader_handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Material_Destroy(uint asset_handle);

        #endregion

        #region MeshRendererComponent

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint MeshRendererComponent_GetMaterial(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRendererComponent_SetMaterial(uint entityID, uint material_handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint[] MeshRendererComponent_GetMaterials(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRendererComponent_SetMaterials(uint entityID, uint[] material_handles, uint num_elements);

        // Uniform Blocks
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_EnableUniformBlock(uint entityID, uint material_index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static IntPtr MeshRenderer_GetUniformBlock(uint entityID, uint material_index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_DisableUniformBlock(uint entityID, uint material_index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_EnableAllUniformBlocks(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MeshRenderer_DisableAllUniformBlocks(uint entityID);

        #endregion

        #region Material Uniform Block

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MaterialUniformBlock_SetUniform(IntPtr uniform_block, string uniform_name, uint type, IntPtr value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MaterialUniformBlock_OverrideAlbedoMap(IntPtr uniform_block, uint asset_handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MaterialUniformBlock_OverrideMetallicMap(IntPtr uniform_block, uint asset_handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MaterialUniformBlock_OverrideNormalMap(IntPtr uniform_block, uint asset_handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MaterialUniformBlock_OverrideAlbedoTint(IntPtr uniform_block, Vector4 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MaterialUniformBlock_OverrideMetallic(IntPtr uniform_block, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MaterialUniformBlock_OverrideRoughness(IntPtr uniform_block, float value);

        #endregion

        #region Texture2D

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Texture2D_Create(int width, int height, byte internal_format);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Texture2D_CreateWithData(IntPtr data, int width, int height, byte internal_format, byte data_format);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Texture2D_Destroy(uint asset_handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Texture2D_SetPixel(uint asset_handle, Vector4 pixel_colour, IVector2 texture_coord);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Texture2D_SetPixelData(uint asset_handle, IntPtr pixel_data, uint pixel_data_size, byte pixel_data_format);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Texture2D_SubmitTextureChanges(uint asset_handle);

        #endregion

    }
}
