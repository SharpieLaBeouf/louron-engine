#include "Components.h"

// Louron Core Headers
#include "../../Core/Logging.h"
#include "../Entity.h"
#include "../Scene.h"

#include "Physics/Collider.h"
#include "Physics/Rigidbody.h"
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

    TransformComponent::TransformComponent() {
        AddFlag(TransformFlag_PropertiesUpdated);
    }

    TransformComponent::TransformComponent(const TransformComponent& other)
    {
        if (!scene)
            scene = other.scene;

        if (entity_uuid == NULL_UUID)
            entity_uuid = other.entity_uuid;

        m_Position = other.m_Position;
        m_Rotation = other.m_Rotation;
        m_Scale = other.m_Scale;

        m_LocalTransform = other.m_LocalTransform;
        m_GlobalTransform = other.m_GlobalTransform;

        m_StateFlags = other.m_StateFlags;
        AddFlag(TransformFlag_PropertiesUpdated);
    }

    TransformComponent::TransformComponent(TransformComponent&& other) noexcept
    {
        scene = other.scene; other.scene = nullptr;
        entity_uuid = other.entity_uuid; other.entity_uuid = NULL_UUID;

        m_Position = other.m_Position; other.m_Position = glm::vec3(0.0f);
        m_Rotation = other.m_Rotation; other.m_Rotation = glm::vec3(0.0f);
        m_Scale = other.m_Scale; other.m_Scale = glm::vec3(1.0f);

        m_LocalTransform = other.m_LocalTransform; other.m_LocalTransform = glm::mat4(1.0f);
        m_GlobalTransform = other.m_GlobalTransform; other.m_GlobalTransform = glm::mat4(1.0f);

        m_StateFlags = other.m_StateFlags; other.m_StateFlags = TransformFlag_None;

        AddFlag(TransformFlag_PropertiesUpdated);
    }

    TransformComponent& TransformComponent::operator=(const TransformComponent& other)
    {
        if (this == &other)
            return *this;


        if (!scene)
            scene = other.scene;

        m_Position = other.m_Position;
        m_Rotation = other.m_Rotation;
        m_Scale = other.m_Scale;

        m_LocalTransform = other.m_LocalTransform;
        m_GlobalTransform = other.m_GlobalTransform;

        m_StateFlags = other.m_StateFlags;

        AddFlag(TransformFlag_PropertiesUpdated);

        return *this;
    }

    TransformComponent& TransformComponent::operator=(TransformComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        scene = other.scene; other.scene = nullptr;
        entity_uuid = other.entity_uuid; other.entity_uuid = NULL_UUID;

        m_Position = other.m_Position; other.m_Position = glm::vec3(0.0f);
        m_Rotation = other.m_Rotation; other.m_Rotation = glm::vec3(0.0f);
        m_Scale = other.m_Scale; other.m_Scale = glm::vec3(1.0f);

        m_LocalTransform = other.m_LocalTransform; other.m_LocalTransform = glm::mat4(1.0f);
        m_GlobalTransform = other.m_GlobalTransform; other.m_GlobalTransform = glm::mat4(1.0f);

        m_StateFlags = other.m_StateFlags; other.m_StateFlags = TransformFlag_None;

        AddFlag(TransformFlag_PropertiesUpdated);

        return *this;
    }

    TransformComponent::TransformComponent(const glm::vec3& translation) : m_Position(translation) {
        AddFlag(TransformFlag_PropertiesUpdated);
    }

    void TransformComponent::AddFlag(TransformFlags flag) { m_StateFlags = static_cast<TransformFlags>(m_StateFlags | flag); }
    void TransformComponent::RemoveFlag(TransformFlags flag) { m_StateFlags = static_cast<TransformFlags>(m_StateFlags & ~flag); }
    bool TransformComponent::CheckFlag(TransformFlags flag) const { return (m_StateFlags & static_cast<TransformFlags>(flag)) != TransformFlag_None; }
    bool TransformComponent::NoFlagsSet() const { return m_StateFlags == TransformFlag_None; }
    void TransformComponent::ClearFlags() { m_StateFlags = TransformFlag_None; }
    TransformFlags TransformComponent::GetFlags() const { return m_StateFlags; }

    /// <summary>
    /// Set the position to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed position.</param>
    void TransformComponent::SetPosition(const glm::vec3& newPosition) {

        m_Position = newPosition;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void TransformComponent::SetPositionX(const float& newXPosition) {
        m_Position.x = newXPosition;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void TransformComponent::SetPositionY(const float& newYPosition) {
        m_Position.y = newYPosition;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void TransformComponent::SetPositionZ(const float& newZPosition) {
        m_Position.z = newZPosition;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Set the rotation to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed rotation.</param>
    void TransformComponent::SetRotation(const glm::vec3& newRotation) {
        m_Rotation = newRotation;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void TransformComponent::SetRotationX(const float& newXRotation) {
        m_Rotation.x = newXRotation;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void TransformComponent::SetRotationY(const float& newYRotation) {
        m_Rotation.y = newYRotation;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void TransformComponent::SetRotationZ(const float& newZRotation) {
        m_Rotation.z = newZRotation;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Set the scale to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed scale.</param>
    void TransformComponent::SetScale(const glm::vec3& newScale) {
        m_Scale = newScale;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void TransformComponent::SetScaleX(const float& newXScale) {
        m_Scale.x = newXScale;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void TransformComponent::SetScaleY(const float& newYScale) {
        m_Scale.y = newYScale;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    void TransformComponent::SetScaleZ(const float& newZScale) {
        m_Scale.z = newZScale;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Translation to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current position.</param>
    void TransformComponent::Translate(const glm::vec3& vector) {
        m_Position += vector;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Translation to the Transform along the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X position.</param>
    void TransformComponent::TranslateX(const float& deltaTranslationX) {
        m_Position.x += deltaTranslationX;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Translation to the Transform along the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y position.</param>
    void TransformComponent::TranslateY(const float& deltaTranslationY) {
        m_Position.y += deltaTranslationY;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Translation to the Transform along the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z position.</param>
    void TransformComponent::TranslateZ(const float& deltaTranslationZ) {
        m_Position.z += deltaTranslationZ;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }
    /// <summary>
    /// Apply a Rotation to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current rotation.</param>
    void TransformComponent::Rotate(const glm::vec3& vector) {
        m_Rotation += vector;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X rotation.</param>
    void TransformComponent::RotateX(const float& deltaRotationX) {
        m_Rotation.x += deltaRotationX;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y rotation.</param>
    void TransformComponent::RotateY(const float& deltaRotationY) {
        m_Rotation.y += deltaRotationY;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z rotation.</param>
    void TransformComponent::RotateZ(const float& deltaRotationZ) {
        m_Rotation.z += deltaRotationZ;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Scale to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current scale.</param>
    void TransformComponent::Scale(const glm::vec3& vector) {
        m_Scale += vector;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Scale to the Transform along the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X scale.</param>
    void TransformComponent::ScaleX(const float& deltaScaleX) {
        m_Scale.x += deltaScaleX;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Scale to the Transform along the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y scale.</param>
    void TransformComponent::ScaleY(const float& deltaScaleY) {
        m_Scale.y += deltaScaleY;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    /// <summary>
    /// Apply a Scale to the Transform along the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z scale.</param>
    void TransformComponent::ScaleZ(const float& deltaScaleZ) {
        m_Scale.z += deltaScaleZ;
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    const glm::vec3& TransformComponent::GetLocalPosition() const    { return m_Position;    }
    const glm::vec3& TransformComponent::GetLocalRotation() const    { return m_Rotation;    }
    const glm::vec3& TransformComponent::GetLocalScale() const       { return m_Scale;       }

    void TransformComponent::SetGlobalPosition(const glm::vec3& globalPosition) {

        Entity entity = GetEntity();
        if (entity && entity.GetScene() && entity.GetComponent<HierarchyComponent>().HasParent()) {
           
            glm::mat4 parentGlobalMatrix = GetComponentInParent<TransformComponent>()->GetGlobalTransform();
            glm::mat4 parentInverseMatrix = glm::inverse(parentGlobalMatrix);
            glm::vec4 localPosition4 = parentInverseMatrix * glm::vec4(globalPosition, 1.0f);
            SetPosition(glm::vec3(localPosition4));
        }
        else {
            SetPosition(globalPosition);
        }
    }

    void TransformComponent::SetGlobalRotation(const glm::vec3& globalRotation) {

        Entity entity = GetEntity();

        if (entity && entity.GetScene() && entity.GetComponent<HierarchyComponent>().HasParent()) {
            
            glm::quat parentGlobalRotation = glm::quat(glm::radians(GetComponentInParent<TransformComponent>()->GetGlobalRotation()));
            glm::quat parentInverseRotation = glm::inverse(parentGlobalRotation);

            glm::quat globalQuat = glm::quat(glm::radians(globalRotation));
            glm::quat localQuat = parentInverseRotation * globalQuat;
            glm::vec3 localRot = glm::degrees(glm::eulerAngles(localQuat));
            SetRotation(localRot);
        }
        else {
            SetRotation(globalRotation);
        }
    }

    void TransformComponent::SetGlobalScale(const glm::vec3& globalScale) {

        Entity entity = GetEntity();

        if (entity && entity.GetScene() && entity.GetComponent<HierarchyComponent>().HasParent()) {

            glm::vec3 parentGlobalScale = GetComponentInParent<TransformComponent>()->GetGlobalScale();
            m_Scale = globalScale / parentGlobalScale;
        }
        else {
            m_Scale = globalScale;
        }

        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalTransformMatrix();
    }

    glm::vec3 TransformComponent::GetPositionFromMatrix(const glm::mat4& transform) {
        // Extract the position directly from the translation components of the matrix
        return glm::vec3(transform[3][0], transform[3][1], transform[3][2]);
    }

    glm::vec3 TransformComponent::GetRotationFromMatrix(const glm::mat4& transform) {
        // Extract the upper-left 3x3 submatrix (rotation and scale)
        glm::mat3 rotationMatrix = glm::mat3(transform);

        // Remove the scaling by normalizing the basis vectors
        rotationMatrix[0] = glm::normalize(rotationMatrix[0]);
        rotationMatrix[1] = glm::normalize(rotationMatrix[1]);
        rotationMatrix[2] = glm::normalize(rotationMatrix[2]);

        // Convert the purified rotation matrix to a quaternion
        return glm::degrees(glm::eulerAngles(glm::quat_cast(rotationMatrix)));
    }

    glm::vec3 TransformComponent::GetScaleFromMatrix(const glm::mat4& transform) {
        // Extract the scale from the lengths of the basis vectors
        return glm::vec3(
            glm::length(glm::vec3(transform[0][0], transform[0][1], transform[0][2])),
            glm::length(glm::vec3(transform[1][0], transform[1][1], transform[1][2])),
            glm::length(glm::vec3(transform[2][0], transform[2][1], transform[2][2]))
        );
    }

    void TransformComponent::OnTransformUpdated() {

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

                child_entity.GetComponent<TransformComponent>().OnTransformUpdated();
            }
        }

    }

    void TransformComponent::UpdateLocalTransformMatrix() {

        Entity entity = GetEntity();

        // Check if any changes made to local transform, if YES WE UPDATE
        if (CheckFlag(TransformFlag_PropertiesUpdated)) {

            // Compute the local transform matrix if it has changed
            m_LocalTransform =  glm::translate(glm::mat4(1.0f), m_Position) *
                                glm::mat4_cast(glm::quat(glm::radians(m_Rotation))) *
                                glm::scale(glm::mat4(1.0f), m_Scale);

            OnTransformUpdated();

            if (entity && entity.GetScene() && entity.HasComponent<RigidbodyComponent>() && entity.GetComponent<RigidbodyComponent>().GetActor())
                entity.GetComponent<RigidbodyComponent>().GetActor()->AddFlag(RigidbodyFlag_TransformUpdated);

            RemoveFlag(TransformFlag_PropertiesUpdated);
        }

    }

    glm::vec3 TransformComponent::GetGlobalPosition() {
        if (CheckFlag(TransformFlag_GlobalTransformUpdated)) 
            return GetGlobalTransform()[3];

        if(m_LocalTransform != m_GlobalTransform)
            return m_GlobalTransform[3];

        return m_Position;
    }

    glm::vec3 TransformComponent::GetGlobalRotation() {

        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
            return GetRotationFromMatrix(GetGlobalTransform());

        if (m_LocalTransform != m_GlobalTransform)
            return GetRotationFromMatrix(m_GlobalTransform);
        

        return m_Rotation;
    }

    glm::vec3 TransformComponent::GetGlobalScale() {

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

    constexpr glm::vec3 global_forward = glm::vec3{ 0.0f, 0.0f, -1.0f };
    constexpr glm::vec3 global_right = glm::vec3{ 1.0f, 0.0f, 0.0f };
    constexpr glm::vec3 global_up = glm::vec3{ 0.0f, 1.0f, 0.0f };

    void TransformComponent::SetForwardDirection(const glm::vec3& direction)
    {
        glm::vec3 forward = glm::normalize(direction);
        glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(glm::rotation(global_forward, forward)));
        SetGlobalRotation(eulerRotation); // Set Global
    }

    void TransformComponent::SetRightDirection(const glm::vec3& direction)
    {
        glm::vec3 right = glm::normalize(direction);
        glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(glm::rotation(global_right, right)));
        SetGlobalRotation(eulerRotation); // Set Global
    }

    void TransformComponent::SetUpDirection(const glm::vec3& direction)
    {
        glm::vec3 up = glm::normalize(direction);
        glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(glm::rotation(global_up, up)));
        SetGlobalRotation(eulerRotation); // Set Global
    }

    glm::vec3 TransformComponent::GetForwardDirection()
    {
        glm::vec3 transform;

        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
            transform = GetGlobalRotation();
        else if (m_LocalTransform != m_GlobalTransform)
            transform = GetRotationFromMatrix(m_GlobalTransform);
        else
            transform = GetRotationFromMatrix(m_LocalTransform);

        glm::quat globalRotation = glm::quat(glm::radians(transform));
        glm::vec3 globalForward = globalRotation * global_forward;
        return glm::normalize(globalForward);
    }

    glm::vec3 TransformComponent::GetRightDirection()
    {
        glm::vec3 transform;

        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
            transform = GetGlobalRotation();
        else if (m_LocalTransform != m_GlobalTransform)
            transform = GetRotationFromMatrix(m_GlobalTransform);
        else
            transform = GetRotationFromMatrix(m_LocalTransform);

        glm::quat globalRotation = glm::quat(glm::radians(transform));
        glm::vec3 globalForward = globalRotation * global_right;
        return glm::normalize(globalForward);
    }

    glm::vec3 TransformComponent::GetUpDirection()
    {
        glm::vec3 transform;

        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
            transform = GetGlobalRotation();
        else if (m_LocalTransform != m_GlobalTransform)
            transform = GetRotationFromMatrix(m_GlobalTransform);
        else
            transform = GetRotationFromMatrix(m_LocalTransform);

        glm::quat globalRotation = glm::quat(glm::radians(transform));
        glm::vec3 globalForward = globalRotation * global_up;
        return glm::normalize(globalForward);
    }
    const glm::mat4& TransformComponent::GetGlobalTransform() { 

        if (CheckFlag(TransformFlag_GlobalTransformUpdated)) {

            glm::vec3 old_global_scale = glm::vec3(
                glm::length(m_GlobalTransform[0]),
                glm::length(m_GlobalTransform[1]),
                glm::length(m_GlobalTransform[2])
            ); // Can't call GetGlobalScale here as it would send it into a never ending recursion as GetGlobalScale will call GetGlobalTransform if the flag is not cleared

            Entity entity = GetEntity();

            if (entity && entity.GetScene() && entity.GetComponent<HierarchyComponent>().HasParent())
                m_GlobalTransform = entity.GetComponent<HierarchyComponent>().GetParentEntity().GetComponent<TransformComponent>().GetGlobalTransform() * GetLocalTransform();
            else
                m_GlobalTransform = m_LocalTransform;

            RemoveFlag(TransformFlag_GlobalTransformUpdated);

            if (entity) {

                if (entity.HasComponent<SphereColliderComponent>()) {

                    auto& component = entity.GetComponent<SphereColliderComponent>();

                    if (component.GetShape() && component.GetShape()->IsStatic())
                        component.AddFlag(ColliderFlag_TransformUpdated);

                    if (old_global_scale != GetGlobalScale())
                        component.AddFlag(ColliderFlag_ShapePropsUpdated); // TODO: Fix this because it goes on the fritz when child is attached to parent, and the parent scale changes

                }
                
                if (entity.HasComponent<BoxColliderComponent>()) {

                    auto& component = entity.GetComponent<BoxColliderComponent>();

                    if(component.GetShape() && component.GetShape()->IsStatic())
                        component.AddFlag(ColliderFlag_TransformUpdated);

                    if (old_global_scale != GetGlobalScale())
                        component.AddFlag(ColliderFlag_ShapePropsUpdated);

                }
            }
        }
        return m_GlobalTransform;
    }

    const glm::mat4& TransformComponent::GetLocalTransform() {
        
        UpdateLocalTransformMatrix();
        return m_LocalTransform;
    }

    void TransformComponent::SetTransform(const glm::mat4& transform)
    {    
        // Decompose Position
        glm::vec3 position = glm::vec3(transform[3]); // Extract translation (last column)

        // Extract Scale
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(transform[0])); // Length of X column
        scale.y = glm::length(glm::vec3(transform[1])); // Length of Y column
        scale.z = glm::length(glm::vec3(transform[2])); // Length of Z column

        // Normalize rotation matrix (remove scale)
        glm::mat3 rotationMatrix = glm::mat3(transform);
        rotationMatrix[0] /= scale.x;
        rotationMatrix[1] /= scale.y;
        rotationMatrix[2] /= scale.z;

        // Convert to quaternion
        glm::quat rotationQuat = glm::quat_cast(rotationMatrix);

        // Convert quaternion to Euler angles (in degrees)
        glm::vec3 rotationEuler = glm::degrees(glm::eulerAngles(rotationQuat));

        // Set internal values
        m_Position = position;
        m_Scale = scale;
        m_Rotation = rotationEuler;

        m_LocalTransform = transform;
    }

    TransformComponent::operator const glm::mat4()& { return m_LocalTransform; }

    glm::mat4 TransformComponent::operator*(const TransformComponent& other) const { return m_LocalTransform * other.m_LocalTransform; }

    void TransformComponent::Serialize(YAML::Emitter& out) {

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

    bool TransformComponent::Deserialize(const YAML::Node data)
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

    Entity Component::GetEntity() const {

        if (!scene) {
            L_CORE_ERROR("Cannot Get Entity - Current Entity Has Invalid Scene Reference!");
            return {};
        }

        if (!scene->HasEntity(entity_uuid)) {
            L_CORE_ERROR("Cannot Get Entity - Current Entity Is Invalid!");
            return {};
        }

        return scene->FindEntityByUUID(entity_uuid);
    }

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
    template ScriptComponent*               Component::GetComponent<ScriptComponent>();
    template CameraComponent*               Component::GetComponent<CameraComponent>();
    template AudioListener*                 Component::GetComponent<AudioListener>();
    template AudioEmitter*                  Component::GetComponent<AudioEmitter>();
    template TransformComponent*            Component::GetComponent<TransformComponent>();
    template AssetMeshFilter*               Component::GetComponent<AssetMeshFilter>();
    template AssetMeshRenderer*             Component::GetComponent<AssetMeshRenderer>();
    template LODMeshComponent*              Component::GetComponent<LODMeshComponent>();
    template PointLightComponent*           Component::GetComponent<PointLightComponent>();
    template SpotLightComponent*            Component::GetComponent<SpotLightComponent>();
    template DirectionalLightComponent*     Component::GetComponent<DirectionalLightComponent>();
    template SkyboxComponent*               Component::GetComponent<SkyboxComponent>();
    template RigidbodyComponent*            Component::GetComponent<RigidbodyComponent>();
    template SphereColliderComponent*       Component::GetComponent<SphereColliderComponent>();
    template BoxColliderComponent*          Component::GetComponent<BoxColliderComponent>();

    template<typename T>
    T* Component::GetComponentInParent() {

        Entity entity = GetEntity();

        // Check if entity has parent
        if (entity && entity.GetComponent<HierarchyComponent>().HasParent()) {

            Entity parentEntity = entity.GetComponent<HierarchyComponent>().GetParentEntity();

            if (!parentEntity)
                return nullptr;

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
    template ScriptComponent*               Component::GetComponentInParent<ScriptComponent>();
    template CameraComponent*               Component::GetComponentInParent<CameraComponent>();
    template AudioListener*                 Component::GetComponentInParent<AudioListener>();
    template AudioEmitter*                  Component::GetComponentInParent<AudioEmitter>();
    template TransformComponent*            Component::GetComponentInParent<TransformComponent>();
    template AssetMeshFilter*               Component::GetComponentInParent<AssetMeshFilter>();
    template AssetMeshRenderer*             Component::GetComponentInParent<AssetMeshRenderer>();
    template LODMeshComponent*              Component::GetComponentInParent<LODMeshComponent>();
    template PointLightComponent*           Component::GetComponentInParent<PointLightComponent>();
    template SpotLightComponent*            Component::GetComponentInParent<SpotLightComponent>();
    template DirectionalLightComponent*     Component::GetComponentInParent<DirectionalLightComponent>();
    template SkyboxComponent*               Component::GetComponentInParent<SkyboxComponent>();
    template RigidbodyComponent*            Component::GetComponentInParent<RigidbodyComponent>();
    template SphereColliderComponent*       Component::GetComponentInParent<SphereColliderComponent>();
    template BoxColliderComponent*          Component::GetComponentInParent<BoxColliderComponent>();

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
    template std::vector<ScriptComponent*>              Component::GetComponentsInParent<ScriptComponent>();
    template std::vector<CameraComponent*>              Component::GetComponentsInParent<CameraComponent>();
    template std::vector<AudioListener*>                Component::GetComponentsInParent<AudioListener>();
    template std::vector<AudioEmitter*>                 Component::GetComponentsInParent<AudioEmitter>();
    template std::vector<TransformComponent*>           Component::GetComponentsInParent<TransformComponent>();
    template std::vector<AssetMeshFilter*>              Component::GetComponentsInParent<AssetMeshFilter>();
    template std::vector<AssetMeshRenderer*>            Component::GetComponentsInParent<AssetMeshRenderer>();
    template std::vector<LODMeshComponent*>             Component::GetComponentsInParent<LODMeshComponent>();
    template std::vector<PointLightComponent*>          Component::GetComponentsInParent<PointLightComponent>();
    template std::vector<SpotLightComponent*>           Component::GetComponentsInParent<SpotLightComponent>();
    template std::vector<DirectionalLightComponent*>    Component::GetComponentsInParent<DirectionalLightComponent>();
    template std::vector<SkyboxComponent*>              Component::GetComponentsInParent<SkyboxComponent>();
    template std::vector<RigidbodyComponent*>           Component::GetComponentsInParent<RigidbodyComponent>();
    template std::vector<SphereColliderComponent*>      Component::GetComponentsInParent<SphereColliderComponent>();
    template std::vector<BoxColliderComponent*>         Component::GetComponentsInParent<BoxColliderComponent>();

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
                T* component_found = child_entity.GetComponent<TransformComponent>().GetComponentInChildren<T>();

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
    template ScriptComponent*               Component::GetComponentInChildren<ScriptComponent>();
    template CameraComponent*               Component::GetComponentInChildren<CameraComponent>();
    template AudioListener*                 Component::GetComponentInChildren<AudioListener>();
    template AudioEmitter*                  Component::GetComponentInChildren<AudioEmitter>();
    template TransformComponent*            Component::GetComponentInChildren<TransformComponent>();
    template AssetMeshFilter*               Component::GetComponentInChildren<AssetMeshFilter>();
    template AssetMeshRenderer*             Component::GetComponentInChildren<AssetMeshRenderer>();
    template LODMeshComponent*              Component::GetComponentInChildren<LODMeshComponent>();
    template PointLightComponent*           Component::GetComponentInChildren<PointLightComponent>();
    template SpotLightComponent*            Component::GetComponentInChildren<SpotLightComponent>();
    template DirectionalLightComponent*     Component::GetComponentInChildren<DirectionalLightComponent>();
    template SkyboxComponent*               Component::GetComponentInChildren<SkyboxComponent>();
    template RigidbodyComponent*            Component::GetComponentInChildren<RigidbodyComponent>();
    template SphereColliderComponent*       Component::GetComponentInChildren<SphereColliderComponent>();
    template BoxColliderComponent*          Component::GetComponentInChildren<BoxColliderComponent>();

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

                auto child_components = child_entity.GetComponent<TransformComponent>().GetComponentsInChildren<T>();
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
    template std::vector<ScriptComponent*>              Component::GetComponentsInChildren<ScriptComponent>();
    template std::vector<CameraComponent*>              Component::GetComponentsInChildren<CameraComponent>();
    template std::vector<AudioListener*>                Component::GetComponentsInChildren<AudioListener>();
    template std::vector<AudioEmitter*>                 Component::GetComponentsInChildren<AudioEmitter>();
    template std::vector<TransformComponent*>           Component::GetComponentsInChildren<TransformComponent>();
    template std::vector<AssetMeshFilter*>              Component::GetComponentsInChildren<AssetMeshFilter>();
    template std::vector<AssetMeshRenderer*>            Component::GetComponentsInChildren<AssetMeshRenderer>();
    template std::vector<LODMeshComponent*>             Component::GetComponentsInChildren<LODMeshComponent>();
    template std::vector<PointLightComponent*>          Component::GetComponentsInChildren<PointLightComponent>();
    template std::vector<SpotLightComponent*>           Component::GetComponentsInChildren<SpotLightComponent>();
    template std::vector<DirectionalLightComponent*>    Component::GetComponentsInChildren<DirectionalLightComponent>();
    template std::vector<SkyboxComponent*>              Component::GetComponentsInChildren<SkyboxComponent>();
    template std::vector<RigidbodyComponent*>           Component::GetComponentsInChildren<RigidbodyComponent>();
    template std::vector<SphereColliderComponent*>      Component::GetComponentsInChildren<SphereColliderComponent>();
    template std::vector<BoxColliderComponent*>         Component::GetComponentsInChildren<BoxColliderComponent>();

#pragma endregion

#pragma region Hierarchy

    // Helper method to recursively get child colliders without a Rigidbody
    static std::vector<Entity> GetChildCollidersWithoutRigidbody(Entity startEntity) {
        
        std::vector<Entity> colliders;

        if(!startEntity.HasComponent<RigidbodyComponent>()) {

            if(startEntity.HasAnyComponent<SphereColliderComponent, BoxColliderComponent>())
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
        auto& entity_transform = entity.GetComponent<TransformComponent>();
        auto& parent_transform = new_parent_entity.GetComponent<TransformComponent>();

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
        entity_transform.m_GlobalTransform = new_parent_entity.GetComponent<TransformComponent>().GetGlobalTransform() * localTransform;

        if (!entity.HasComponent<RigidbodyComponent>()) {

            // 1. I need to recursively check my children to see if there are
            // any children entities that HAVE a SphereCollider or BoxCollider, and 
            // DO NOT HAVE a Rigidbody.
            std::vector<Entity> child_colliders = GetChildCollidersWithoutRigidbody(entity);

            // 2. Flag all children with Collider Components without Rigidbodies to update
            //    rigidbody reference in the physics system
            for (Entity child_entity : child_colliders) {

                // Attach the collider to the parent's Rigidbody
                if (child_entity.HasComponent<SphereColliderComponent>()) {

                    child_entity.GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
                }
                if (child_entity.HasComponent<BoxColliderComponent>()) {

                    child_entity.GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
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
        auto& entityTransform = entity.GetComponent<TransformComponent>();
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

        if (!entity.HasComponent<RigidbodyComponent>()) {

            // 1. I need to recursively check my children to see if there are
            // any children entities that HAVE a SphereCollider or BoxCollider, and 
            // DO NOT HAVE a Rigidbody.
            std::vector<Entity> child_colliders = GetChildCollidersWithoutRigidbody(entity);

            // 2. Flag all children with Collider Components without Rigidbodies to update
            //    rigidbody reference in the physics system
            for (Entity child_entity : child_colliders) {

                // Attach the collider to the parent's Rigidbody
                if (child_entity.HasComponent<SphereColliderComponent>()) {

                    child_entity.GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
                }
                if (child_entity.HasComponent<BoxColliderComponent>()) {

                    child_entity.GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
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

        if (GetEntity().GetUUID() == childUUID)
        {
            L_CORE_WARN("Cannot Pass Child UUID to Self.");
            return {};
        }

        if (scene->HasEntity(childUUID)) {

            // First check current children
            for (const auto& child : m_Children)
                if (child && child == childUUID)
                    return scene->FindEntityByUUID(child);

            // If still not found, recurse to next level
            for (const auto& child : m_Children)
                if(child)
                    return scene->FindEntityByUUID(childUUID).GetComponent<HierarchyComponent>().FindChild(childUUID);
        }

        return {};
    }

    Entity HierarchyComponent::FindChild(const std::string& childName) const {

        if (!scene) {
            L_CORE_ASSERT(scene, "Cannot Find Child - Current Entity Has Invalid Scene Reference!");
            return {};
        }

        if (GetEntity().GetName() == childName)
        {
            L_CORE_WARN("Cannot Pass Child UUID to Self.");
            return {};
        }

        if (scene->HasEntity(childName)) {

            // First check current children
            for (const auto& child : m_Children)
                if (Entity child_entity = scene->FindEntityByUUID(child); child_entity && child_entity.GetName() == childName)
                    return child_entity;

            // If still not found, recurse to next level
            for (const auto& child : m_Children)
                if (Entity child_entity = scene->FindEntityByUUID(child); child_entity)
                    return child_entity.GetComponent<HierarchyComponent>().FindChild(childName);
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

    void TagComponent::SetUniqueName(const std::string& name)
    {
        std::string uniqueName = name.empty() ? "Untitled Entity" : name;
        int suffix = 1;
        std::string baseName = uniqueName;

        // Ensure the name is unique by appending a numeric suffix
        auto check_tags = [&](const char* name) -> bool {

            if (GetComponent<HierarchyComponent>()->HasParent()) { // Has Parent
                std::vector<UUID> uuids = GetComponentInParent<HierarchyComponent>()->GetChildren();
                for (auto& uuid : uuids) {

                    if (uuid == entity_uuid)
                        continue;

                    const TagComponent& tag = scene->FindEntityByUUID(uuid).GetComponent<TagComponent>();
                    if (tag.Tag == name)
                        return true;
                }
            }
            else { // At Root Level

                auto view = scene->GetAllEntitiesWith<HierarchyComponent, TagComponent>();
                for (auto& entity : view) {

                    if (!view.get<HierarchyComponent>(entity).HasParent()) {
                        if (view.get<TagComponent>(entity).Tag == name)
                            return true;
                    }
                }
            }

            return false;
        };

        while (check_tags(uniqueName.c_str())) {
            uniqueName = baseName + " (" + std::to_string(suffix++) + ")";
        }
        Tag = uniqueName;
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
                    const char* type_string = (CameraInstance->GetProjectionType() == SceneCamera::ProjectionType::Perspective) ? "Perspective" : "Orthographic";
                    out << YAML::Key << "Projection Type" << YAML::Value << type_string;
                    if (type_string == "Perspective")
                    {
                        out << YAML::Key << "FOV" << YAML::Value << glm::degrees(CameraInstance->GetPerspectiveVerticalFOV());
                        out << YAML::Key << "Near" << YAML::Value << CameraInstance->GetPerspectiveNearClip();
                        out << YAML::Key << "Far" << YAML::Value << CameraInstance->GetPerspectiveFarClip();
                    }
                    else {
                        out << YAML::Key << "FOV" << YAML::Value << CameraInstance->GetOrthographicSize();
                        out << YAML::Key << "Near" << YAML::Value << CameraInstance->GetOrthographicNearClip();
                        out << YAML::Key << "Far" << YAML::Value << CameraInstance->GetOrthographicFarClip();
                    }
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

            if (cameraNode["Projection Type"] && cameraNode["Projection Type"].as<std::string>() == "Perspective")
            {
                CameraInstance->SetProjectionType(SceneCamera::ProjectionType::Perspective);

                if (cameraNode["FOV"]) {
                    CameraInstance->SetPerspectiveVerticalFOV(glm::radians(cameraNode["FOV"].as<float>()));
                }
                else {
                    return false;
                }
                if (cameraNode["Near"]) {
                    CameraInstance->SetPerspectiveNearClip(cameraNode["Near"].as<float>());
                }
                else {
                    return false;
                }
                if (cameraNode["Far"]) {
                    CameraInstance->SetPerspectiveFarClip(cameraNode["Far"].as<float>());
                }
                else {
                    return false;
                }
            }
            else if (cameraNode["Projection Type"] && cameraNode["Projection Type"].as<std::string>() == "Orthographic")
            {
                CameraInstance->SetProjectionType(SceneCamera::ProjectionType::Orthographic);

                if (cameraNode["FOV"]) {
                    CameraInstance->SetOrthographicSize(cameraNode["FOV"].as<float>());
                }
                else {
                    return false;
                }
                if (cameraNode["Near"]) {
                    CameraInstance->SetOrthographicNearClip(cameraNode["Near"].as<float>());
                }
                else {
                    return false;
                }
                if (cameraNode["Far"]) {
                    CameraInstance->SetOrthographicFarClip(cameraNode["Far"].as<float>());
                }
                else {
                    return false;
                }
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

    void LODMeshComponent::Serialize(YAML::Emitter& out)
    {
        out << YAML::Key << "LODMeshComponent";
        out << YAML::BeginMap;

        if (LOD_Elements.empty())
        {
            out << YAML::EndMap;
            return;
        }

        out << YAML::Key << "MaxDistanceOverFarPlane" << MaxDistanceOverFarPlane;
        out << YAML::Key << "MaxDistance" << MaxDistance;

        out << YAML::Key << "LOD Elements" << YAML::BeginMap;
        for (int i = 0; i < LOD_Elements.size(); i++)
        {
            out << YAML::Key << "Element " + std::to_string(i) << YAML::Value;
            out << YAML::BeginMap;
            {
                out << YAML::Key << "Distance Threshold" << YAML::Value << LOD_Elements[i].DistanceThresholdNormalised;

                out << YAML::Key << "Entities" << YAML::Value;
                out << YAML::BeginSeq;
                for (const auto& entity_uuid : LOD_Elements[i].MeshRendererEntities) {
                    out << (uint32_t)entity_uuid;
                }
                out << YAML::EndSeq;
            }
            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        out << YAML::EndMap;
    }

    bool LODMeshComponent::Deserialize(const YAML::Node data)
    {
        if (data["MaxDistanceOverFarPlane"])
            MaxDistanceOverFarPlane = data["MaxDistanceOverFarPlane"].as<bool>();

        if (data["MaxDistance"])
            MaxDistance = data["MaxDistance"].as<float>();

        if (YAML::Node elements = data["LOD Elements"]; elements && elements.size() > 0)
        {
            LOD_Elements.clear();

            for (auto it = elements.begin(); it != elements.end();)
            {
                LOD_Elements.push_back({});

                LOD_Elements.back().DistanceThresholdNormalised = it->second["Distance Threshold"].as<float>();

                auto entitiesSeq = it->second["Entities"];
                if (entitiesSeq.IsSequence() && entitiesSeq.size() > 0) {

                    for (int i = 0; i < entitiesSeq.size(); i++)
                    {
                        LOD_Elements.back().MeshRendererEntities.push_back(entitiesSeq[i].as<uint32_t>());
                    }
                }

                ++it;
            }
        }

        return true;
    }

    void ScriptComponent::Serialize(YAML::Emitter& out) const
    {
        Entity entity = GetEntity();

        if (!entity) {
            return;
        }

        out << YAML::Key << "ScriptComponent" << YAML::Value;
        out << YAML::BeginMap;

        int i = 1;
        for (auto& [script_name, active] : Scripts) {

            out << YAML::Key << "Script_" + std::to_string(i) << YAML::Value;
            out << YAML::BeginMap;
            
            out << YAML::Key << "Active" << YAML::Value << active;
            out << YAML::Key << "Script Name" << YAML::Value << script_name;

            auto entity_class = ScriptManager::GetEntityClass(script_name);

            if (!entity_class)
                continue;

            auto& class_fields = entity_class->GetFields();
            if (class_fields.size() > 0) {

                auto& entity_fields = ScriptManager::GetScriptFieldMap(entity.GetUUID(), script_name);

                out << YAML::Key << "Fields" << YAML::Value;
                out << YAML::BeginSeq;
                for (const auto& [name, field] : entity_fields) {


                    out << YAML::BeginMap;

                    out << YAML::Key << "Name" << YAML::Value << name;
                    out << YAML::Key << "Type" << YAML::Value << ScriptingUtils::ScriptFieldTypeToString(field.Field.Type);

                    switch (field.Field.Type) {

                        case ScriptFieldType::Entity:
                        case ScriptFieldType::TransformComponent:
                        case ScriptFieldType::TagComponent:
                        case ScriptFieldType::ScriptComponent:
                        case ScriptFieldType::PointLightComponent:
                        case ScriptFieldType::SpotLightComponent:
                        case ScriptFieldType::DirectionalLightComponent:
                        case ScriptFieldType::RigidbodyComponent:
                        case ScriptFieldType::BoxColliderComponent:
                        case ScriptFieldType::SphereColliderComponent:
                        case ScriptFieldType::Component:
                        case ScriptFieldType::ComputeShader:
                        case ScriptFieldType::Prefab:   out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<UUID>();           break;

                        case ScriptFieldType::Bool:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<bool>();           break;
                        case ScriptFieldType::Byte:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<uint16_t>();       break; // Have to get data as uint16_t and not uint8_t because uint8_t is unsigned char which serialises as an alphanumeric character
                        case ScriptFieldType::Sbyte:    out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<int8_t>();         break;
                        case ScriptFieldType::Char:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<char>();           break;
                        case ScriptFieldType::Decimal:  out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<double>();         break;
                        case ScriptFieldType::Double:   out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<double>();         break;
                        case ScriptFieldType::Float:    out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<float>();          break;
                        case ScriptFieldType::Int:      out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<int>();            break;
                        case ScriptFieldType::Uint:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<unsigned int>();   break;
                        case ScriptFieldType::Long:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<int64_t>();        break;
                        case ScriptFieldType::Ulong:    out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<uint64_t>();       break;
                        case ScriptFieldType::Short:    out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<int16_t>();        break;
                        case ScriptFieldType::Ushort:   out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<uint16_t>();       break;

                        case ScriptFieldType::Vector2:
                        {
                            glm::vec2 v = entity_fields.at(name).GetValue<glm::vec2>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << YAML::EndSeq;   
                            break;
                        }
                        case ScriptFieldType::Vector3:
                        {
                            glm::vec3 v = entity_fields.at(name).GetValue<glm::vec3>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::Vector4:
                        {
                            glm::vec4 v = entity_fields.at(name).GetValue<glm::vec4>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << v.w
                                << YAML::EndSeq;
                            break;
                        }

                    }

                    out << YAML::EndMap;


                }
                out << YAML::EndSeq;
            }

            
            out << YAML::EndMap;

            i++;
        }

        out << YAML::EndMap;
    }

    bool ScriptComponent::Deserialize(const YAML::Node data, UUID entity_uuid)
    {
        for (int i = 1; i < data.size() + 1; i++) {

            std::string index = "Script_" + std::to_string(i);
            YAML::Node script_node = data[index.c_str()];

            if (script_node) {

                bool active = script_node["Active"].as<bool>();
                std::string script_name = script_node["Script Name"].as<std::string>();

                Scripts.push_back({ script_name, active});

                auto field_node = script_node["Fields"];
                if (field_node) {

                    auto entity_class = ScriptManager::GetEntityClass(script_name);
                    if (entity_class) {

                        const auto& fields = entity_class->GetFields();
                        auto& entity_fields = ScriptManager::GetScriptFieldMap(entity_uuid, script_name);

                        for (auto script_field : field_node) {

                            std::string field_name = script_field["Name"].as<std::string>();
                            ScriptFieldType type = ScriptingUtils::ScriptFieldTypeFromString(script_field["Type"].as<std::string>());

                            ScriptFieldInstance& field_instance = entity_fields[field_name];

                            if (fields.find(field_name) == fields.end())
                            {
                                L_CORE_WARN("Field Name ({}) Not Found In Script Class({}).", field_name, script_name);
                                continue;
                            }

                            field_instance.Field = fields.at(field_name);

                            switch (type) {

                                case ScriptFieldType::Entity:
                                case ScriptFieldType::TransformComponent:
                                case ScriptFieldType::TagComponent:
                                case ScriptFieldType::ScriptComponent:
                                case ScriptFieldType::PointLightComponent:
                                case ScriptFieldType::SpotLightComponent:
                                case ScriptFieldType::DirectionalLightComponent:
                                case ScriptFieldType::RigidbodyComponent:
                                case ScriptFieldType::BoxColliderComponent:
                                case ScriptFieldType::SphereColliderComponent:
                                case ScriptFieldType::ComputeShader:
                                case ScriptFieldType::Component:
                                case ScriptFieldType::Prefab:   { Louron::UUID data = script_field["Data"].as<uint32_t>(); field_instance.SetValue(data); break; }
                                                                                                                                                                    
                                case ScriptFieldType::Bool:     { bool data = script_field["Data"].as<bool>(); field_instance.SetValue(data);                 break; }
                                case ScriptFieldType::Byte:     { uint8_t data = script_field["Data"].as<uint8_t>(); field_instance.SetValue(data);           break; }
                                case ScriptFieldType::Sbyte:    { int8_t data = script_field["Data"].as<int8_t>(); field_instance.SetValue(data);             break; }
                                case ScriptFieldType::Char:     { char data = script_field["Data"].as<char>(); field_instance.SetValue(data);                 break; }
                                case ScriptFieldType::Decimal:  { double data = script_field["Data"].as<double>(); field_instance.SetValue(data);             break; }
                                case ScriptFieldType::Double:   { double data = script_field["Data"].as<double>(); field_instance.SetValue(data);             break; }
                                case ScriptFieldType::Float:    { float data = script_field["Data"].as<float>(); field_instance.SetValue(data);               break; }
                                case ScriptFieldType::Int:      { int data = script_field["Data"].as<int>(); field_instance.SetValue(data);                   break; }
                                case ScriptFieldType::Uint:     { unsigned int data = script_field["Data"].as<unsigned int>(); field_instance.SetValue(data); break; }
                                case ScriptFieldType::Long:     { int64_t data = script_field["Data"].as<int64_t>(); field_instance.SetValue(data);           break; }
                                case ScriptFieldType::Ulong:    { uint64_t data = script_field["Data"].as<uint64_t>(); field_instance.SetValue(data);         break; }
                                case ScriptFieldType::Short:    { int16_t data = script_field["Data"].as<int16_t>(); field_instance.SetValue(data);           break; }
                                case ScriptFieldType::Ushort:   { uint16_t data = script_field["Data"].as<uint16_t>(); field_instance.SetValue(data);         break; }

                                case ScriptFieldType::Vector2:
                                {
                                    if(script_field["Data"].IsSequence() && script_field["Data"].size() == 2)
                                    {
                                        glm::vec2 v;
                                        v.x = script_field["Data"][0].as<float>();
                                        v.y = script_field["Data"][1].as<float>();
                                        field_instance.SetValue(v);
                                    }
                                    break;
                                }
                                case ScriptFieldType::Vector3:
                                {
                                    if (script_field["Data"].IsSequence() && script_field["Data"].size() == 3)
                                    {
                                        glm::vec3 v;
                                        v.x = script_field["Data"][0].as<float>();
                                        v.y = script_field["Data"][1].as<float>();
                                        v.z = script_field["Data"][2].as<float>();
                                        field_instance.SetValue(v);
                                    }
                                    break;
                                }
                                case ScriptFieldType::Vector4:
                                {
                                    if (script_field["Data"].IsSequence() && script_field["Data"].size() == 4)
                                    {
                                        glm::vec4 v;
                                        v.x = script_field["Data"][0].as<float>();
                                        v.y = script_field["Data"][1].as<float>();
                                        v.z = script_field["Data"][2].as<float>();
                                        v.w = script_field["Data"][3].as<float>();
                                        field_instance.SetValue(v);
                                    }
                                    break;
                                }
                            }

                        }

                    }

                }

            }
        }
        return true;
    }
}