#include "Components.h"

// Louron Core Headers
#include "../../Core/Logging.h"
#include "../Entity.h"
#include "../Scene.h"

#include "Physics/Collider.h"
#include "Physics/Rigidbody.h"
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "Skybox.h"

// External Vendor Library Headers
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron {

#pragma region TransformComponent

    Transform::Transform() {
        AddFlag(TransformFlag_PropertiesUpdated);
    }

    Transform::Transform(const glm::vec3& translation) : m_Position(translation) {
        AddFlag(TransformFlag_PropertiesUpdated);
    }

    void Transform::AddFlag(TransformFlags flag) { m_StateFlags = static_cast<TransformFlags>(m_StateFlags | flag); }
    void Transform::RemoveFlag(TransformFlags flag) { m_StateFlags = static_cast<TransformFlags>(m_StateFlags & ~flag); }
    bool Transform::CheckFlag(TransformFlags flag) const { return (m_StateFlags & static_cast<TransformFlags>(flag)) != TransformFlag_None; }
    bool Transform::NoFlagsSet() const { return m_StateFlags == TransformFlag_None; }
    void Transform::ClearFlags() { m_StateFlags = TransformFlag_None; }
    TransformFlags Transform::GetFlags() const { return m_StateFlags; }

    /// <summary>
    /// Set the position to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed position.</param>
    void Transform::SetPosition(const glm::vec3& newPosition) {
        m_Position = newPosition;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::SetPositionX(const float& newXPosition) {
        m_Position.x = newXPosition;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::SetPositionY(const float& newYPosition) {
        m_Position.y = newYPosition;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::SetPositionZ(const float& newZPosition) {
        m_Position.z = newZPosition;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Set the rotation to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed rotation.</param>
    void Transform::SetRotation(const glm::vec3& newRotation) {
        m_Rotation = newRotation;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::SetRotationX(const float& newXRotation) {
        m_Rotation.x = newXRotation;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::SetRotationY(const float& newYRotation) {
        m_Rotation.y = newYRotation;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::SetRotationZ(const float& newZRotation) {
        m_Rotation.z = newZRotation;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Set the scale to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed scale.</param>
    void Transform::SetScale(const glm::vec3& newScale) {
        m_Scale = newScale;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::SetScaleX(const float& newXScale) {
        m_Scale.x = newXScale;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::SetScaleY(const float& newYScale) {
        m_Scale.y = newYScale;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::SetScaleZ(const float& newZScale) {
        m_Scale.z = newZScale;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Translation to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current position.</param>
    void Transform::Translate(const glm::vec3& vector) {
        m_Position += vector;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Translation to the Transform along the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X position.</param>
    void Transform::TranslateX(const float& deltaTranslationX) {
        m_Position.x += deltaTranslationX;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Translation to the Transform along the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y position.</param>
    void Transform::TranslateY(const float& deltaTranslationY) {
        m_Position.y += deltaTranslationY;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Translation to the Transform along the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z position.</param>
    void Transform::TranslateZ(const float& deltaTranslationZ) {
        m_Position.z += deltaTranslationZ;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }
    /// <summary>
    /// Apply a Rotation to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current rotation.</param>
    void Transform::Rotate(const glm::vec3& vector) {
        m_Rotation += vector;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X rotation.</param>
    void Transform::RotateX(const float& deltaRotationX) {
        m_Rotation.x += deltaRotationX;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y rotation.</param>
    void Transform::RotateY(const float& deltaRotationY) {
        m_Rotation.y += deltaRotationY;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z rotation.</param>
    void Transform::RotateZ(const float& deltaRotationZ) {
        m_Rotation.z += deltaRotationZ;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Scale to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current scale.</param>
    void Transform::Scale(const glm::vec3& vector) {
        m_Scale += vector;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Scale to the Transform along the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X scale.</param>
    void Transform::ScaleX(const float& deltaScaleX) {
        m_Scale.x += deltaScaleX;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Scale to the Transform along the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y scale.</param>
    void Transform::ScaleY(const float& deltaScaleY) {
        m_Scale.y += deltaScaleY;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Scale to the Transform along the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z scale.</param>
    void Transform::ScaleZ(const float& deltaScaleZ) {
        m_Scale.z += deltaScaleZ;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    const glm::vec3& Transform::GetLocalPosition() const    { return m_Position;    }
    const glm::vec3& Transform::GetLocalRotation() const    { return m_Rotation;    }
    const glm::vec3& Transform::GetLocalScale() const       { return m_Scale;       }

    void Transform::SetGlobalPosition(const glm::vec3& globalPosition) {

        Entity entity = GetEntity();
        if (entity && entity.GetScene() && entity.GetComponent<HierarchyComponent>().HasParent()) {

            glm::mat4 parentGlobalMatrix = GetComponentInParent<Transform>()->GetGlobalTransform();
            glm::mat4 parentInverseMatrix = glm::inverse(parentGlobalMatrix);
            glm::vec4 localPosition4 = parentInverseMatrix * glm::vec4(globalPosition, 1.0f);
            SetPosition(glm::vec3(localPosition4));
        }
        else {
            SetPosition(globalPosition);
        }
    }

    void Transform::SetGlobalRotation(const glm::vec3& globalRotation) {

        Entity entity = GetEntity();

        if (entity && entity.GetScene() && entity.GetComponent<HierarchyComponent>().HasParent()) {

            glm::quat parentGlobalRotation = glm::quat(glm::radians(GetComponentInParent<Transform>()->GetGlobalRotation()));
            glm::quat parentInverseRotation = glm::inverse(parentGlobalRotation);

            glm::quat globalQuat = glm::quat(glm::radians(globalRotation));
            glm::quat localQuat = parentInverseRotation * globalQuat;
            SetRotation(glm::degrees(glm::eulerAngles(localQuat)));
        }
        else {
            SetRotation(globalRotation);
        }
    }

    void Transform::SetGlobalScale(const glm::vec3& globalScale) {

        Entity entity = GetEntity();

        if (entity && entity.GetScene() && entity.GetComponent<HierarchyComponent>().HasParent()) {

            glm::vec3 parentGlobalScale = GetComponentInParent<Transform>()->GetGlobalScale();
            m_Scale = globalScale / parentGlobalScale;
        }
        else {
            m_Scale = globalScale;
        }

        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void Transform::OnTransformUpdated() {

        AddFlag(TransformFlag_GlobalTransformUpdated);

        Entity entity = GetEntity();

        if (entity.HasComponent<AssetMeshFilter>()) {

            auto& component = entity.GetComponent<AssetMeshFilter>();
            component.AABBNeedsUpdate = true;
            component.OctreeNeedsUpdate = true;
        }

        if (entity && entity.GetScene()) {

            for (const auto& child_uuid : entity.GetComponent<HierarchyComponent>().GetChildren()) {
                Entity child_entity = entity.GetScene()->FindEntityByUUID(child_uuid);

                child_entity.GetComponent<Transform>().OnTransformUpdated();
            }
        }

    }

    void Transform::UpdateLocalTransformMatrix() {

        Entity entity = GetEntity();

        // Check if any changes made to local transform, if YES WE UPDATE
        if (CheckFlag(TransformFlag_PropertiesUpdated)) {

            // Compute the local transform matrix if it has changed
            m_LocalTransform =  glm::translate(glm::mat4(1.0f), m_Position) *
                                glm::mat4_cast(glm::quat(glm::radians(m_Rotation))) *
                                glm::scale(glm::mat4(1.0f), m_Scale);

            OnTransformUpdated();

            if (entity && entity.GetScene() && entity.HasComponent<Rigidbody>() && entity.GetComponent<Rigidbody>().GetActor())
                entity.GetComponent<Rigidbody>().GetActor()->AddFlag(RigidbodyFlag_TransformUpdated);

            RemoveFlag(TransformFlag_PropertiesUpdated);
            RemoveFlag(TransformFlag_PropertiesUpdated);
            RemoveFlag(TransformFlag_PropertiesUpdated);
        }

    }

    glm::vec3 Transform::GetGlobalPosition() {
        if (CheckFlag(TransformFlag_GlobalTransformUpdated)) 
            return GetGlobalTransform()[3];

        if(m_LocalTransform != m_GlobalTransform)
            return m_GlobalTransform[3];

        return m_Position;
    }

    glm::vec3 Transform::GetGlobalRotation() {

        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
            return glm::vec3(glm::degrees(glm::eulerAngles(glm::quat_cast(GetGlobalTransform()))));

        if (m_LocalTransform != m_GlobalTransform)
            return glm::vec3(glm::degrees(glm::eulerAngles(glm::quat_cast(m_GlobalTransform))));

        return m_Rotation;
    }

    glm::vec3 Transform::GetGlobalScale() {

        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
        {
            glm::mat4 global_transfom = GetGlobalTransform();
            return  glm::vec3(
                        glm::length(global_transfom[0]),
                        glm::length(global_transfom[1]),
                        glm::length(global_transfom[2])
                    );
        }

        if (m_LocalTransform != m_GlobalTransform)
            return  glm::vec3(
                        glm::length(m_GlobalTransform[0]),
                        glm::length(m_GlobalTransform[1]),
                        glm::length(m_GlobalTransform[2])
                    );

        return m_Scale;
    }

    const glm::vec3 local_forward = glm::vec3{ 0.0f, 0.0f, -1.0f };

    void Transform::SetForwardDirection(const glm::vec3& direction) {
        glm::vec3 forward = glm::normalize(direction);
        glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(glm::rotation(local_forward, forward)));
        SetRotation(eulerRotation); // Set Local
    }

    glm::vec3 Transform::GetForwardDirection() {
        glm::quat rotation = glm::quat(glm::radians(m_Rotation));
        glm::vec3 forward = rotation * local_forward;
        return glm::normalize(forward);
    }

    void Transform::SetGlobalForwardDirection(const glm::vec3& direction) {
        glm::vec3 forward = glm::normalize(direction);
        glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(glm::rotation(local_forward, forward)));
        SetGlobalRotation(eulerRotation); // Set Global
    }

    glm::vec3 Transform::GetGlobalForwardDirection() {
        glm::quat globalRotation = glm::quat(glm::radians(GetGlobalRotation()));
        glm::vec3 globalForward = globalRotation * local_forward;
        return glm::normalize(globalForward);
    }

    const glm::mat4& Transform::GetGlobalTransform() { 

        if (CheckFlag(TransformFlag_GlobalTransformUpdated)) {

            glm::vec3 old_global_scale = glm::vec3(
                glm::length(m_GlobalTransform[0]),
                glm::length(m_GlobalTransform[1]),
                glm::length(m_GlobalTransform[2])
            ); // Can't call GetGlobalScale here as it would send it into a never ending recursion as GetGlobalScale will call GetGlobalTransform if the flag is not cleared

            Entity entity = GetEntity();

            if (entity && entity.GetScene() && entity.GetComponent<HierarchyComponent>().HasParent())
                m_GlobalTransform = entity.GetComponent<HierarchyComponent>().GetParentEntity().GetComponent<Transform>().GetGlobalTransform() * GetLocalTransform();
            else
                m_GlobalTransform = m_LocalTransform;

            RemoveFlag(TransformFlag_GlobalTransformUpdated);

            if (entity) {

                if (entity.HasComponent<SphereCollider>()) {

                    if (entity.GetComponent<SphereCollider>().GetShape()->IsStatic())
                        entity.GetComponent<SphereCollider>().GetShape()->AddFlag(ColliderFlag_TransformUpdated);

                    if (old_global_scale != GetGlobalScale())
                        entity.GetComponent<SphereCollider>().GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated); // TODO: Fix this because it goes on the fritz when child is attached to parent, and the parent scale changes

                }
                
                if (entity.HasComponent<BoxCollider>()) {

                    if(entity.GetComponent<BoxCollider>().GetShape()->IsStatic())
                        entity.GetComponent<BoxCollider>().GetShape()->AddFlag(ColliderFlag_TransformUpdated);

                    if (old_global_scale != GetGlobalScale())
                        entity.GetComponent<BoxCollider>().GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);

                }
            }
        }
        return m_GlobalTransform;
    }

    const glm::mat4& Transform::GetLocalTransform() {
        
        UpdateLocalTransformMatrix();
        return m_LocalTransform;
    }

    void Transform::SetTransform(const glm::mat4& transform)
    {
        m_LocalTransform = transform;
    }

    Transform::operator const glm::mat4()& { return m_LocalTransform; }

    glm::mat4 Transform::operator*(const Transform& other) const { return m_LocalTransform * other.m_LocalTransform; }

    void Transform::Serialize(YAML::Emitter& out) {

        out << YAML::Key << "TransformComponent";
        out << YAML::BeginMap;

        out << YAML::Key << "Translation" << YAML::Value << YAML::Flow
            << YAML::BeginSeq
            << m_Position.x
            << m_Position.y
            << m_Position.z
            << YAML::EndSeq;

        out << YAML::Key << "Rotation" << YAML::Value << YAML::Flow
            << YAML::BeginSeq
            << m_Rotation.x
            << m_Rotation.y
            << m_Rotation.z
            << YAML::EndSeq;

        out << YAML::Key << "Scale" << YAML::Value << YAML::Flow
            << YAML::BeginSeq
            << m_Scale.x
            << m_Scale.y
            << m_Scale.z
            << YAML::EndSeq;

        out << YAML::EndMap;
    }

    bool Transform::Deserialize(const YAML::Node data)
    {
        AddFlag(TransformFlag_GlobalTransformUpdated);
        AddFlag(TransformFlag_PropertiesUpdated);

        YAML::Node component = data;

        if (component["Translation"]) {
            auto translationSeq = component["Translation"];
            if (translationSeq.IsSequence() && translationSeq.size() == 3) {
                m_Position.x = translationSeq[0].as<float>();
                m_Position.y = translationSeq[1].as<float>();
                m_Position.z = translationSeq[2].as<float>();
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }

        if (component["Rotation"]) {
            auto rotationSeq = component["Rotation"];
            if (rotationSeq.IsSequence() && rotationSeq.size() == 3) {
                m_Rotation.x = rotationSeq[0].as<float>();
                m_Rotation.y = rotationSeq[1].as<float>();
                m_Rotation.z = rotationSeq[2].as<float>();
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }

        if (component["Scale"]) {
            auto scaleSeq = component["Scale"];
            if (scaleSeq.IsSequence() && scaleSeq.size() == 3) {
                m_Scale.x = scaleSeq[0].as<float>();
                m_Scale.y = scaleSeq[1].as<float>();
                m_Scale.z = scaleSeq[2].as<float>();
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }

        return true;
    }

#pragma endregion

#pragma region ComponentBase

    template<typename T>
    T* Component::GetComponent() {
        Entity entity = GetEntity();
        if(entity)
            return &(entity.GetComponent<T>());
        return nullptr;
    }
    
    // Explicitly instantiate the template for specific types
    template Component*                     Component::GetComponent<Component>();
    template IDComponent*                   Component::GetComponent<IDComponent>();
    template TagComponent*                  Component::GetComponent<TagComponent>();
    template HierarchyComponent*            Component::GetComponent<HierarchyComponent>();
    template CameraComponent*               Component::GetComponent<CameraComponent>();
    template AudioListener*                 Component::GetComponent<AudioListener>();
    template AudioEmitter*                  Component::GetComponent<AudioEmitter>();
    template Transform*                     Component::GetComponent<Transform>();
    template AssetMeshFilter*               Component::GetComponent<AssetMeshFilter>();
    template AssetMeshRenderer*             Component::GetComponent<AssetMeshRenderer>();
    template PointLightComponent*           Component::GetComponent<PointLightComponent>();
    template SpotLightComponent*            Component::GetComponent<SpotLightComponent>();
    template DirectionalLightComponent*     Component::GetComponent<DirectionalLightComponent>();
    template SkyboxComponent*               Component::GetComponent<SkyboxComponent>();
    template Rigidbody*                     Component::GetComponent<Rigidbody>();
    template SphereCollider*                Component::GetComponent<SphereCollider>();
    template BoxCollider*                   Component::GetComponent<BoxCollider>();

    template<typename T>
    T* Component::GetComponentInParent() {

        Entity entity = GetEntity();

        // Check if entity has parent
        if (entity && entity.GetComponent<HierarchyComponent>().HasParent() && entity.GetComponent<HierarchyComponent>().GetParentEntity()) {

            Entity parentEntity = entity.GetScene()->FindEntityByUUID(entity.GetComponent<HierarchyComponent>().GetParentID());

            // Check if parent has entity
            if(parentEntity && parentEntity.HasComponent<T>())
                return &(parentEntity.GetComponent<T>());

            // Recursively check parents upwards for if they have this component
            if (parentEntity.GetComponent<HierarchyComponent>().HasParent())
                return parentEntity.GetComponent<HierarchyComponent>().GetComponentInParent<T>();

        }

        return nullptr; // No component found - return nullptr
    }
    
    // Explicitly instantiate the template for specific types
    template Component*                     Component::GetComponentInParent<Component>();
    template IDComponent*                   Component::GetComponentInParent<IDComponent>();
    template TagComponent*                  Component::GetComponentInParent<TagComponent>();
    template HierarchyComponent*            Component::GetComponentInParent<HierarchyComponent>();
    template CameraComponent*               Component::GetComponentInParent<CameraComponent>();
    template AudioListener*                 Component::GetComponentInParent<AudioListener>();
    template AudioEmitter*                  Component::GetComponentInParent<AudioEmitter>();
    template Transform*                     Component::GetComponentInParent<Transform>();
    template AssetMeshFilter*               Component::GetComponentInParent<AssetMeshFilter>();
    template AssetMeshRenderer*             Component::GetComponentInParent<AssetMeshRenderer>();
    template PointLightComponent*           Component::GetComponentInParent<PointLightComponent>();
    template SpotLightComponent*            Component::GetComponentInParent<SpotLightComponent>();
    template DirectionalLightComponent*     Component::GetComponentInParent<DirectionalLightComponent>();
    template SkyboxComponent*               Component::GetComponentInParent<SkyboxComponent>();
    template Rigidbody*                     Component::GetComponentInParent<Rigidbody>();
    template SphereCollider*                Component::GetComponentInParent<SphereCollider>();
    template BoxCollider*                   Component::GetComponentInParent<BoxCollider>();

    template<typename T>
    std::vector<T*> Component::GetComponentsInParent() {

        Entity entity = GetEntity();

        // Check if entity has parent
        if (entity && entity.GetComponent<HierarchyComponent>().HasParent() && entity.GetComponent<HierarchyComponent>().GetParentEntity()) {

            Entity parentEntity = entity.GetScene()->FindEntityByUUID(entity.GetComponent<HierarchyComponent>().GetParentID());

            std::vector<T*> component_vector;

            // Add component if the parent entity has it
            if (parentEntity.HasComponent<T>()) {
                component_vector.push_back(&(parentEntity.GetComponent<T>()));
            }

            // Recursively check parents upwards for if they have this component
            if (parentEntity.GetComponent<HierarchyComponent>().HasParent())
            {
                std::vector<T*> parent_component_vector;

                parent_component_vector = parentEntity.GetComponent<HierarchyComponent>().GetComponentsInParent<T>();

                component_vector.insert(component_vector.end(), parent_component_vector.begin(), parent_component_vector.end());
            }

            return component_vector;
        }

        return std::vector<T*>(); // No component found - return empty vector
    }

    // Explicitly instantiate the template for specific types
    template std::vector<Component*>                    Component::GetComponentsInParent<Component>();
    template std::vector<IDComponent*>                  Component::GetComponentsInParent<IDComponent>();
    template std::vector<TagComponent*>                 Component::GetComponentsInParent<TagComponent>();
    template std::vector<HierarchyComponent*>           Component::GetComponentsInParent<HierarchyComponent>();
    template std::vector<CameraComponent*>              Component::GetComponentsInParent<CameraComponent>();
    template std::vector<AudioListener*>                Component::GetComponentsInParent<AudioListener>();
    template std::vector<AudioEmitter*>                 Component::GetComponentsInParent<AudioEmitter>();
    template std::vector<Transform*>                    Component::GetComponentsInParent<Transform>();
    template std::vector<AssetMeshFilter*>              Component::GetComponentsInParent<AssetMeshFilter>();
    template std::vector<AssetMeshRenderer*>            Component::GetComponentsInParent<AssetMeshRenderer>();
    template std::vector<PointLightComponent*>          Component::GetComponentsInParent<PointLightComponent>();
    template std::vector<SpotLightComponent*>           Component::GetComponentsInParent<SpotLightComponent>();
    template std::vector<DirectionalLightComponent*>    Component::GetComponentsInParent<DirectionalLightComponent>();
    template std::vector<SkyboxComponent*>              Component::GetComponentsInParent<SkyboxComponent>();
    template std::vector<Rigidbody*>                    Component::GetComponentsInParent<Rigidbody>();
    template std::vector<SphereCollider*>               Component::GetComponentsInParent<SphereCollider>();
    template std::vector<BoxCollider*>                  Component::GetComponentsInParent<BoxCollider>();

    template<typename T>
    T* Component::GetComponentInChildren() {

        Entity entity = GetEntity();

        // Check if entity has children
        if (entity && !entity.GetComponent<HierarchyComponent>().GetChildren().empty()) {

            auto& children_vector = entity.GetComponent<HierarchyComponent>().GetChildren();

            // Does the current entity have any children with the component we are looking for?
            for (auto& child_uuid : children_vector) {

                Entity child_entity = entity.GetScene()->FindEntityByUUID(child_uuid);

                if (child_entity.HasComponent<T>())
                    return &(child_entity.GetComponent<T>());
            }

            // If not, let's start to recursively check the children of the children
            for (auto& child_uuid : children_vector) {

                Entity child_entity = entity.GetScene()->FindEntityByUUID(child_uuid);
                T* component_found = child_entity.GetComponent<Transform>().GetComponentInChildren<T>();

                if (component_found)
                    return component_found;

            }

        }

        return nullptr; // No component found - return nullptr
    }

    // Explicitly instantiate the template for specific types
    template Component*                     Component::GetComponentInChildren<Component>();
    template IDComponent*                   Component::GetComponentInChildren<IDComponent>();
    template TagComponent*                  Component::GetComponentInChildren<TagComponent>();
    template HierarchyComponent*            Component::GetComponentInChildren<HierarchyComponent>();
    template CameraComponent*               Component::GetComponentInChildren<CameraComponent>();
    template AudioListener*                 Component::GetComponentInChildren<AudioListener>();
    template AudioEmitter*                  Component::GetComponentInChildren<AudioEmitter>();
    template Transform*                     Component::GetComponentInChildren<Transform>();
    template AssetMeshFilter*               Component::GetComponentInChildren<AssetMeshFilter>();
    template AssetMeshRenderer*             Component::GetComponentInChildren<AssetMeshRenderer>();
    template PointLightComponent*           Component::GetComponentInChildren<PointLightComponent>();
    template SpotLightComponent*            Component::GetComponentInChildren<SpotLightComponent>();
    template DirectionalLightComponent*     Component::GetComponentInChildren<DirectionalLightComponent>();
    template SkyboxComponent*               Component::GetComponentInChildren<SkyboxComponent>();
    template Rigidbody*                     Component::GetComponentInChildren<Rigidbody>();
    template SphereCollider*                Component::GetComponentInChildren<SphereCollider>();
    template BoxCollider*                   Component::GetComponentInChildren<BoxCollider>();

    template<typename T>
    std::vector<T*> Component::GetComponentsInChildren()
    {
        std::vector<T*> components;

        Entity entity = GetEntity();

        if (entity && entity.HasComponent<HierarchyComponent>()) {
            auto& children_vector = entity.GetComponent<HierarchyComponent>().GetChildren();

            // Check all direct children first
            for (auto& child_uuid : children_vector) {
                Entity child_entity = entity.GetScene()->FindEntityByUUID(child_uuid);

                if (child_entity.HasComponent<T>()) {
                    components.push_back(&(child_entity.GetComponent<T>()));
                }

                auto child_components = child_entity.GetComponent<Transform>().GetComponentsInChildren<T>();
                components.insert(components.end(), child_components.begin(), child_components.end());

            }

        }

        return components;
    }

    // Explicitly instantiate the template for specific types
    template std::vector<Component*>                    Component::GetComponentsInChildren<Component>();
    template std::vector<IDComponent*>                  Component::GetComponentsInChildren<IDComponent>();
    template std::vector<TagComponent*>                 Component::GetComponentsInChildren<TagComponent>();
    template std::vector<HierarchyComponent*>           Component::GetComponentsInChildren<HierarchyComponent>();
    template std::vector<CameraComponent*>              Component::GetComponentsInChildren<CameraComponent>();
    template std::vector<AudioListener*>                Component::GetComponentsInChildren<AudioListener>();
    template std::vector<AudioEmitter*>                 Component::GetComponentsInChildren<AudioEmitter>();
    template std::vector<Transform*>                    Component::GetComponentsInChildren<Transform>();
    template std::vector<AssetMeshFilter*>              Component::GetComponentsInChildren<AssetMeshFilter>();
    template std::vector<AssetMeshRenderer*>            Component::GetComponentsInChildren<AssetMeshRenderer>();
    template std::vector<PointLightComponent*>          Component::GetComponentsInChildren<PointLightComponent>();
    template std::vector<SpotLightComponent*>           Component::GetComponentsInChildren<SpotLightComponent>();
    template std::vector<DirectionalLightComponent*>    Component::GetComponentsInChildren<DirectionalLightComponent>();
    template std::vector<SkyboxComponent*>              Component::GetComponentsInChildren<SkyboxComponent>();
    template std::vector<Rigidbody*>                    Component::GetComponentsInChildren<Rigidbody>();
    template std::vector<SphereCollider*>               Component::GetComponentsInChildren<SphereCollider>();
    template std::vector<BoxCollider*>                  Component::GetComponentsInChildren<BoxCollider>();

#pragma endregion

#pragma region Hierarchy

    // Helper method to recursively get child colliders without a Rigidbody
    static std::vector<Entity> GetChildCollidersWithoutRigidbody(Entity startEntity) {
        
        std::vector<Entity> colliders;

        if(!startEntity.HasComponent<Rigidbody>()) {

            if(startEntity.HasAnyComponent<SphereCollider, BoxCollider>())
                colliders.push_back(startEntity);

            for (UUID child_uuid : startEntity.GetComponent<HierarchyComponent>().GetChildren()) {

                // Recursively check the child's children
                Entity child_entity = startEntity.GetScene()->FindEntityByUUID(child_uuid);
                std::vector<Entity> child_colliders = GetChildCollidersWithoutRigidbody(child_entity);
                colliders.insert(colliders.end(), child_colliders.begin(), child_colliders.end());
            }
        }

        return colliders;
    }

    void HierarchyComponent::AttachParent(const UUID& newParentID) {

        if (newParentID == NULL_UUID) {
            return;
        }

        if (newParentID == m_Parent) {
            L_CORE_WARN("Cannot Attach Self as Parent!");
            return;
        }

        Entity entity = GetEntity();

        if (!entity || !entity.GetScene()) {
            L_CORE_ERROR("Cannot Attach Parent - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        for (const auto& child_uuid : m_Children) {

            if (child_uuid == newParentID) {
                L_CORE_WARN("Cannot Attach Child Entity As New Parent!");
                return;
            }

        }

        // 1. Is this Current Entity already connected to another parent? If so call DetachParent first
        if (m_Parent != NULL_UUID) 
            DetachParent();

        Entity new_parent_entity = entity.GetScene()->FindEntityByUUID(newParentID);
        if (!new_parent_entity) {
            L_CORE_ERROR("Cannot Attach Parent - Parent Entity Is Invalid! Entity({0}) will be at the root of the scene now.", entity.GetName());
            return;
        }

        // 2. Convert Current Global Transform to Local Transform relative to newParent
        // Get references to the relevant components
        auto& entity_transform = entity.GetComponent<Transform>();
        auto& parent_transform = new_parent_entity.GetComponent<Transform>();

        // Calculate the local transform relative to the new parent
        glm::mat4 localTransform = glm::inverse(parent_transform.GetGlobalTransform()) * entity_transform.GetGlobalTransform();
        entity_transform.SetPosition(localTransform[3]);
        entity_transform.SetRotation(glm::vec3(glm::degrees(glm::eulerAngles(glm::quat_cast(localTransform)))));
        entity_transform.SetScale(glm::vec3(
            glm::length(localTransform[0]),
            glm::length(localTransform[1]),
            glm::length(localTransform[2])
        ));
        entity_transform.m_LocalTransform = localTransform;
        entity_transform.m_GlobalTransform = new_parent_entity.GetComponent<Transform>().GetGlobalTransform() * localTransform;

        if (!entity.HasComponent<Rigidbody>()) {

            // 1. I need to recursively check my children to see if there are
            // any children entities that HAVE a SphereCollider or BoxCollider, and 
            // DO NOT HAVE a Rigidbody.
            std::vector<Entity> child_colliders = GetChildCollidersWithoutRigidbody(entity);

            // 2. Flag all children with Collider Components without Rigidbodies to update
            //    rigidbody reference in the physics system
            for (Entity child_entity : child_colliders) {

                // Attach the collider to the parent's Rigidbody
                if (child_entity.HasComponent<SphereCollider>()) {

                    child_entity.GetComponent<SphereCollider>().GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
                }
                if (child_entity.HasComponent<BoxCollider>()) {

                    child_entity.GetComponent<BoxCollider>().GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
                }
            }
        }

        // Finalise relationship 8==D~({})
        m_Parent = newParentID;
        new_parent_entity.GetComponent<HierarchyComponent>().m_Children.push_back(entity.GetUUID());
    }

    void HierarchyComponent::DetachParent() {

        Entity entity = GetEntity();

        if (!entity || !entity.GetScene()) {
            L_CORE_ERROR("Cannot Detach Parent - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        // 1. Calculate the child's global transform
        auto& entityTransform = entity.GetComponent<Transform>();
        glm::mat4 globalTransform = entityTransform.GetGlobalTransform();

        // 2. Update the child's local transform to match the global transform
        entityTransform.SetPosition(globalTransform[3]);
        entityTransform.SetRotation(glm::vec3(glm::degrees(glm::eulerAngles(glm::quat_cast(globalTransform)))));
        entityTransform.SetScale(glm::vec3(
            glm::length(globalTransform[0]),
            glm::length(globalTransform[1]),
            glm::length(globalTransform[2])
        ));
        entityTransform.m_LocalTransform = globalTransform;
        entityTransform.m_GlobalTransform = globalTransform;

        if (!entity.HasComponent<Rigidbody>()) {

            // 1. I need to recursively check my children to see if there are
            // any children entities that HAVE a SphereCollider or BoxCollider, and 
            // DO NOT HAVE a Rigidbody.
            std::vector<Entity> child_colliders = GetChildCollidersWithoutRigidbody(entity);

            // 2. Flag all children with Collider Components without Rigidbodies to update
            //    rigidbody reference in the physics system
            for (Entity child_entity : child_colliders) {

                // Attach the collider to the parent's Rigidbody
                if (child_entity.HasComponent<SphereCollider>()) {

                    child_entity.GetComponent<SphereCollider>().GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
                }
                if (child_entity.HasComponent<BoxCollider>()) {

                    child_entity.GetComponent<BoxCollider>().GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
                }
            }

        }

        if (m_Parent != NULL_UUID) {
            Entity parentEntity = entity.GetScene()->FindEntityByUUID(m_Parent);
            if (parentEntity) {
                auto& parentChildren = parentEntity.GetComponent<HierarchyComponent>().m_Children;

                // Erase-remove idiom to remove the child from the parent's children list
                parentChildren.erase(
                    std::remove(parentChildren.begin(), parentChildren.end(), entity.GetUUID()),
                    parentChildren.end()
                );
            }

            // Clear the parent ID
            m_Parent = NULL_UUID;
        }
    }

    void HierarchyComponent::DetachChildren() {

        Entity entity = GetEntity();

        if (!entity || !entity.GetScene()) {
            L_CORE_ERROR("Cannot Rehome Children - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        for (const auto& child : m_Children) {
            entity.GetScene()->FindEntityByUUID(child).GetComponent<HierarchyComponent>().DetachParent();
        }

        L_CORE_INFO("Detached {0} Children From Entity({1}).", m_Children.size(), entity.GetName());

        m_Children.clear();
    }

    void HierarchyComponent::RehomeChildren(const UUID& newParentID) {

        Entity entity = GetEntity();

        if (!entity || !entity.GetScene()) {
            L_CORE_ERROR("Cannot Rehome Children - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        Entity new_parent = entity.GetScene()->FindEntityByUUID(newParentID);
        if (!new_parent) {
            L_CORE_ERROR("Cannot Rehome Children - Parent Entity Is Invalid.");
            return;
        }

        for (const auto& child : m_Children) {

            entity.GetScene()->FindEntityByUUID(child).GetComponent<HierarchyComponent>().AttachParent(newParentID);

        }
        
        L_CORE_INFO("Rehomed {0} Children From Entity({1}) to Entity({2}).", m_Children.size(), entity.GetName(), new_parent.GetName());

        m_Children.clear();

    }

    Entity HierarchyComponent::FindChild(const UUID& childUUID) const {

        if (!scene) {
            L_CORE_ASSERT(scene, "Cannot Find Child - Current Entity Has Invalid Scene Reference!");
            return {};
        }

        if (scene->HasEntity(childUUID)) {
            return scene->FindEntityByUUID(childUUID);
        }

        return {};
    }

    Entity HierarchyComponent::FindChild(const std::string& childName) const {

        if (!scene) {
            L_CORE_ASSERT(scene, "Cannot Find Child - Current Entity Has Invalid Scene Reference!");
            return {};
        }

        if (scene->HasEntity(childName)) {
            return scene->FindEntityByName(childName);
        }

        return {};
    }

    const std::vector<UUID>& HierarchyComponent::GetChildren() const {
        return m_Children;
    }

    Entity HierarchyComponent::GetParentEntity() const {

        if (!HasParent()) {
            L_CORE_WARN("Cannot Get Parent Entity - No Parent Attached.");
            return {};
        }

        if (!scene) {
            L_CORE_ASSERT(scene, "Cannot Get Parent Entity - Current Entity Has Invalid Scene Reference!");
            return {};
        }

        return scene->FindEntityByUUID(m_Parent);
    }

    const UUID& HierarchyComponent::GetParentID() const {
        return m_Parent;
    }

    bool HierarchyComponent::HasParent() const {
        return m_Parent != NULL_UUID;
    }

    void HierarchyComponent::Serialize(YAML::Emitter& out) {

        out << YAML::Key << "HierarchyComponent";
        out << YAML::BeginMap;

        out << YAML::Key << "Parent" << YAML::Value << m_Parent;

        {
            out << YAML::Key << "Children" << YAML::Value;
            out << YAML::BeginSeq;
                
            for (const auto& child : m_Children) {
                out << child;
            }

            out << YAML::EndSeq;;
        }

        out << YAML::EndMap;
    }

    bool HierarchyComponent::Deserialize(const YAML::Node data) {

        YAML::Node component = data;

        // Deserialize the Parent value
        if (component["Parent"]) {
            m_Parent = component["Parent"].as<uint32_t>();  
        }

        // Deserialize the Children sequence
        if (component["Children"]) {

            if (component["Children"].IsSequence()) {
                
                m_Children.clear();
                for (size_t i = 0; i < component["Children"].size(); ++i) {
                    m_Children.push_back(component["Children"][i].as<uint32_t>());
                }

            }

        }
        return true;
    }

#pragma endregion

    void TagComponent::Serialize(YAML::Emitter& out) {

        out << YAML::Key << "TagComponent";
        out << YAML::BeginMap;

        out << YAML::Key << "Tag" << YAML::Value << Tag;

        out << YAML::EndMap;
    }

    bool TagComponent::Deserialize(const YAML::Node data)
    {
        if (data["Tag"])
            Tag = data["Tag"].as<std::string>();

        return true;
    }

    void CameraComponent::Serialize(YAML::Emitter& out)
    {

        if (CameraInstance) {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap;
            {

                out << YAML::Key << "Camera" << YAML::Value;
                out << YAML::BeginMap;
                {

                    out << YAML::Key << "FOV" << YAML::Value << CameraInstance->FOV;
                    out << YAML::Key << "MovementToggle" << YAML::Value << CameraInstance->m_Movement;
                    out << YAML::Key << "MovementSpeed" << YAML::Value << CameraInstance->MovementSpeed;
                    out << YAML::Key << "MovementYDamp" << YAML::Value << CameraInstance->MovementYDamp;
                    out << YAML::Key << "MouseSensitivity" << YAML::Value << CameraInstance->MouseSensitivity;
                    out << YAML::Key << "MouseToggledOff" << YAML::Value << CameraInstance->MouseToggledOff;

                    glm::vec3 v = CameraInstance->GetGlobalPosition();
                    out << YAML::Key << "Position" << YAML::Value << YAML::Flow
                        << YAML::BeginSeq
                        << v.x
                        << v.y
                        << v.z
                        << YAML::EndSeq;

                    out << YAML::Key << "Yaw" << YAML::Value << CameraInstance->GetYaw();
                    out << YAML::Key << "Pitch" << YAML::Value << CameraInstance->GetPitch();
                }
                out << YAML::EndMap;
            }

            out << YAML::Key << "Primary" << YAML::Value << Primary;

            const char* clear_string = (ClearFlags == CameraClearFlags::SKYBOX) ? "Skybox" : "Colour";
            out << YAML::Key << "ClearFlag" << YAML::Value << clear_string;

            glm::vec4 v  = ClearColour;
            out << YAML::Key << "ClearColour" << YAML::Value << YAML::Flow
                << YAML::BeginSeq
                << v.r
                << v.g
                << v.b
                << v.a
                << YAML::EndSeq;

            out << YAML::EndMap;
        }

    }

    bool CameraComponent::Deserialize(const YAML::Node data) {
        
        YAML::Node component = data;

        if (component["Camera"]) {
            YAML::Node cameraNode = component["Camera"];

            if (cameraNode["FOV"]) {
                float temp = cameraNode["FOV"].as<float>();
                CameraInstance->FOV = cameraNode["FOV"].as<float>();
            }
            else {
                return false;
            }

            if (cameraNode["MovementToggle"]) {
                CameraInstance->m_Movement = cameraNode["MovementToggle"].as<bool>();
            }
            else {
                return false;
            }

            if (cameraNode["MovementSpeed"]) {
                CameraInstance->MovementSpeed = cameraNode["MovementSpeed"].as<float>();
            }
            else {
                return false;
            }

            if (cameraNode["MovementYDamp"]) {
                CameraInstance->MovementYDamp = cameraNode["MovementYDamp"].as<float>();
            }
            else {
                return false;
            }

            if (cameraNode["MouseSensitivity"]) {
                CameraInstance->MouseSensitivity = cameraNode["MouseSensitivity"].as<float>();
            }
            else {
                return false;
            }

            if (cameraNode["MouseToggledOff"]) {
                CameraInstance->MouseToggledOff = cameraNode["MouseToggledOff"].as<bool>();
            }
            else {
                return false;
            }

            if (cameraNode["Position"]) {
                auto positionSeq = cameraNode["Position"];
                if (positionSeq.IsSequence() && positionSeq.size() == 3) {
                    glm::vec3 position{};
                    position.x = positionSeq[0].as<float>();
                    position.y = positionSeq[1].as<float>();
                    position.z = positionSeq[2].as<float>();
                    CameraInstance->SetPosition(position);
                }
                else {
                    return false;
                }
            }
            else {
                return false;
            }

            if (cameraNode["Yaw"]) {
                CameraInstance->SetYaw(cameraNode["Yaw"].as<float>());
            }
            else {
                return false;
            }

            if (cameraNode["Pitch"]) {
                CameraInstance->SetPitch(cameraNode["Pitch"].as<float>());
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }

        if (component["Primary"]) {
            Primary = component["Primary"].as<bool>();
        }
        else {
            return false;
        }

        if (component["ClearFlag"]) {
            std::string clearFlagStr = component["ClearFlag"].as<std::string>();
            if (clearFlagStr == "Skybox") {
                ClearFlags = CameraClearFlags::SKYBOX;
            }
            else if (clearFlagStr == "Colour") {
                ClearFlags = CameraClearFlags::COLOUR_ONLY;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }

        if (component["ClearColour"]) {
            auto clearColourSeq = component["ClearColour"];
            if (clearColourSeq.IsSequence() && clearColourSeq.size() == 4) {
                ClearColour.r = clearColourSeq[0].as<float>();
                ClearColour.g = clearColourSeq[1].as<float>();
                ClearColour.b = clearColourSeq[2].as<float>();
                ClearColour.a = clearColourSeq[3].as<float>();
            }
        }

        return true;
    }

    void IDComponent::Serialize(YAML::Emitter& out) {

        out << YAML::Key << "Entity" << YAML::Value << ID;
    }

    bool IDComponent::Deserialize(const YAML::Node data) {

        if (data)
            ID = data.as<uint32_t>();
        else
            return false;

        return true;
    }
    Entity Component::GetEntity() const {

        if (!scene) {
            L_CORE_ASSERT(scene, "Cannot Get Entity - Current Entity Has Invalid Scene Reference!");
            return {};
        }
        
        if (!scene->HasEntity(entity_uuid)) {
            L_CORE_ASSERT(scene, "Cannot Get Entity - Current Entity Is Invalid!");
            return {};
        }

        return scene->FindEntityByUUID(entity_uuid);
    }
}