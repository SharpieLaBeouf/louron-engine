#pragma once

// Louron Core Headers
#include "../Mesh.h"
#include "../Components.h"

// C++ Standard Library Headers
#include <memory>

// External Vendor Library Headers
#include <physx/PxPhysicsAPI.h>
#include <glm/glm.hpp>

using namespace physx;

namespace YAML {

    class Emitter;
    class Node;
}

namespace Louron {

    struct RigidbodyComponent;
    class RigidStatic;
    class RigidDynamic;
    class PhysicsShape;
    class PhysicsMaterial;
    class Entity;

    /// <summary>
    /// Flags that are set to determine what state changes have occured
    /// each frame so the PhysicsSystem may process these changes.
    /// </summary>
    enum ColliderFlags : uint8_t {

        ColliderFlag_None = 0,

        ColliderFlag_TransformUpdated = 1U << 0,  // Only add this flag where there have been manual changes made to the transform of the shape.
        ColliderFlag_RigidbodyUpdated = 1U << 1,  // Add this flag when there have been changes to the rigidbody reference.
        ColliderFlag_ShapePropsUpdated = 1U << 2,	// Add this flag when the properties of the shape have been updated.
    };

    struct SphereColliderComponent : public Component {

    private:

        std::shared_ptr<PhysicsShape> m_Shape = nullptr;
        std::shared_ptr<PhysicsMaterial> m_Material = nullptr;

        float m_Radius = 0.5f;
        bool m_IsTrigger = false;
        glm::vec3 m_Centre{ 0.0f, 0.0f, 0.0f };

        ColliderFlags m_StateFlags = ColliderFlag_None;

        // we do this because we store this in the PxShape void* userData,
        // we don't want any cases when the memory of the entityUUID is 
        // reallocated or suddenly becomes null! 
        UUID m_EntityUUID = NULL_UUID;

        // We hold the UUID of the parent that our collider is attached to.
        UUID m_RigidbodyUUID = NULL_UUID;

        void CreateStaticRigidbody();
        void UpdateRigidbody(const UUID& rigidbodyEntityUUID);
        void ResetRigidbody();

    public:

        void Init();
        void Shutdown();

        SphereColliderComponent();
        SphereColliderComponent(glm::vec3 SphereCentre, float SphereRadius);
        SphereColliderComponent(const SphereColliderComponent& other);
        SphereColliderComponent(SphereColliderComponent&& other) noexcept;
        ~SphereColliderComponent();

        SphereColliderComponent& operator=(const SphereColliderComponent& other);
        SphereColliderComponent& operator=(SphereColliderComponent&& other) noexcept;

        void SetColliderUserData(const UUID& uuid);

        const UUID& GetRigidbodyUUID() const { return m_RigidbodyUUID; }

        void Release();

        void UpdateTransform(TransformComponent& collider_transform, TransformComponent& rigidbody_transform);

        // FLAGS
        void AddFlag(ColliderFlags flag);
        void ClearFlag(ColliderFlags flag);
        bool CheckFlag(ColliderFlags flag) const;
        bool NoFlagsSet() const;
        void ClearFlags();
        ColliderFlags GetFlags() const;

        // GETTERS
        const bool& IsTrigger() const;
        std::shared_ptr<PhysicsMaterial> GetMaterial() const;

        const glm::vec3& GetCentre() const;
        const float& GetRadius() const;

        std::shared_ptr<PhysicsShape> GetShape() const;

        // SETTERS
        void SetIsTrigger(bool isTrigger);
        void SetMaterial(std::shared_ptr<PhysicsMaterial> material);

        void SetCentre(const glm::vec3& centre);
        void SetRadius(float radius);

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);

    private:

        friend class Scene;
        friend class Entity;
        friend class PhysicsSystem;
        friend class TransformSystem;
        friend class CollisionCallback;

        friend struct HierarchyComponent;

        template<typename ColliderType>
        friend void ProcessColliderChanges(Entity& entity);

    };

    struct BoxColliderComponent : public Component {

    private:

        std::shared_ptr<PhysicsShape> m_Shape = nullptr;
        std::shared_ptr<PhysicsMaterial> m_Material = nullptr;

        bool m_IsTrigger = false;
        glm::vec3 m_Centre{ 0.0f, 0.0f, 0.0f };
        glm::vec3 m_BoxExtents{ 1.0f, 1.0f, 1.0f };

        ColliderFlags m_StateFlags = ColliderFlag_None;

        // we do this because we store this in the PxShape void* userData,
        // we don't want any cases when the memory of the entityUUID is 
        // reallocated or suddenly becomes null! 
        UUID m_EntityUUID; 

        // We hold the UUID of the parent that our collider is attached to.
        UUID m_RigidbodyUUID = NULL_UUID;

        void CreateStaticRigidbody();
        void UpdateRigidbody(const UUID& rigidbodyEntityUUID);
        void ResetRigidbody();

    public:

        void Init();
        void Shutdown();

        BoxColliderComponent();
        BoxColliderComponent(const glm::vec3& boxCentre, const glm::vec3& boxExtents);
        BoxColliderComponent(const BoxColliderComponent& other);
        BoxColliderComponent(BoxColliderComponent&& other) noexcept;
        ~BoxColliderComponent();

        BoxColliderComponent& operator=(const BoxColliderComponent& other);
        BoxColliderComponent& operator=(BoxColliderComponent&& other) noexcept;

        void SetColliderUserData(const UUID& uuid);

        const UUID& GetRigidbodyUUID() const { return m_RigidbodyUUID; }

        void Release();

        void UpdateTransform(TransformComponent& collider_transform, TransformComponent& rigidbody_transform);

        // FLAGS
        void AddFlag(ColliderFlags flag);
        void ClearFlag(ColliderFlags flag);
        bool CheckFlag(ColliderFlags flag) const;
        bool NoFlagsSet() const;
        void ClearFlags();
        ColliderFlags GetFlags() const;

        // GETTERS
        const bool& IsTrigger() const;
        std::shared_ptr<PhysicsMaterial> GetMaterial() const;

        const glm::vec3& GetCentre() const;
        const glm::vec3& GetSize() const;

        std::shared_ptr<PhysicsShape> GetShape() const;

        // SETTERS
        void SetIsTrigger(bool isTrigger);
        void SetMaterial(std::shared_ptr<PhysicsMaterial> material);

        void SetCentre(const glm::vec3& centre);
        void SetSize(const glm::vec3& boxExtents);

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);

    private:

        friend class Scene;
        friend class Entity;
        friend class PhysicsSystem;
        friend class TransformSystem;
        friend class CollisionCallback;

        friend struct HierarchyComponent;

        template<typename ColliderType>
        friend void ProcessColliderChanges(Entity& entity);
    };

 

}