#include "Collider.h"

// Louron Core Headers
#include "../Mesh.h"
#include "../Components.h"

#include "Rigidbody.h"
#include "PhysicsWrappers.h"

#include "../../../Core/Logging.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

#pragma region SphereCollider


    SphereCollider::SphereCollider() {
        m_Material = std::make_shared<PhysicsMaterial>();
        m_Shape = std::make_shared<PhysicsShape>(PxSphereGeometry(m_Radius * 2.0f), *m_Material->GetMaterial());
    }

    SphereCollider::SphereCollider(glm::vec3 SphereCentre, float SphereRadius) : m_Centre(SphereCentre), m_Radius(SphereRadius) {
        m_Material = std::make_shared<PhysicsMaterial>();
        m_Shape = std::make_shared<PhysicsShape>(PxSphereGeometry(m_Radius * 2.0f), *m_Material->GetMaterial());
    }

    SphereCollider::~SphereCollider() {

    }

    void SphereCollider::Release() {

        if(m_Shape) {
            m_Shape->Release();
            m_Shape = nullptr;
        }

        if(m_Material)
            m_Material = nullptr;

    }

    // GETTERS
    const bool& SphereCollider::GetIsTrigger() const { return m_IsTrigger; }
    std::shared_ptr<PhysicsMaterial> SphereCollider::GetMaterial() const { return m_Material; }

    const glm::vec3& SphereCollider::GetCentre() const { return m_Centre; }
    const float& SphereCollider::GetRadius() const { return m_Radius; }

    std::shared_ptr<PhysicsShape> SphereCollider::GetShape() const { return m_Shape; }

    // SETTERS
    void SphereCollider::SetIsTrigger(bool isTrigger) {
        m_IsTrigger = isTrigger;
        if (m_Shape) {
            m_Shape->SetFlag(PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
            m_Shape->SetFlag(PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
        }
    }

    void SphereCollider::SetMaterial(std::shared_ptr<PhysicsMaterial> material) {
        if (material) {
            m_Material = material;
            if (m_Shape) {
                m_Shape->SetMaterials((PxMaterial* const*)m_Material->GetMaterial(), 1);
            }
        }
        else {
            L_CORE_ERROR("Invalid Material!");
        }
    }

    void SphereCollider::SetCentre(const glm::vec3& centre) {
        m_Centre = centre;

        m_Shape->AddFlag(ColliderFlag_ShapePropsUpdated);
    }

    void SphereCollider::SetRadius(float radius) {
        m_Radius = radius;

        m_Shape->AddFlag(ColliderFlag_ShapePropsUpdated);
    }


    void SphereCollider::UpdateTransform(Transform& collider_transform, Transform& rigidbody_transform) {

        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);

        glm::mat4 collider_matrix = collider_transform.GetTransform();
        glm::mat4 rigidbody_matrix = rigidbody_transform.GetTransform();

        if (m_Shape->IsStatic() || collider_matrix == rigidbody_matrix) {

            // If the collider does not refer to a Rigidbody Component, we simply
            // apply the custom collider centre, and multiply the transforms
            // largest absolute scale value by the m_Radius modifier.
            position = m_Centre;
            scale = collider_transform.GetScale();

            m_Shape->SetLocalPose(PxTransform(position.x, position.y, position.z));
            m_Shape->SetGeometry(PxSphereGeometry(glm::compMax(glm::abs(scale)) * (m_Radius * 2.0f)));

            if (auto rb_ref = m_Shape->GetRigidbody(); rb_ref && *rb_ref) {

                rb_ref->SetGlobalPose(collider_transform);
                rb_ref->AddFlag(RigidbodyFlag_ShapesUpdated);
            }
        }
        else {

            // If the collider and rigidbody transform is not the same, we find 
            // the local pose of the collider relative to the rigidbody, then 
            // add the m_Centre offset to the local position, and multiply the
            // largest absolute scale value by the m_Radius modifier.
            m_Shape->SetGeometry(PxSphereGeometry(glm::compMax(glm::abs(collider_transform.GetScale())) * (m_Radius * 2.0f)));

            collider_matrix = glm::inverse(rigidbody_matrix) * collider_matrix;

            scale = glm::vec3(glm::length(collider_matrix[0]), glm::length(collider_matrix[1]), glm::length(collider_matrix[2]));
            position = glm::vec3(collider_matrix[3]) * glm::abs(collider_transform.GetScale()) + m_Centre;
            glm::quat quaternion = glm::quat_cast(collider_matrix);

            m_Shape->SetLocalPose(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

            if (auto rb_ref = m_Shape->GetRigidbody(); rb_ref && *rb_ref) {

                rb_ref->AddFlag(RigidbodyFlag_ShapesUpdated);
            }
        }
    }

#pragma endregion

#pragma region BoxCollider


    BoxCollider::BoxCollider() {
        m_Material = std::make_shared<PhysicsMaterial>();
        m_Shape = std::make_shared<PhysicsShape>(PxBoxGeometry(m_BoxExtents.x, m_BoxExtents.y, m_BoxExtents.z), *m_Material->GetMaterial());
    }

    BoxCollider::BoxCollider(const glm::vec3& boxCentre, const glm::vec3& boxExtents) : m_Centre(boxCentre), m_BoxExtents(glm::abs(boxExtents)) {
        m_Material = std::make_shared<PhysicsMaterial>();
        m_Shape = std::make_shared<PhysicsShape>(PxBoxGeometry(m_BoxExtents.x, m_BoxExtents.y, m_BoxExtents.z), *m_Material->GetMaterial());
    }

    BoxCollider::~BoxCollider() {

    }

    void BoxCollider::Release() {

        if (m_Shape) {
            m_Shape->Release();
            m_Shape = nullptr;
        }

        if (m_Material)
            m_Material = nullptr;

    }

    // GETTERS
    const bool& BoxCollider::GetIsTrigger() const { return m_IsTrigger; }
    std::shared_ptr<PhysicsMaterial> BoxCollider::GetMaterial() const { return m_Material; }

    const glm::vec3& BoxCollider::GetCentre() const { return m_Centre; }
    const glm::vec3& BoxCollider::GetSize() const { return m_BoxExtents; }

    std::shared_ptr<PhysicsShape> BoxCollider::GetShape() const { return m_Shape; }

    // SETTERS
    void BoxCollider::SetIsTrigger(bool isTrigger) {
        m_IsTrigger = isTrigger;
        if (m_Shape) {
            m_Shape->SetFlag(PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
            m_Shape->SetFlag(PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
        }
    }

    void BoxCollider::SetMaterial(std::shared_ptr<PhysicsMaterial> material) {
        if (material) {
            m_Material = material;
            if (m_Shape) {
                m_Shape->SetMaterials((PxMaterial* const*)m_Material->GetMaterial(), 1);
            }
        }
        else {
            L_CORE_ERROR("Invalid Material!");
        }
    }

    void BoxCollider::SetCentre(const glm::vec3& centre) {
        m_Centre = centre;

        m_Shape->AddFlag(ColliderFlag_ShapePropsUpdated);
    }

    void BoxCollider::SetSize(const glm::vec3& boxExtents) {
        m_BoxExtents = glm::abs(boxExtents);

        m_Shape->AddFlag(ColliderFlag_ShapePropsUpdated);
    }


    void BoxCollider::UpdateTransform(Transform& collider_transform, Transform& rigidbody_transform) {

        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);

        glm::mat4 collider_matrix = collider_transform.GetTransform();
        glm::mat4 rigidbody_matrix = rigidbody_transform.GetTransform();

        if (m_Shape->IsStatic() || collider_matrix == rigidbody_matrix) {

            // If the collider does not refer to a Rigidbody Component, we simply
            // apply the custom collider centre, and multiply the transforms
            // largest absolute scale value by the m_Radius modifier.
            position = m_Centre; // <- this is the offset of the shape to the rigidbody's origin
            scale = collider_transform.GetScale();

            m_Shape->SetLocalPose(PxTransform(position.x, position.y, position.z));

            glm::vec3 size = glm::abs(scale) * m_BoxExtents;
            m_Shape->SetGeometry(PxBoxGeometry(size.x, size.y, size.z));

            if (auto rb_ref = m_Shape->GetRigidbody(); rb_ref && *rb_ref) {

                rb_ref->SetGlobalPose(collider_transform);
                rb_ref->AddFlag(RigidbodyFlag_ShapesUpdated);
            }
        }
        else {

            // If the collider and rigidbody transform is not the same, we find 
            // the local pose of the collider relative to the rigidbody, then 
            // add the m_Centre offset to the local position, and multiply the
            // largest absolute scale value by the m_Radius modifier.
            collider_matrix = glm::inverse(rigidbody_matrix) * collider_matrix;

            position = glm::vec3(collider_matrix[3]) + m_Centre;
            scale = glm::vec3(glm::length(collider_matrix[0]), glm::length(collider_matrix[1]), glm::length(collider_matrix[2]));
            glm::quat quaternion = glm::quat_cast(collider_matrix);

            m_Shape->SetLocalPose(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

            glm::vec3 size = glm::abs(scale) * m_BoxExtents;
            m_Shape->SetGeometry(PxBoxGeometry(size.x, size.y, size.z));

            if (auto rb_ref = m_Shape->GetRigidbody(); rb_ref && *rb_ref) {

                rb_ref->AddFlag(RigidbodyFlag_ShapesUpdated);
            }
        }
    }

#pragma endregion

}