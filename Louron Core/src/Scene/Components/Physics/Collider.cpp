#include "Collider.h"

// Louron Core Headers
#include "../Mesh.h"
#include "../Components.h"

#include "Rigidbody.h"
#include "PhysicsWrappers.h"

#include "../../Entity.h"

#include "../../../Core/Logging.h"
#include "../../../Project/Project.h"

// C++ Standard Library Headers

// External Vendor Library Headers

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron {

#pragma region SphereColliderComponent


    void SphereColliderComponent::Init()
    {
        GetMaterial()->Init();

        m_Shape = std::make_shared<PhysicsShape>(PxSphereGeometry(m_Radius * 2.0f), *m_Material->GetMaterial());

        AddFlag(ColliderFlag_TransformUpdated);
        AddFlag(ColliderFlag_RigidbodyUpdated);
        AddFlag(ColliderFlag_ShapePropsUpdated);

        if(auto entity = GetEntity(); entity)
            SetColliderUserData(entity.GetUUID());

        SetIsTrigger(m_IsTrigger);
        SetCentre(m_Centre);
        SetRadius(m_Radius);
    }

    void SphereColliderComponent::Shutdown()
    {
        if(m_Material)
            m_Material->Shutdown();
        Release();
    }

    SphereColliderComponent::SphereColliderComponent() {
        m_Material = std::make_shared<PhysicsMaterial>();
    }

    SphereColliderComponent::SphereColliderComponent(glm::vec3 SphereCentre, float SphereRadius) : m_Centre(SphereCentre), m_Radius(SphereRadius) {
        m_Material = std::make_shared<PhysicsMaterial>();
    }

    SphereColliderComponent::SphereColliderComponent(const SphereColliderComponent& other) {

        scene = other.scene;

        m_Radius = other.m_Radius;
        m_IsTrigger = other.m_IsTrigger;
        m_Centre = other.m_Centre;

        m_StateFlags = other.m_StateFlags;

        Entity entity = GetEntity();

        m_EntityUUID = entity ? entity.GetUUID() : (UUID)NULL_UUID;
        m_RigidbodyUUID = NULL_UUID;

        if (other.m_Material)
            m_Material = std::make_shared<PhysicsMaterial>(*other.m_Material);

        if (scene->IsRunning() || scene->IsSimulating())
            Init();
    }

    SphereColliderComponent::SphereColliderComponent(SphereColliderComponent&& other) noexcept {

        // Component Base Class Move
        entity_uuid = other.entity_uuid; other.entity_uuid = NULL_UUID;
        scene = other.scene; other.scene = nullptr;

        // Sphere Collider Class Move
        m_Radius = other.m_Radius; other.m_Radius = 0.5f;
        m_IsTrigger = other.m_IsTrigger; other.m_IsTrigger = false;
        m_Centre = other.m_Centre; other.m_Centre = { 0.0f, 0.0f, 0.0f };

        m_RigidbodyUUID = other.m_RigidbodyUUID; other.m_RigidbodyUUID = NULL_UUID;
        m_EntityUUID = other.m_EntityUUID; other.m_EntityUUID = NULL_UUID;

        m_StateFlags = other.m_StateFlags; other.m_StateFlags = ColliderFlag_None;

        m_Shape = other.m_Shape; other.m_Shape = nullptr;
        m_Material = other.m_Material; other.m_Material = nullptr;
    }

    SphereColliderComponent::~SphereColliderComponent() {
        if (m_Material)
            m_Material->Shutdown();
    }

    // COPY ASSIGNMENT OPERATOR
    SphereColliderComponent& SphereColliderComponent::operator=(const SphereColliderComponent& other) {

        if (this == &other || !scene)
            return *this;

        this->Shutdown();

        m_Radius = other.m_Radius;
        m_IsTrigger = other.m_IsTrigger;
        m_Centre = other.m_Centre;

        m_StateFlags = other.m_StateFlags;

        Entity entity = GetEntity();

        m_EntityUUID = entity ? entity.GetUUID() : (UUID)NULL_UUID;
        m_RigidbodyUUID = NULL_UUID;

        if(other.m_Material)
            m_Material = std::make_shared<PhysicsMaterial>(*other.m_Material);

        if (scene->IsRunning() || scene->IsSimulating())
            Init();
        
        return *this;
    }

    // MOVE ASSIGNMENT OPERATOR
    SphereColliderComponent& SphereColliderComponent::operator=(SphereColliderComponent&& other) noexcept
    {
        if (this == &other) // Guard against self-assignment
            return *this;

        this->Shutdown();

        // Component Base Class Move
        entity_uuid = other.entity_uuid; other.entity_uuid = NULL_UUID;
        scene = other.scene; other.scene = nullptr;

        // Sphere Collider Class Move
        m_Radius = other.m_Radius; other.m_Radius = 0.5f;
        m_IsTrigger = other.m_IsTrigger; other.m_IsTrigger = false;
        m_Centre = other.m_Centre; other.m_Centre = { 0.0f, 0.0f, 0.0f };

        m_RigidbodyUUID = other.m_RigidbodyUUID; other.m_RigidbodyUUID = NULL_UUID;
        m_EntityUUID = other.m_EntityUUID; other.m_EntityUUID = NULL_UUID;

        m_StateFlags = other.m_StateFlags; other.m_StateFlags = ColliderFlag_None;

        m_Shape = other.m_Shape; other.m_Shape = nullptr;
        m_Material = other.m_Material; other.m_Material = nullptr;
        
        return *this;
    }

    void SphereColliderComponent::SetColliderUserData(const UUID& uuid) {
        m_EntityUUID = uuid;
        if (m_Shape && m_Shape->GetShape()) {
            m_Shape->GetShape()->userData = reinterpret_cast<void*>(static_cast<uintptr_t>((uint32_t)m_EntityUUID));
        }
    }

    void SphereColliderComponent::Release() {

        if(m_Shape) {
            m_Shape->Release();
            m_Shape = nullptr;
        }

        if(m_Material)
            m_Material->Shutdown();

    }

    void SphereColliderComponent::CreateStaticRigidbody() {

        Entity entity = GetEntity();
        if (!entity || !entity.GetScene()) {
            L_CORE_ERROR("Cannot Create Static Rigidbody - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        ResetRigidbody();

        const glm::vec3& position = entity.GetComponent<TransformComponent>().GetGlobalPosition();
        const glm::quat& quaternion = glm::quat(glm::radians(entity.GetComponent<TransformComponent>().GetGlobalRotation()));
        
        m_Shape->m_StaticBody = std::make_shared<RigidDynamic>();
        m_Shape->m_StaticBody->Init(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

        m_Shape->m_StaticBody->GetActor()->attachShape(*m_Shape->m_Shape);

        m_Shape->m_StaticBody->SetKinematic(true);

        PxRigidBodyExt::updateMassAndInertia(*m_Shape->m_StaticBody->GetActor(), 1.0f);
        entity.GetScene()->GetPhysScene()->addActor(*m_Shape->m_StaticBody->GetActor());

        AddFlag(ColliderFlag_TransformUpdated);
        AddFlag(ColliderFlag_ShapePropsUpdated);

        m_Shape->m_RigidbodyRef = m_Shape->m_StaticBody;
        m_RigidbodyUUID = entity.GetUUID();

    }

    /// <summary>
    /// Updates the weak_ptr reference to the RigidDynamic, and the Rigidbody entity UUID.
    /// </summary>
    void SphereColliderComponent::UpdateRigidbody(const UUID& rigidbodyEntityUUID) {

        Entity entity = GetEntity();
        if (!entity || !entity.GetScene()) {
            L_CORE_ERROR("Cannot Update Rigidbody - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        if (!entity.GetScene()->FindEntityByUUID(rigidbodyEntityUUID)) {
            
            L_CORE_WARN("Cannot Update Rigidbody - New Rigidbody Entity Is Invalid and Cannot Access Scene!");
            CreateStaticRigidbody();
            return;
        }
        
        if (!entity.GetScene()->FindEntityByUUID(rigidbodyEntityUUID).HasComponent<RigidbodyComponent>()) {
            
            L_CORE_WARN("Cannot Update Rigidbody - New Rigidbody Entity Does Not Have Rigidbody Component!");
            CreateStaticRigidbody();
            return;
        }

        if (auto rb_ref = entity.GetScene()->FindEntityByUUID(rigidbodyEntityUUID).GetComponent<RigidbodyComponent>().GetActor();  rb_ref && *rb_ref) {

            ResetRigidbody();

            AddFlag(ColliderFlag_RigidbodyUpdated);
            AddFlag(ColliderFlag_TransformUpdated);

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
    void SphereColliderComponent::ResetRigidbody() {

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

        Entity entity = GetEntity();
        SetRadius(glm::compMax(entity.GetComponent<TransformComponent>().GetGlobalScale()) / 2.0f);
    }

    // GETTERS
    const bool& SphereColliderComponent::IsTrigger() const { return m_IsTrigger; }
    std::shared_ptr<PhysicsMaterial> SphereColliderComponent::GetMaterial() const { return m_Material; }

    const glm::vec3& SphereColliderComponent::GetCentre() const { return m_Centre; }
    const float& SphereColliderComponent::GetRadius() const { return m_Radius; }

    std::shared_ptr<PhysicsShape> SphereColliderComponent::GetShape() const { return m_Shape; }

    // SETTERS
    void SphereColliderComponent::SetIsTrigger(bool isTrigger) {
        m_IsTrigger = isTrigger;
        if (m_Shape) {
            m_Shape->SetFlag(PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
            m_Shape->SetFlag(PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
        }
    }

    void SphereColliderComponent::SetMaterial(std::shared_ptr<PhysicsMaterial> material) {
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

    void SphereColliderComponent::SetCentre(const glm::vec3& centre) {
        m_Centre = centre;

        AddFlag(ColliderFlag_ShapePropsUpdated);
    }

    void SphereColliderComponent::SetRadius(float radius) {
        m_Radius = radius;

        AddFlag(ColliderFlag_ShapePropsUpdated);
    }

    void SphereColliderComponent::Serialize(YAML::Emitter& out)
    {
        out << YAML::Key << "SphereColliderComponent";
        out << YAML::BeginMap;


        out << YAML::Key << "IsTrigger" << YAML::Value << m_IsTrigger;

        out << YAML::Key << "Radius" << YAML::Value << m_Radius;

        out << YAML::Key << "Centre" << YAML::Value << YAML::Flow
            << YAML::BeginSeq
            << m_Centre.x
            << m_Centre.y
            << m_Centre.z
            << YAML::EndSeq;

        out << YAML::Key << "MaterialProperties" << YAML::Value << YAML::Flow
            << YAML::BeginSeq
            << m_Material->GetStaticFriction()
            << m_Material->GetDynamicFriction()
            << m_Material->GetBounciness()
            << YAML::EndSeq;

        out << YAML::EndMap;
    }

    bool SphereColliderComponent::Deserialize(const YAML::Node data)
    {
        // Deserialize the IsTrigger value
        if (data["IsTrigger"]) {
            SetIsTrigger(data["IsTrigger"].as<bool>());
        }

        // Deserialize the Radius value
        if (data["Radius"]) {
            SetRadius(data["Radius"].as<float>());
        }

        // Deserialize the Centre sequence
        if (data["Centre"] && data["Centre"].IsSequence()) {
            const YAML::Node& centreNode = data["Centre"];

            glm::vec3 temp{};
            temp.x = centreNode[0].as<float>();
            temp.y = centreNode[1].as<float>();
            temp.z = centreNode[2].as<float>();

            SetCentre(temp);
        }

        // Deserialize the MaterialProperties sequence
        if (data["MaterialProperties"] && data["MaterialProperties"].IsSequence()) {
            const YAML::Node& materialNode = data["MaterialProperties"];

            float staticFriction = materialNode[0].as<float>();
            float dynamicFriction = materialNode[1].as<float>();
            float bounciness = materialNode[2].as<float>();

            // Assuming you have a method to set material properties
            m_Material->SetStaticFriction(staticFriction);
            m_Material->SetDynamicFriction(dynamicFriction);
            m_Material->SetBounciness(bounciness);
        }

        return true;
    }


    void SphereColliderComponent::UpdateTransform(TransformComponent& collider_transform, TransformComponent& rigidbody_transform) {

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

    #pragma region Flagging System

        void SphereColliderComponent::AddFlag(ColliderFlags flag) { m_StateFlags = static_cast<ColliderFlags>(m_StateFlags | flag); }
        void SphereColliderComponent::ClearFlag(ColliderFlags flag) { m_StateFlags = static_cast<ColliderFlags>(m_StateFlags & ~flag); }
        bool SphereColliderComponent::CheckFlag(ColliderFlags flag) const { return (m_StateFlags & static_cast<ColliderFlags>(flag)) != ColliderFlag_None; }
        bool SphereColliderComponent::NoFlagsSet() const { return m_StateFlags == ColliderFlag_None; }
        void SphereColliderComponent::ClearFlags() { m_StateFlags = ColliderFlag_None; }
        ColliderFlags SphereColliderComponent::GetFlags() const { return m_StateFlags; }

    #pragma endregion

    #pragma endregion

#pragma region BoxColliderComponent

    void BoxColliderComponent::Init() {

        GetMaterial()->Init();

        m_Shape = std::make_shared<PhysicsShape>(PxBoxGeometry(m_BoxExtents.x, m_BoxExtents.y, m_BoxExtents.z), *m_Material->GetMaterial());

        AddFlag(ColliderFlag_TransformUpdated);
        AddFlag(ColliderFlag_RigidbodyUpdated);
        AddFlag(ColliderFlag_ShapePropsUpdated);

        if (auto entity = GetEntity(); entity)
            SetColliderUserData(entity.GetUUID());

        SetIsTrigger(m_IsTrigger);
        SetCentre(m_Centre);
        SetSize(m_BoxExtents);
    }

    void BoxColliderComponent::Shutdown() {
        if(m_Material)
            m_Material->Shutdown();
        Release();
    }

    BoxColliderComponent::BoxColliderComponent() {
        m_Material = std::make_shared<PhysicsMaterial>();
    }

    BoxColliderComponent::BoxColliderComponent(const glm::vec3& boxCentre, const glm::vec3& boxExtents) : m_Centre(boxCentre), m_BoxExtents(glm::abs(boxExtents)) { 
        m_Material = std::make_shared<PhysicsMaterial>();
    }

    // COPY CONSTRUCTOR
    // If you make a copy of this during runtime, you need to ensure 
    // that the correct entity_uuid and scene are set afterward! 
    BoxColliderComponent::BoxColliderComponent(const BoxColliderComponent& other) {

        entity_uuid = other.entity_uuid;
        scene = other.scene;

        m_BoxExtents = other.m_BoxExtents;
        m_IsTrigger = other.m_IsTrigger;
        m_Centre = other.m_Centre;

        m_StateFlags = other.m_StateFlags;

        if (other.m_Material)
            m_Material = std::make_shared<PhysicsMaterial>(*other.m_Material);

    }

    BoxColliderComponent::BoxColliderComponent(BoxColliderComponent&& other) noexcept
    {
        // Component Base Class Move
        scene = other.scene; other.scene = nullptr;
        entity_uuid = other.entity_uuid; other.entity_uuid = NULL_UUID;

        // Sphere Collider Class Move
        m_BoxExtents = other.m_BoxExtents; other.m_BoxExtents = { 1.0f, 1.0f, 1.0f };
        m_IsTrigger = other.m_IsTrigger; other.m_IsTrigger = false;
        m_Centre = other.m_Centre; other.m_Centre = { 0.0f, 0.0f, 0.0f };

        m_RigidbodyUUID = other.m_RigidbodyUUID; other.m_RigidbodyUUID = NULL_UUID;
        m_EntityUUID = other.m_EntityUUID; other.m_EntityUUID = NULL_UUID;

        m_StateFlags = other.m_StateFlags; other.m_StateFlags = ColliderFlag_None;

        m_Shape = other.m_Shape; other.m_Shape = nullptr;
        m_Material = other.m_Material; other.m_Material = nullptr;

    }

    BoxColliderComponent::~BoxColliderComponent() {
        if (m_Material)
            m_Material = nullptr;
    }

    // COPY ASSIGNMENT OPERATOR
    BoxColliderComponent& BoxColliderComponent::operator=(const BoxColliderComponent& other) {

        if (this == &other || !scene)
            return *this;

        this->Shutdown();

        m_BoxExtents = other.m_BoxExtents;
        m_IsTrigger = other.m_IsTrigger;
        m_Centre = other.m_Centre;

        m_StateFlags = other.m_StateFlags;

        Entity entity = GetEntity();

        m_EntityUUID = entity ? entity.GetUUID() : (UUID)NULL_UUID;
        m_RigidbodyUUID = NULL_UUID;

        if (other.m_Material)
            m_Material = std::make_shared<PhysicsMaterial>(*other.m_Material);

        if (scene->IsRunning() || scene->IsSimulating())
            Init();

        return *this;
    }

    BoxColliderComponent& BoxColliderComponent::operator=(BoxColliderComponent&& other) noexcept
    {
        if (this == &other) // Guard against self-assignment
            return *this;

        this->Shutdown();

        // Component Base Class Move
        entity_uuid = other.entity_uuid; other.entity_uuid = NULL_UUID;
        scene = other.scene; other.scene = nullptr;

        // Sphere Collider Class Move
        m_BoxExtents = other.m_BoxExtents; other.m_BoxExtents = { 1.0f, 1.0f, 1.0f };
        m_IsTrigger = other.m_IsTrigger; other.m_IsTrigger = false;
        m_Centre = other.m_Centre; other.m_Centre = { 0.0f, 0.0f, 0.0f };

        m_RigidbodyUUID = other.m_RigidbodyUUID; other.m_RigidbodyUUID = NULL_UUID;
        m_EntityUUID = other.m_EntityUUID; other.m_EntityUUID = NULL_UUID;

        m_StateFlags = other.m_StateFlags; other.m_StateFlags = ColliderFlag_None;


        m_Shape = other.m_Shape; other.m_Shape = nullptr;
        m_Material = other.m_Material; other.m_Material = nullptr;

        return *this;
    }

    void BoxColliderComponent::SetColliderUserData(const UUID& uuid) {
        m_EntityUUID = uuid;
        if (m_Shape && m_Shape->GetShape()) {
            m_Shape->GetShape()->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(m_EntityUUID));
        }
    }

    void BoxColliderComponent::Release() {

        if (m_Shape) {
            m_Shape->Release();
            m_Shape = nullptr;
        }
    }

    void BoxColliderComponent::CreateStaticRigidbody() {

        Entity entity = GetEntity();
        if (!entity || !entity.GetScene()) {
            L_CORE_ERROR("Cannot Create Static Rigidbody - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        this->ResetRigidbody();

        const glm::vec3& position = entity.GetComponent<TransformComponent>().GetGlobalPosition();
        const glm::quat& quaternion = glm::quat(glm::radians(entity.GetComponent<TransformComponent>().GetGlobalRotation()));

        m_Shape->m_StaticBody = std::make_shared<RigidDynamic>();
        m_Shape->m_StaticBody->Init(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

        m_Shape->m_StaticBody->GetActor()->attachShape(*m_Shape->m_Shape);

        m_Shape->m_StaticBody->SetKinematic(true);

        PxRigidBodyExt::updateMassAndInertia(*m_Shape->m_StaticBody->GetActor(), 1.0f);
        entity.GetScene()->GetPhysScene()->addActor(*m_Shape->m_StaticBody->GetActor());

        AddFlag(ColliderFlag_TransformUpdated);
        AddFlag(ColliderFlag_ShapePropsUpdated);

        m_Shape->m_RigidbodyRef = m_Shape->m_StaticBody;
        m_RigidbodyUUID = entity.GetUUID();

    }

    /// <summary>
    /// Updates the weak_ptr reference to the RigidDynamic, and the Rigidbody entity UUID.
    /// </summary>
    void BoxColliderComponent::UpdateRigidbody(const UUID& rigidbodyEntityUUID) {

        Entity entity = GetEntity();
        if (!entity || !entity.GetScene()) {
            L_CORE_ERROR("Cannot Update Rigidbody - Current Entity Is Invalid and Cannot Access Scene!");
            return;
        }

        if (!entity.GetScene()->FindEntityByUUID(rigidbodyEntityUUID)) {

            L_CORE_WARN("Cannot Update Rigidbody - New Rigidbody Entity Is Invalid and Cannot Access Scene!");
            CreateStaticRigidbody();
            return;
        }

        if (!entity.GetScene()->FindEntityByUUID(rigidbodyEntityUUID).HasComponent<RigidbodyComponent>()) {

            L_CORE_WARN("Cannot Update Rigidbody - New Rigidbody Entity Does Not Have Rigidbody Component!");
            CreateStaticRigidbody();
            return;
        }

        if (auto rb_ref = entity.GetScene()->FindEntityByUUID(rigidbodyEntityUUID).GetComponent<RigidbodyComponent>().GetActor();  rb_ref && *rb_ref) {

            ResetRigidbody();

           AddFlag(ColliderFlag_RigidbodyUpdated);
           AddFlag(ColliderFlag_TransformUpdated);

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
    void BoxColliderComponent::ResetRigidbody() {

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
    const bool& BoxColliderComponent::IsTrigger() const { return m_IsTrigger; }
    std::shared_ptr<PhysicsMaterial> BoxColliderComponent::GetMaterial() const { return m_Material; }

    const glm::vec3& BoxColliderComponent::GetCentre() const { return m_Centre; }
    const glm::vec3& BoxColliderComponent::GetSize() const { return m_BoxExtents; }

    std::shared_ptr<PhysicsShape> BoxColliderComponent::GetShape() const { return m_Shape; }

    // SETTERS
    void BoxColliderComponent::SetIsTrigger(bool isTrigger) {
        m_IsTrigger = isTrigger;
        if (m_Shape) {
            m_Shape->SetFlag(PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
            m_Shape->SetFlag(PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
        }
    }

    void BoxColliderComponent::SetMaterial(std::shared_ptr<PhysicsMaterial> material) {
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

    void BoxColliderComponent::SetCentre(const glm::vec3& centre) {
        m_Centre = centre;

        AddFlag(ColliderFlag_ShapePropsUpdated);
    }

    void BoxColliderComponent::SetSize(const glm::vec3& boxExtents) {
        m_BoxExtents = glm::abs(boxExtents);

        AddFlag(ColliderFlag_ShapePropsUpdated);
    }

    void BoxColliderComponent::Serialize(YAML::Emitter& out)
    {
        out << YAML::Key << "BoxColliderComponent";
        out << YAML::BeginMap;

        out << YAML::Key << "IsTrigger" << YAML::Value << m_IsTrigger;

        out << YAML::Key << "Centre" << YAML::Value << YAML::Flow
            << YAML::BeginSeq
            << m_Centre.x
            << m_Centre.y
            << m_Centre.z
            << YAML::EndSeq;

        out << YAML::Key << "Size" << YAML::Value << YAML::Flow
            << YAML::BeginSeq
            << m_BoxExtents.x
            << m_BoxExtents.y
            << m_BoxExtents.z
            << YAML::EndSeq;

        out << YAML::Key << "MaterialProperties" << YAML::Value << YAML::Flow
            << YAML::BeginSeq
            << m_Material->GetStaticFriction()
            << m_Material->GetDynamicFriction()
            << m_Material->GetBounciness()
            << YAML::EndSeq;

        out << YAML::EndMap;
    }

    bool BoxColliderComponent::Deserialize(const YAML::Node data)
    {
        // Deserialize the IsTrigger value
        if (data["IsTrigger"]) {
            SetIsTrigger(data["IsTrigger"].as<bool>());
        }

        // Deserialize the Size sequence
        if (data["Size"]) {
            const YAML::Node& sizeNode = data["Size"];

            glm::vec3 temp{};

            temp.x = sizeNode[0].as<float>();
            temp.y = sizeNode[1].as<float>();
            temp.z = sizeNode[2].as<float>();

            SetSize(temp);
        }

        // Deserialize the Centre sequence
        if (data["Centre"] && data["Centre"].IsSequence()) {
            const YAML::Node& centreNode = data["Centre"];

            glm::vec3 temp{};
            temp.x = centreNode[0].as<float>();
            temp.y = centreNode[1].as<float>();
            temp.z = centreNode[2].as<float>();

            SetCentre(temp);
        }

        // Deserialize the MaterialProperties sequence
        if (data["MaterialProperties"] && data["MaterialProperties"].IsSequence()) {
            const YAML::Node& materialNode = data["MaterialProperties"];

            float staticFriction = materialNode[0].as<float>();
            float dynamicFriction = materialNode[1].as<float>();
            float bounciness = materialNode[2].as<float>();

            // Assuming you have a method to set material properties
            m_Material->SetStaticFriction(staticFriction);
            m_Material->SetDynamicFriction(dynamicFriction);
            m_Material->SetBounciness(bounciness);
        }

        return true;
    }


    void BoxColliderComponent::UpdateTransform(TransformComponent& collider_transform, TransformComponent& rigidbody_transform) {

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

    #pragma region Flagging System

    void BoxColliderComponent::AddFlag(ColliderFlags flag) { m_StateFlags = static_cast<ColliderFlags>(m_StateFlags | flag); }
    void BoxColliderComponent::ClearFlag(ColliderFlags flag) { m_StateFlags = static_cast<ColliderFlags>(m_StateFlags & ~flag); }
    bool BoxColliderComponent::CheckFlag(ColliderFlags flag) const { return (m_StateFlags & static_cast<ColliderFlags>(flag)) != ColliderFlag_None; }
    bool BoxColliderComponent::NoFlagsSet() const { return m_StateFlags == ColliderFlag_None; }
    void BoxColliderComponent::ClearFlags() { m_StateFlags = ColliderFlag_None; }
    ColliderFlags BoxColliderComponent::GetFlags() const { return m_StateFlags; }

    #pragma endregion

    #pragma endregion

}