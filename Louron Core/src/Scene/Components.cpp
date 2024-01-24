#include "Components.h"

namespace Louron {

    Transform::Transform() {
        UpdateTransform();
    }

    Transform::Transform(const glm::vec3& translation) : m_Position(translation) {
        UpdateTransform();
    }

    /// <summary>
    /// Set the position to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed position.</param>
    void Transform::SetPosition(const glm::vec3& newPosition) {
        m_Position = newPosition;
        UpdateTransform();
    }

    void Transform::SetPositionX(const float& newXPosition) {
        m_Position.x = newXPosition;
        UpdateTransform();
    }

    void Transform::SetPositionY(const float& newYPosition) {
        m_Position.y = newYPosition;
        UpdateTransform();
    }

    void Transform::SetPositionZ(const float& newZPosition) {
        m_Position.z = newZPosition;
        UpdateTransform();
    }

    /// <summary>
    /// Set the rotation to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed rotation.</param>
    void Transform::SetRotation(const glm::vec3& newRotation) {
        m_Rotation = newRotation;
        UpdateTransform();
    }

    void Transform::SetRotationX(const float& newXRotation) {
        m_Rotation.x = newXRotation;
        UpdateTransform();
    }

    void Transform::SetRotationY(const float& newYRotation) {
        m_Rotation.y = newYRotation;
        UpdateTransform();
    }

    void Transform::SetRotationZ(const float& newZRotation) {
        m_Rotation.z = newZRotation;
        UpdateTransform();
    }

    /// <summary>
    /// Set the scale to a fixed value.
    /// </summary>
    /// <param name="newScale">This will be the new fixed scale.</param>
    void Transform::SetScale(const glm::vec3& newScale) {
        m_Scale = newScale;
        UpdateTransform();
    }

    void Transform::SetScaleX(const float& newXScale) {
        m_Scale.x = newXScale;
        UpdateTransform();
    }

    void Transform::SetScaleY(const float& newYScale) {
        m_Scale.y = newYScale;
        UpdateTransform();
    }

    void Transform::SetScaleZ(const float& newZScale) {
        m_Scale.z = newZScale;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Translation to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current position.</param>
    void Transform::Translate(const glm::vec3& vector) {
        m_Position += vector;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Translation to the Transform along the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X position.</param>
    void Transform::TranslateX(const float& deltaTranslationX) {
        m_Position.x += deltaTranslationX;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Translation to the Transform along the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y position.</param>
    void Transform::TranslateY(const float& deltaTranslationY) {
        m_Position.y += deltaTranslationY;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Translation to the Transform along the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z position.</param>
    void Transform::TranslateZ(const float& deltaTranslationZ) {
        m_Position.z += deltaTranslationZ;
        UpdateTransform();
    }
    /// <summary>
    /// Apply a Rotation to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current rotation.</param>
    void Transform::Rotate(const glm::vec3& vector) {
        m_Rotation += vector;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X rotation.</param>
    void Transform::RotateX(const float& deltaRotationX) {
        m_Rotation.x += deltaRotationX;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y rotation.</param>
    void Transform::RotateY(const float& deltaRotationY) {
        m_Rotation.y += deltaRotationY;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Rotation to the Transform around the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z rotation.</param>
    void Transform::RotateZ(const float& deltaRotationZ) {
        m_Rotation.z += deltaRotationZ;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Scale to the Transform.
    /// </summary>
    /// <param name="vector">This will be added to the current scale.</param>
    void Transform::Scale(const glm::vec3& vector) {
        m_Scale += vector;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Scale to the Transform along the X-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current X scale.</param>
    void Transform::ScaleX(const float& deltaScaleX) {
        m_Scale.x += deltaScaleX;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Scale to the Transform along the Y-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Y scale.</param>
    void Transform::ScaleY(const float& deltaScaleY) {
        m_Scale.y += deltaScaleY;
        UpdateTransform();
    }

    /// <summary>
    /// Apply a Scale to the Transform along the Z-axis.
    /// </summary>
    /// <param name="delta">This value will be added to the current Z scale.</param>
    void Transform::ScaleZ(const float& deltaScaleZ) {
        m_Scale.z += deltaScaleZ;
        UpdateTransform();
    }

    const glm::vec3& Transform::GetPosition() { return m_Position; }
    const glm::vec3& Transform::GetRotation() { return m_Rotation; }
    const glm::vec3& Transform::GetScale() { return m_Scale; }

    glm::mat4 Transform::GetTransform() const { return m_Transform; }

    void Transform::UpdateTransform() {
        m_Transform = glm::translate(glm::mat4(1.0f), m_Position)
            * glm::toMat4(glm::quat(glm::radians(m_Rotation)))
            * glm::scale(glm::mat4(1.0f), m_Scale);
    }
}