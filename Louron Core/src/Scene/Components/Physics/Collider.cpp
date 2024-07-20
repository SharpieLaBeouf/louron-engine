#include "Collider.h"

// Louron Core Headers
#include "../Mesh.h"
#include "../Components.h"

#include "Rigidbody.h"
#include "PhysicsWrappers.h"

#include "../../Entity.h"

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

    SphereCollider::SphereCollider(const SphereCollider& other) {

        m_Radius = other.m_Radius;
        m_IsTrigger = other.m_IsTrigger;
        m_Centre = other.m_Centre;

        if (entity && *entity)
            m_EntityUUID = entity->GetUUID();
        else
            m_EntityUUID = NULL_UUID;

        m_RigidbodyUUID = NULL_UUID;

        if(other.m_Material)
            m_Material = std::make_shared<PhysicsMaterial>(*other.m_Material);
        else
            m_Material = m_Material = std::make_shared<PhysicsMaterial>();

        m_Shape = std::make_shared<PhysicsShape>(PxSphereGeometry(m_Radius * 2.0f), *m_Material->GetMaterial());
    }

    SphereCollider::~SphereCollider() {

    }

    SphereCollider& SphereCollider::operator=(const SphereCollider& other) {

        m_Radius = other.m_Radius;
        m_IsTrigger = other.m_IsTrigger;
        m_Centre = other.m_Centre;

        if (entity && *entity)
            m_EntityUUID = entity->GetUUID();
        else
            m_EntityUUID = NULL_UUID;

        m_RigidbodyUUID = NULL_UUID;

        if (other.m_Material)
            m_Material = std::make_shared<PhysicsMaterial>(*other.m_Material);
        else
            m_Material = m_Material = std::make_shared<PhysicsMaterial>();

        m_Shape = std::make_shared<PhysicsShape>(PxSphereGeometry(m_Radius * 2.0f), *m_Material->GetMaterial());

        SetColliderUserData(m_EntityUUID);

        m_Shape->AddFlag(ColliderFlag_TransformUpdated);
        m_Shape->AddFlag(ColliderFlag_RigidbodyUpdated);
        m_Shape->AddFlag(ColliderFlag_ShapePropsUpdated);

        return *this;
    }

    void SphereCollider::SetColliderUserData(const UUID& uuid) {
        m_EntityUUID = uuid;
        if (m_Shape) {
            m_Shape->GetShape()->userData = reinterpret_cast<void*>(static_cast<uintptr_t>((uint32_t)m_EntityUUID));
        }
    }

    void SphereCollider::Release() {

        if(m_Shape) {
            m_Shape->Release();
            m_Shape = nullptr;
        }

        if(m_Material)
            m_Material = nullptr;

    }

    void SphereCollider::CreateStaticRigidbody() {

        if (!entity || !*entity || !entity->GetScene()) {
            L_CORE_ERROR("Cannot Create Static Rigidbody - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        ResetRigidbody();

        const glm::vec3& position = entity->GetComponent<Transform>().GetGlobalPosition();
        const glm::quat& quaternion = glm::quat(glm::radians(entity->GetComponent<Transform>().GetGlobalRotation()));
        
        m_Shape->m_StaticBody = std::make_shared<RigidDynamic>(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

        m_Shape->m_StaticBody->GetActor()->attachShape(*m_Shape->m_Shape);

        m_Shape->m_StaticBody->SetKinematic(true);

        PxRigidBodyExt::updateMassAndInertia(*m_Shape->m_StaticBody->GetActor(), 1.0f);
        entity->GetScene()->GetPhysScene()->addActor(*m_Shape->m_StaticBody->GetActor());

        m_Shape->AddFlag(ColliderFlag_TransformUpdated);
        m_Shape->AddFlag(ColliderFlag_ShapePropsUpdated);

        m_Shape->m_RigidbodyRef = m_Shape->m_StaticBody;
        m_RigidbodyUUID = entity->GetUUID();

    }

    /// <summary>
    /// Updates the weak_ptr reference to the RigidDynamic, and the Rigidbody entity UUID.
    /// </summary>
    void SphereCollider::UpdateRigidbody(const UUID& rigidbodyEntityUUID) {

        if (!entity || !*entity || !entity->GetScene()) {
            L_CORE_ERROR("Cannot Update Rigidbody - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        if (!entity->GetScene()->FindEntityByUUID(rigidbodyEntityUUID)) {
            
            L_CORE_WARN("Cannot Update Rigidbody - New Rigidbody Entity Is Invalid and Cannot Access Scene!");
            CreateStaticRigidbody();
            return;
        }
        
        if (!entity->GetScene()->FindEntityByUUID(rigidbodyEntityUUID).HasComponent<Rigidbody>()) {
            
            L_CORE_WARN("Cannot Update Rigidbody - New Rigidbody Entity Does Not Have Rigidbody Component!");
            CreateStaticRigidbody();
            return;
        }

        if (auto rb_ref = entity->GetScene()->FindEntityByUUID(rigidbodyEntityUUID).GetComponent<Rigidbody>().GetActor();  rb_ref && *rb_ref) {

            ResetRigidbody();

            m_Shape->AddFlag(ColliderFlag_RigidbodyUpdated);
            m_Shape->AddFlag(ColliderFlag_TransformUpdated);

            m_Shape->m_RigidbodyRef = rb_ref;
            m_RigidbodyUUID = rigidbodyEntityUUID;

            L_CORE_INFO("Successfully Updated Rigidbody Reference in Physics Shape.");
        }
        else {
            L_CORE_WARN("New Rigidbody Reference Invalid - Creating Static Rigidbody for Collider.");
            CreateStaticRigidbody();
        }

        return;
    }

    /// <summary>
    /// Reset the weak_ptr to the RigidDynamic and the Rigidbody entity UUID.
    /// </summary>
    void SphereCollider::ResetRigidbody() {

        if (auto rb_ref = m_Shape->m_RigidbodyRef.lock(); rb_ref && *rb_ref)
            rb_ref->DetachShape(m_Shape);

        m_Shape->m_RigidbodyRef.reset();
        m_RigidbodyUUID = NULL_UUID;

        if (m_Shape->m_StaticBody && *m_Shape->m_StaticBody) {
            m_Shape->m_StaticBody->GetActor()->detachShape(*m_Shape->m_Shape);
            m_Shape->m_StaticBody->Release();
            m_Shape->m_StaticBody = nullptr;
        }

        m_Shape->SetLocalPose({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
        SetRadius(glm::compMax(entity->GetComponent<Transform>().GetGlobalScale()) / 2.0f);
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

        glm::mat4 collider_matrix = collider_transform.GetGlobalTransform();
        glm::mat4 rigidbody_matrix = rigidbody_transform.GetGlobalTransform();

        if (m_Shape->IsStatic() || collider_matrix == rigidbody_matrix) {

            // If the collider does not refer to a Rigidbody Component, we simply
            // apply the custom collider centre, and multiply the transforms
            // largest absolute scale value by the m_Radius modifier.
            position = m_Centre;
            scale = collider_transform.GetGlobalScale();

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
            m_Shape->SetGeometry(PxSphereGeometry(glm::compMax(glm::abs(collider_transform.GetGlobalScale())) * (m_Radius * 2.0f)));

            collider_matrix = glm::inverse(rigidbody_matrix) * collider_matrix;

            scale = glm::vec3(glm::length(collider_matrix[0]), glm::length(collider_matrix[1]), glm::length(collider_matrix[2]));
            position = glm::vec3(collider_matrix[3]) * glm::abs(collider_transform.GetGlobalScale()) + m_Centre;
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

    BoxCollider::BoxCollider(const BoxCollider& other) {

        m_BoxExtents = other.m_BoxExtents;
        m_IsTrigger = other.m_IsTrigger;
        m_Centre = other.m_Centre;

        if (entity && *entity)
            m_EntityUUID = entity->GetUUID();
        else
            m_EntityUUID = NULL_UUID;

        m_RigidbodyUUID = NULL_UUID;

        m_Material = std::make_shared<PhysicsMaterial>(*other.m_Material);
        m_Shape = std::make_shared<PhysicsShape>(PxBoxGeometry(m_BoxExtents.x, m_BoxExtents.y, m_BoxExtents.z), *m_Material->GetMaterial());
    }

    BoxCollider::~BoxCollider() {

    }

    BoxCollider& BoxCollider::operator=(const BoxCollider& other) {

        m_BoxExtents = other.m_BoxExtents;
        m_IsTrigger = other.m_IsTrigger;
        m_Centre = other.m_Centre;

        if (entity && *entity)
            m_EntityUUID = entity->GetUUID();
        else
            m_EntityUUID = NULL_UUID;

        m_RigidbodyUUID = NULL_UUID;

        m_Material = std::make_shared<PhysicsMaterial>(*other.m_Material);
        m_Shape = std::make_shared<PhysicsShape>(PxBoxGeometry(m_BoxExtents.x, m_BoxExtents.y, m_BoxExtents.z), *m_Material->GetMaterial());

        SetColliderUserData(m_EntityUUID);

        m_Shape->AddFlag(ColliderFlag_TransformUpdated);
        m_Shape->AddFlag(ColliderFlag_RigidbodyUpdated);
        m_Shape->AddFlag(ColliderFlag_ShapePropsUpdated);

        return *this;
    }

    void BoxCollider::SetColliderUserData(const UUID& uuid) {
        m_EntityUUID = uuid;
        if (m_Shape) {
            m_Shape->GetShape()->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(m_EntityUUID));
        }
    }

    void BoxCollider::Release() {

        if (m_Shape) {
            m_Shape->Release();
            m_Shape = nullptr;
        }

        if (m_Material)
            m_Material = nullptr;

    }

    void BoxCollider::CreateStaticRigidbody() {

        if (!entity || !*entity || !entity->GetScene()) {
            L_CORE_ERROR("Cannot Create Static Rigidbody - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        this->ResetRigidbody();

        const glm::vec3& position = entity->GetComponent<Transform>().GetGlobalPosition();
        const glm::quat& quaternion = glm::quat(glm::radians(entity->GetComponent<Transform>().GetGlobalRotation()));

        m_Shape->m_StaticBody = std::make_shared<RigidDynamic>(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

        m_Shape->m_StaticBody->GetActor()->attachShape(*m_Shape->m_Shape);

        m_Shape->m_StaticBody->SetKinematic(true);

        PxRigidBodyExt::updateMassAndInertia(*m_Shape->m_StaticBody->GetActor(), 1.0f);
        entity->GetScene()->GetPhysScene()->addActor(*m_Shape->m_StaticBody->GetActor());

        m_Shape->AddFlag(ColliderFlag_TransformUpdated);
        m_Shape->AddFlag(ColliderFlag_ShapePropsUpdated);

        m_Shape->m_RigidbodyRef = m_Shape->m_StaticBody;
        m_RigidbodyUUID = entity->GetUUID();

    }

    /// <summary>
    /// Updates the weak_ptr reference to the RigidDynamic, and the Rigidbody entity UUID.
    /// </summary>
    void BoxCollider::UpdateRigidbody(const UUID& rigidbodyEntityUUID) {

        if (!entity || !*entity || !entity->GetScene()) {
            L_CORE_ERROR("Cannot Update Rigidbody - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        if (!entity->GetScene()->FindEntityByUUID(rigidbodyEntityUUID)) {

            L_CORE_WARN("Cannot Update Rigidbody - New Rigidbody Entity Is Invalid and Cannot Access Scene!");
            CreateStaticRigidbody();
            return;
        }

        if (!entity->GetScene()->FindEntityByUUID(rigidbodyEntityUUID).HasComponent<Rigidbody>()) {

            L_CORE_WARN("Cannot Update Rigidbody - New Rigidbody Entity Does Not Have Rigidbody Component!");
            CreateStaticRigidbody();
            return;
        }

        if (auto rb_ref = entity->GetScene()->FindEntityByUUID(rigidbodyEntityUUID).GetComponent<Rigidbody>().GetActor();  rb_ref && *rb_ref) {

            ResetRigidbody();

            m_Shape->AddFlag(ColliderFlag_RigidbodyUpdated);
            m_Shape->AddFlag(ColliderFlag_TransformUpdated);

            m_Shape->m_RigidbodyRef = rb_ref;
            m_RigidbodyUUID = rigidbodyEntityUUID;

            L_CORE_INFO("Successfully Updated Rigidbody Reference in Physics Shape.");
        }
        else {
            L_CORE_WARN("New Rigidbody Reference Invalid - Creating Static Rigidbody for Collider.");
            CreateStaticRigidbody();
        }

        return;
    }

    /// <summary>
    /// Reset the weak_ptr to the RigidDynamic and the Rigidbody entity UUID.
    /// </summary>
    void BoxCollider::ResetRigidbody() {

        if (auto rb_ref = m_Shape->m_RigidbodyRef.lock(); rb_ref && *rb_ref)
            rb_ref->DetachShape(m_Shape);

        m_Shape->m_RigidbodyRef.reset();
        m_RigidbodyUUID = NULL_UUID;

        if (m_Shape->m_StaticBody && *m_Shape->m_StaticBody) {
            m_Shape->m_StaticBody->GetActor()->detachShape(*m_Shape->m_Shape);
            m_Shape->m_StaticBody->Release();
            m_Shape->m_StaticBody = nullptr;
        }
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

        glm::mat4 collider_matrix = collider_transform.GetGlobalTransform();
        glm::mat4 rigidbody_matrix = rigidbody_transform.GetGlobalTransform();

        if (m_Shape->IsStatic() || collider_matrix == rigidbody_matrix) {

            // If the collider does not refer to a Rigidbody Component, we simply
            // apply the custom collider centre, and multiply the transforms
            // largest absolute scale value by the m_Radius modifier.
            position = m_Centre; // <- this is the offset of the shape to the rigidbody's origin
            scale = collider_transform.GetGlobalScale();

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