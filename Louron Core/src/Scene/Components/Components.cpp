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
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Louron {

#pragma region TransformComponent

    Transform::Transform() {
        AddFlag(TransformFlag_PositionUpdated);
    }

    Transform::Transform(const glm::vec3& translation) : m_Position(translation) {
        AddFlag(TransformFlag_PositionUpdated);
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
        AddFlag(TransformFlag_PositionUpdated);
    }

    void Transform::SetPositionX(const float& newXPosition) {
        m_Position.x = newXPosition;
        AddFlag(TransformFlag_PositionUpdated);
    }

    void Transform::SetPositionY(const float& newYPosition) {
        m_Position.y = newYPosition;
        AddFlag(TransformFlag_PositionUpdated);
    }

    void Transform::SetPositionZ(const float& newZPosition) {
        m_Position.z = newZPosition;
        AddFlag(TransformFlag_PositionUpdated);
    }

    /// <summary>
    /// Set the rotation to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed rotation.</param>
    void Transform::SetRotation(const glm::vec3& newRotation) {
        m_Rotation = newRotation;
        AddFlag(TransformFlag_RotationUpdated);
    }

    void Transform::SetRotationX(const float& newXRotation) {
        m_Rotation.x = newXRotation;
        AddFlag(TransformFlag_RotationUpdated);
    }

    void Transform::SetRotationY(const float& newYRotation) {
        m_Rotation.y = newYRotation;
        AddFlag(TransformFlag_RotationUpdated);
    }

    void Transform::SetRotationZ(const float& newZRotation) {
        m_Rotation.z = newZRotation;
        AddFlag(TransformFlag_RotationUpdated);
    }

    /// <summary>
    /// Set the scale to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed scale.</param>
    void Transform::SetScale(const glm::vec3& newScale) {
        m_Scale = newScale;
        AddFlag(TransformFlag_ScaleUpdated);
    }

    void Transform::SetScaleX(const float& newXScale) {
        m_Scale.x = newXScale;
        AddFlag(TransformFlag_ScaleUpdated);
    }

    void Transform::SetScaleY(const float& newYScale) {
        m_Scale.y = newYScale;
        AddFlag(TransformFlag_ScaleUpdated);
    }

    void Transform::SetScaleZ(const float& newZScale) {
        m_Scale.z = newZScale;
        AddFlag(TransformFlag_ScaleUpdated);
    }

    /// <summary>
    /// Apply a Translation to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current position.</param>
    void Transform::Translate(const glm::vec3& vector) {
        m_Position += vector;
        AddFlag(TransformFlag_PositionUpdated);
    }

    /// <summary>
    /// Apply a Translation to the Transform along the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X position.</param>
    void Transform::TranslateX(const float& deltaTranslationX) {
        m_Position.x += deltaTranslationX;
        AddFlag(TransformFlag_PositionUpdated);
    }

    /// <summary>
    /// Apply a Translation to the Transform along the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y position.</param>
    void Transform::TranslateY(const float& deltaTranslationY) {
        m_Position.y += deltaTranslationY;
        AddFlag(TransformFlag_PositionUpdated);
    }

    /// <summary>
    /// Apply a Translation to the Transform along the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z position.</param>
    void Transform::TranslateZ(const float& deltaTranslationZ) {
        m_Position.z += deltaTranslationZ;
        AddFlag(TransformFlag_PositionUpdated);
    }
    /// <summary>
    /// Apply a Rotation to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current rotation.</param>
    void Transform::Rotate(const glm::vec3& vector) {
        m_Rotation += vector;
        AddFlag(TransformFlag_RotationUpdated);
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X rotation.</param>
    void Transform::RotateX(const float& deltaRotationX) {
        m_Rotation.x += deltaRotationX;
        AddFlag(TransformFlag_RotationUpdated);
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y rotation.</param>
    void Transform::RotateY(const float& deltaRotationY) {
        m_Rotation.y += deltaRotationY;
        AddFlag(TransformFlag_RotationUpdated);
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z rotation.</param>
    void Transform::RotateZ(const float& deltaRotationZ) {
        m_Rotation.z += deltaRotationZ;
        AddFlag(TransformFlag_RotationUpdated);
    }

    /// <summary>
    /// Apply a Scale to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current scale.</param>
    void Transform::Scale(const glm::vec3& vector) {
        m_Scale += vector;
        AddFlag(TransformFlag_ScaleUpdated);
    }

    /// <summary>
    /// Apply a Scale to the Transform along the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X scale.</param>
    void Transform::ScaleX(const float& deltaScaleX) {
        m_Scale.x += deltaScaleX;
        AddFlag(TransformFlag_ScaleUpdated);
    }

    /// <summary>
    /// Apply a Scale to the Transform along the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y scale.</param>
    void Transform::ScaleY(const float& deltaScaleY) {
        m_Scale.y += deltaScaleY;
        AddFlag(TransformFlag_ScaleUpdated);
    }

    /// <summary>
    /// Apply a Scale to the Transform along the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z scale.</param>
    void Transform::ScaleZ(const float& deltaScaleZ) {
        m_Scale.z += deltaScaleZ;
        AddFlag(TransformFlag_ScaleUpdated);
    }

    const glm::vec3& Transform::GetLocalPosition() { return m_Position; }
    const glm::vec3& Transform::GetLocalRotation() { return m_Rotation; }
    const glm::vec3& Transform::GetLocalScale() { return m_Scale; }


    glm::vec3 Transform::GetPosition() {
        if (m_WorldTransform.has_value()) {
            glm::mat4 transform = m_WorldTransform.value() * glm::translate(glm::mat4(1.0f), m_Position);
            return glm::vec3(transform[3]);
        }
        return m_Position;
    }
    glm::vec3 Transform::GetRotation() {
        if (m_WorldTransform.has_value()) {

            glm::mat4 transform = m_WorldTransform.value() * glm::toMat4(glm::quat(glm::radians(m_Rotation)));
            return glm::vec3(glm::degrees(glm::eulerAngles(glm::quat_cast(transform))));
        }
        return m_Rotation;
    }
    glm::vec3 Transform::GetScale() {
        if (m_WorldTransform.has_value()) {

            glm::mat4 transform = m_WorldTransform.value() * glm::scale(glm::mat4(1.0f), m_Scale);
            return glm::vec3(
                glm::length(transform[0]),
                glm::length(transform[1]),
                glm::length(transform[2])
            );
        }
        return m_Scale;
    }

    glm::mat4 Transform::GetTransform() const { 
        if (m_WorldTransform.has_value())
            return m_WorldTransform.value() * m_Transform;
        return m_Transform;
    }

    glm::mat4 Transform::GetLocalTransform() const { return glm::translate(glm::mat4(1.0f), m_Position)
        * glm::toMat4(glm::quat(glm::radians(m_Rotation)))
        * glm::scale(glm::mat4(1.0f), m_Scale);
    }

    void Transform::SetTransform(const glm::mat4& transform) {
        m_Transform = transform;
    }

    Transform::operator const glm::mat4()& { return m_Transform; }

    glm::mat4 Transform::operator*(const Transform& other) const {
        return m_Transform * other.m_Transform;
    }

#pragma endregion


#pragma region ComponentBase

    template<typename T>
    T& Component::GetComponent() {
        return entity->GetComponent<T>();
    }
    
    // Explicitly instantiate the template for specific types
    template Component&                     Component::GetComponent<Component>();
    template IDComponent&                   Component::GetComponent<IDComponent>();
    template TagComponent&                  Component::GetComponent<TagComponent>();
    template CameraComponent&               Component::GetComponent<CameraComponent>();
    template AudioListener&                 Component::GetComponent<AudioListener>();
    template AudioEmitter&                  Component::GetComponent<AudioEmitter>();
    template Transform&                     Component::GetComponent<Transform>();
    template MeshFilter&                    Component::GetComponent<MeshFilter>();
    template MeshRenderer&                  Component::GetComponent<MeshRenderer>();
    template PointLightComponent&           Component::GetComponent<PointLightComponent>();
    template SpotLightComponent&            Component::GetComponent<SpotLightComponent>();
    template DirectionalLightComponent&     Component::GetComponent<DirectionalLightComponent>();
    template SkyboxComponent&               Component::GetComponent<SkyboxComponent>();
    template Rigidbody&                     Component::GetComponent<Rigidbody>();
    template SphereCollider&                Component::GetComponent<SphereCollider>();
    template BoxCollider&                   Component::GetComponent<BoxCollider>();

#pragma endregion

}