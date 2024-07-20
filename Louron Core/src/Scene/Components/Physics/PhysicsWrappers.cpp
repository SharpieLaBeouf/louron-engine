#include "PhysicsWrappers.h"

// Louron Core Headers
#include "../../../Core/Logging.h"
#include "../Components.h"
#include "../UUID.h"

// C++ Standard Library Headers


// External Vendor Library Headers
#include <glm/gtx/quaternion.hpp>

namespace Louron {

#pragma region RigidDynamic

    #pragma region Constructors and Class Setup

    RigidDynamic::RigidDynamic(const PxTransform& transform) : 
        m_Actor(PxGetPhysics().createRigidDynamic(transform))
    {
        if (!m_Actor)
            L_CORE_ERROR("Failed to Create PxRigidDynamic.");
    }

    RigidDynamic::RigidDynamic(RigidDynamic&& other) noexcept : 
        m_Actor(other.m_Actor)
    {
        other.m_Actor = nullptr;
    }

    RigidDynamic::~RigidDynamic() {
        //Release();
    }

    RigidDynamic& RigidDynamic::operator=(RigidDynamic&& other) noexcept {
        if (this != &other) {
            Release();
            m_Actor = other.m_Actor;
            other.m_Actor = nullptr;
        }
        return *this;
    }

    RigidDynamic::operator bool() const {
        return m_Actor;
    }

    #pragma endregion

    #pragma region Class Logic

    PxRigidDynamic* RigidDynamic::GetActor() const { return m_Actor; }

    void RigidDynamic::Release() {
        if (m_Actor) {
            m_Actor->release();
            m_Actor = nullptr;
        }
    }

    bool RigidDynamic::AttachShape(std::shared_ptr<PhysicsShape> shape, const UUID& shapeEntityUUID) {

        if(shape && *shape) {
            if (!IsShapeAttached(shape)) {
                if (m_Actor) {

                    m_ShapesRef[shape] = shapeEntityUUID;
                    m_Actor->attachShape(*shape->GetShape());
                    AddFlag(RigidbodyFlag_ShapesUpdated);
                    L_CORE_INFO("Shape Attached to Rigidbody Successfully.");
                    return true;

                } else  L_CORE_ERROR("Attempted to Attach Shape - Actor is Invalid.");
            } else      L_CORE_WARN ("Attempted to Attach Shape - Shape Already Attached.");
        } else          L_CORE_ERROR("Attempted to Attach Shape - Shape is Invalid.");
        return false;
    }

    void RigidDynamic::DetachShape(std::shared_ptr<PhysicsShape> shape) {

        if(shape) {
            if(IsShapeAttached(shape)) {
                if (m_Actor) {

                    // Erase the shape from the map
                    if (auto it = m_ShapesRef.find(shape); it != m_ShapesRef.end()) 
                        m_ShapesRef.erase(it); 

                    m_Actor->detachShape(*shape->GetShape());
                    AddFlag(RigidbodyFlag_ShapesUpdated);
                    L_CORE_INFO("Shape Detached from Rigidbody Successfully.");

                } else  L_CORE_ERROR("Attempted to Detach Shape - Actor is Invalid.");
            } else      L_CORE_WARN ("Attempted to Detach Shape - Shape Not Attached.");
        } else          L_CORE_ERROR("Attempted to Detach Shape - Shape is Invalid.");
    }

    bool RigidDynamic::IsShapeAttached(std::shared_ptr<PhysicsShape> shape) {

        for (auto it = m_ShapesRef.begin(); it != m_ShapesRef.end(); ) {
           
            if (auto shape_ref = it->first.lock(); shape_ref) {
                if (shape_ref == shape) {
                    return true;
                }
                ++it;
            }
            else {

                L_CORE_WARN("Shape Attached to Rigidbody No Longer Exists!");
                it = m_ShapesRef.erase(it);
            }
        }
        return false;

    }

    void RigidDynamic::WakeUp() {
        if (m_Actor) {
            m_Actor->wakeUp();
        }
        else {
            L_CORE_ERROR("Cannot Wake Up Actor - Actor is Nullptr.");
        }
    }

    void RigidDynamic::PutToSleep() {
        if (m_Actor) {
            m_Actor->putToSleep();
        }
        else {
            L_CORE_ERROR("Cannot Put Actor to Sleep - Actor is Nullptr.");
        }
    }

    #pragma endregion

    #pragma region Flagging System

    void RigidDynamic::AddFlag(RigidbodyFlags flag) { m_StateFlags = static_cast<RigidbodyFlags>(m_StateFlags | flag); }
    void RigidDynamic::RemoveFlag(RigidbodyFlags flag) { m_StateFlags = static_cast<RigidbodyFlags>(m_StateFlags & ~flag); }
    bool RigidDynamic::CheckFlag(RigidbodyFlags flag) const { return (m_StateFlags & static_cast<RigidbodyFlags>(flag)) != RigidbodyFlag_None; }
    bool RigidDynamic::NoFlagsSet() const { return m_StateFlags == RigidbodyFlag_None; }
    void RigidDynamic::ClearFlags() { m_StateFlags = RigidbodyFlag_None; }
    RigidbodyFlags RigidDynamic::GetFlags() const { return m_StateFlags; }

    #pragma endregion

    #pragma region Getters and Setters

    PxTransform RigidDynamic::GetGlobalPose() const {
        if (m_Actor) {
            return m_Actor->getGlobalPose();
        }
        L_CORE_ERROR("Cannot Get Transform - Actor is Nullptr.");
        return PxTransform();
    }

    PxVec3 RigidDynamic::GetLinearVelocity() const {
        if (m_Actor) {
            return m_Actor->getLinearVelocity();
        }
        L_CORE_ERROR("Cannot Get Linear Velocity - Actor is Nullptr.");
        return PxVec3();
    }

    PxVec3 RigidDynamic::GetAngularVelocity() const {
        if (m_Actor) {
            return m_Actor->getAngularVelocity();
        }
        L_CORE_ERROR("Cannot Get Angular Velocity - Actor is Nullptr.");
        return PxVec3();
    }

    void RigidDynamic::SetGlobalPose(const PxTransform& pose) {
        if (m_Actor) {
            m_Actor->setGlobalPose(pose);
        }
        else {
            L_CORE_ERROR("Cannot Set Pose - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetGlobalPose(Transform& transform) {
        if (m_Actor) {
            glm::vec3 position = transform.GetGlobalPosition();
            glm::quat quaternion = glm::quat(glm::radians(transform.GetGlobalRotation()));
            m_Actor->setGlobalPose(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));
        }
        else {
            L_CORE_ERROR("Cannot Set Pose - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetGlobalPose(const glm::vec3& position, const glm::vec3& rotation) {
        if (m_Actor) {
            glm::quat quaternion = glm::quat(glm::radians(rotation));
            m_Actor->setGlobalPose(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));
        }
        else {
            L_CORE_ERROR("Cannot Set Pose - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetKinematic(bool isKinematicEnabled) {
        if (m_Actor) {
            m_Actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, isKinematicEnabled);
        }
        else {
            L_CORE_ERROR("Cannot Set Kinematic State - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetKinematicTarget(const PxTransform& pose) {
        if (m_Actor) {
            m_Actor->setKinematicTarget(pose);
        }
        else {
            L_CORE_ERROR("Cannot Set Kinematic Target - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetGravity(bool isGravityEnabled) {
        if (m_Actor) {
            m_Actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !isGravityEnabled);
        }
        else {
            L_CORE_ERROR("Cannot Set Gravity - Actor is Nullptr.");
        }
    }

    void RigidDynamic::AddForce(const PxVec3& force, PxForceMode::Enum mode) {
        if (m_Actor) {
            m_Actor->addForce(force, mode);
        }
        else {
            L_CORE_ERROR("Cannot Add Force - Actor is Nullptr.");
        }
    }

    void RigidDynamic::AddTorque(const PxVec3& torque, PxForceMode::Enum mode) {
        if (m_Actor) {
            m_Actor->addTorque(torque, mode);
        }
        else {
            L_CORE_ERROR("Cannot Add Torque - Actor is Nullptr.");
        }
    }

    void RigidDynamic::AddForceAtPosition(const PxVec3& force, const PxVec3& position, PxForceMode::Enum mode) {
        if (m_Actor) {
            PxRigidBodyExt::addForceAtPos(*m_Actor, force, position, mode);
        }
        else {
            L_CORE_ERROR("Cannot Add Force at Position - Actor is Nullptr.");
        }
    }

    void RigidDynamic::AddForceAtLocalPosition(const PxVec3& force, const PxVec3& position, PxForceMode::Enum mode) {
        if (m_Actor) {
            PxRigidBodyExt::addForceAtPos(*m_Actor, force, position, mode);
        }
        else {
            L_CORE_ERROR("Cannot Add Force at Local Position - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetLinearDamping(PxReal damping) {
        if (m_Actor) {
            m_Actor->setLinearDamping(damping);
        }
        else {
            L_CORE_ERROR("Cannot Set Linear Damping - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetLinearVelocity(const glm::vec3& velocity) {
        if (m_Actor) {
            m_Actor->setLinearVelocity(PxVec3(velocity.x, velocity.y, velocity.z));
        }
        else {
            L_CORE_ERROR("Cannot Set Linear Velocity - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetMaxAngularVelocity(PxReal maxAngularVelocity) {
        if (m_Actor) {
            m_Actor->setMaxAngularVelocity(maxAngularVelocity);
        }
        else {
            L_CORE_ERROR("Cannot Set Max Angular Velocity - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetAngularDamping(PxReal damping) {
        if (m_Actor) {
            m_Actor->setAngularDamping(damping);
        }
        else {
            L_CORE_ERROR("Cannot Set Angular Damping - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetAngularVelocity(const PxVec3& velocity) {
        if (m_Actor) {
            m_Actor->setAngularVelocity(velocity);
        }
        else {
            L_CORE_ERROR("Cannot Set Angular Velocity - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetMaxDepenetrationVelocity(PxReal maxDepenetrationVelocity) {
        if (m_Actor) {
            m_Actor->setMaxDepenetrationVelocity(maxDepenetrationVelocity);
        }
        else {
            L_CORE_ERROR("Cannot Set Max Depenetration Velocity - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetMass(PxReal mass) {
        if (m_Actor) {
            m_Actor->setMass(mass);
        }
        else {
            L_CORE_ERROR("Cannot Set Mass - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetMassSpaceInertiaTensor(const PxVec3& inertia) {
        if (m_Actor) {
            m_Actor->setMassSpaceInertiaTensor(inertia);
        }
        else {
            L_CORE_ERROR("Cannot Set Mass Space Inertia Tensor - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetCMassLocalPose(const PxTransform& pose) {
        if (m_Actor) {
            m_Actor->setCMassLocalPose(pose);
        }
        else {
            L_CORE_ERROR("Cannot Set Center of Mass Local Pose - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetRigidBodyFlag(PxRigidBodyFlag::Enum flag, bool value) {
        if (m_Actor) {
            m_Actor->setRigidBodyFlag(flag, value);
        }
        else {
            L_CORE_ERROR("Cannot Set Rigid Body Flag - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetSolverIterationCounts(PxU32 minPositionIters, PxU32 minVelocityIters) {
        if (m_Actor) {
            m_Actor->setSolverIterationCounts(minPositionIters, minVelocityIters);
        }
        else {
            L_CORE_ERROR("Cannot Set Solver Iteration Counts - Actor is Nullptr.");
        }
    }

    void RigidDynamic::SetRigidDynamicLockFlag(PxRigidDynamicLockFlag::Enum flag, bool value) {
        if (m_Actor) {
            m_Actor->setRigidDynamicLockFlag(flag, value);
        }
        else {
            L_CORE_ERROR("Cannot Set Rigid Dynamic Lock Flag - Actor is Nullptr.");
        }
    }

    #pragma endregion

#pragma endregion
    
#pragma region PhysicsMaterial

    PhysicsMaterial::PhysicsMaterial() {
        m_Material = PxGetPhysics().createMaterial(m_StaticFriction, m_DynamicFriction, m_Bounciness);
    }

    PhysicsMaterial::PhysicsMaterial(float dynamicFriction, float staticFriction, float bounciness) :
        m_DynamicFriction(dynamicFriction),
        m_StaticFriction(staticFriction),
        m_Bounciness(bounciness) {

        m_Material = PxGetPhysics().createMaterial(m_StaticFriction, m_DynamicFriction, m_Bounciness);
    }

    PhysicsMaterial::PhysicsMaterial(const PhysicsMaterial& other) {
        m_Material = PxGetPhysics().createMaterial(other.m_StaticFriction, other.m_DynamicFriction, other.m_Bounciness);
    }

    PhysicsMaterial::~PhysicsMaterial() {
        if (m_Material) {
            m_Material->release();
            m_Material = nullptr;
        }
    }

    PxMaterial* PhysicsMaterial::GetMaterial() const { return m_Material; }

    // GETTERS
    float PhysicsMaterial::GetDynamicFriction() const { return m_DynamicFriction; }
    float PhysicsMaterial::GetStaticFriction() const { return m_StaticFriction; }
    float PhysicsMaterial::GetBounciness() const { return m_Bounciness; }

    // SETTERS
    void PhysicsMaterial::SetDynamicFriction(float dynamicFriction) {
        m_DynamicFriction = dynamicFriction;
        if (m_Material) m_Material->setDynamicFriction(m_DynamicFriction);
    }

    void PhysicsMaterial::SetStaticFriction(float staticFriction) {
        m_StaticFriction = staticFriction;
        if (m_Material) m_Material->setStaticFriction(m_StaticFriction);
    }

    void PhysicsMaterial::SetBounciness(float bounciness) {
        m_Bounciness = bounciness;
        if (m_Material) m_Material->setRestitution(m_Bounciness);
    }

#pragma endregion


#pragma region PhysicsShape

    #pragma region Constructors and Class Setup

    PhysicsShape::PhysicsShape(const PxGeometry& geometry, const PxMaterial& material, bool isExclusive, PxShapeFlags shapeFlags) :
        m_Shape(PxGetPhysics().createShape(geometry, material, isExclusive, shapeFlags))
    {
        m_Shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
        if (!m_Shape) {
            L_CORE_ERROR("Failed to Create PxShape.");
            m_Shape = nullptr;
        }
    }

    PhysicsShape::PhysicsShape(PhysicsShape&& other) noexcept : 
        m_Shape(other.m_Shape) 
    {
        m_Shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
        other.m_Shape = nullptr;
    }

    PhysicsShape& PhysicsShape::operator=(PhysicsShape&& other) noexcept {
        if (this != &other) {
            Release();
            m_Shape = other.m_Shape;
            m_Shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
            other.m_Shape = nullptr;
        }
        return *this;
    }

    PhysicsShape::operator bool() const { return m_Shape; }

    PhysicsShape::~PhysicsShape() { Release(); }

    #pragma endregion

    #pragma region Class Logic

    PxShape* PhysicsShape::GetShape() const { return m_Shape;  }

    void PhysicsShape::Release() {
        if (m_Shape) {
            m_Shape->release();
            m_Shape = nullptr;
        }
        if (m_StaticBody) {
            m_StaticBody->Release();
            m_StaticBody = nullptr;
        }
    }

    bool PhysicsShape::IsStatic() const { return (m_StaticBody && *m_StaticBody) ? true : false; }

    std::shared_ptr<RigidDynamic> PhysicsShape::GetRigidbody() {

        if (m_StaticBody && *m_StaticBody)
            return m_StaticBody;

        if (auto rb_ref = m_RigidbodyRef.lock(); rb_ref && *rb_ref)
            return rb_ref;

        return nullptr;
    }

    #pragma endregion

    #pragma region Flagging System

    void PhysicsShape::AddFlag(ColliderFlags flag) { m_StateFlags = static_cast<ColliderFlags>(m_StateFlags | flag); }
    void PhysicsShape::ClearFlag(ColliderFlags flag) { m_StateFlags = static_cast<ColliderFlags>(m_StateFlags & ~flag); }
    bool PhysicsShape::CheckFlag(ColliderFlags flag) const { return (m_StateFlags & static_cast<ColliderFlags>(flag)) != ColliderFlag_None; }
    bool PhysicsShape::NoFlagsSet() const { return m_StateFlags == ColliderFlag_None; }
    void PhysicsShape::ClearFlags() { m_StateFlags = ColliderFlag_None; }
    ColliderFlags PhysicsShape::GetFlags() const { return m_StateFlags; }

    #pragma endregion

    #pragma region Getters and Setters

    PxGeometryType::Enum PhysicsShape::GetGeometryType() const {
        if (m_Shape) {
            return m_Shape->getGeometry().getType();
        }
        L_CORE_ERROR("Cannot Get Geometry Type - Shape is Nullptr.");
        return PxGeometryType::eINVALID;
    }

    PxMaterial* PhysicsShape::GetMaterial() const {
        if (m_Shape) {
            PxMaterial* material = nullptr;
            m_Shape->getMaterials(&material, 1);
            return material;
        }
        L_CORE_ERROR("Cannot Get PxMaterial - Shape is Nullptr.");
        return nullptr;
    }

    PxTransform PhysicsShape::GetLocalPose() const {
        if (m_Shape) {
            return m_Shape->getLocalPose();
        }
        L_CORE_ERROR("Cannot Get PxTransform - Shape is Nullptr.");
        return PxTransform();
    }

    PxFilterData PhysicsShape::GetSimulationFilterData() const {
        if (m_Shape) {
            return m_Shape->getSimulationFilterData();
        }
        L_CORE_ERROR("Cannot Get PxFilterData - Shape is Nullptr.");
        return PxFilterData();
    }

    void PhysicsShape::SetLocalPose(const PxTransform& pose) {
        if (m_Shape) {

            auto rb_ref = m_RigidbodyRef.lock();
            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->detachShape(*m_Shape);

            m_Shape->setLocalPose(pose);

            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->attachShape(*m_Shape);

            return;
        }
        L_CORE_ERROR("Cannot Set Pose - Shape is Nullptr.");
    }

    void PhysicsShape::SetLocalPose(const glm::vec3& local_position, const glm::vec3& local_rotation) {
        if (m_Shape) {

            auto rb_ref = m_RigidbodyRef.lock();
            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->detachShape(*m_Shape);
            
            glm::quat quaternion = glm::quat(glm::radians(local_rotation));
            m_Shape->setLocalPose(
                PxTransform(
                    local_position.x,
                    local_position.y,
                    local_position.z,
                PxQuat(
                    quaternion.x,
                    quaternion.y,
                    quaternion.z,
                    quaternion.w))
            );

            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->attachShape(*m_Shape);

            return;
        }
        L_CORE_ERROR("Cannot Set Pose - Shape is Nullptr.");    
    }

    void PhysicsShape::SetLocalPose(Transform& transform) {
        if (m_Shape) {

            auto rb_ref = m_RigidbodyRef.lock();
            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->detachShape(*m_Shape);

            glm::vec3 local_position = transform.GetLocalPosition();
            glm::quat quaternion = glm::quat(glm::radians(transform.GetLocalRotation()));
            m_Shape->setLocalPose(
                PxTransform(
                    local_position.x,
                    local_position.y,
                    local_position.z,
                PxQuat(
                    quaternion.x,
                    quaternion.y,
                    quaternion.z,
                    quaternion.w))
            );

            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->attachShape(*m_Shape);
            return;
        }
        L_CORE_ERROR("Cannot Set Pose - Shape is Invalid.");
    }

    void PhysicsShape::SetGeometry(const PxGeometry& geom) {
        if (m_Shape) {
            auto rb_ref = m_RigidbodyRef.lock();
            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->detachShape(*m_Shape);

            m_Shape->setGeometry(geom);

            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->attachShape(*m_Shape);
            
            return;
        }
        L_CORE_ERROR("Cannot Set Geometry - Shape is Invalid.");
    }

    void PhysicsShape::SetSimulationFilterData(const PxFilterData& filterData) {
        if (m_Shape) {
            auto rb_ref = m_RigidbodyRef.lock();
            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->detachShape(*m_Shape);

            m_Shape->setSimulationFilterData(filterData);

            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->attachShape(*m_Shape);
            return;
        }
        L_CORE_ERROR("Cannot Set PxFilterData - Shape is Invalid.");
    }

    void PhysicsShape::SetFlag(PxShapeFlag::Enum flag, bool value) {
        if (m_Shape) {
            auto rb_ref = m_RigidbodyRef.lock();
            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->detachShape(*m_Shape);

            m_Shape->setFlag(flag, value);

            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->attachShape(*m_Shape);
            return;
        }
        L_CORE_ERROR("Cannot Set PxShapeFlag - Shape is Invalid.");
        
    }

    void PhysicsShape::SetFlags(PxShapeFlags flags) {
        if (m_Shape) {
            auto rb_ref = m_RigidbodyRef.lock();
            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->detachShape(*m_Shape);

            m_Shape->setFlags(flags);

            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->attachShape(*m_Shape);
            return;
        }
        L_CORE_ERROR("Cannot Set PxShapeFlags - Shape is Invalid.");
    }

    void PhysicsShape::SetMaterials(PxMaterial* const* materials, PxU32 materialCount) {
        if (m_Shape) {
            auto rb_ref = m_RigidbodyRef.lock();
            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->detachShape(*m_Shape);

            m_Shape->setMaterials(materials, materialCount);

            if (rb_ref && *rb_ref)
                rb_ref->GetActor()->attachShape(*m_Shape);
            return;
        }
        L_CORE_ERROR("Cannot Set PxMaterial - Shape is Invalid.");
    }

    #pragma endregion

#pragma endregion

}
