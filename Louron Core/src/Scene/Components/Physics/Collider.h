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

    struct Rigidbody;
    class RigidStatic;
    class RigidDynamic;
    class PhysicsShape;
    class PhysicsMaterial;
    class Entity;

    struct SphereCollider : public Component {

    private:

        std::shared_ptr<PhysicsShape> m_Shape = nullptr;
        std::shared_ptr<PhysicsMaterial> m_Material = nullptr;

        float m_Radius = 0.5f;
        bool m_IsTrigger = false;
        glm::vec3 m_Centre{ 0.0f, 0.0f, 0.0f };

        std::function<void(Entity&, Entity&)> OnCollideCallback;
        std::function<void(Entity&, Entity&)> OnTriggerCallback;

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

        SphereCollider();
        SphereCollider(glm::vec3 SphereCentre, float SphereRadius);
        SphereCollider(const SphereCollider& other);
        SphereCollider(SphereCollider&& other) = default;
        ~SphereCollider();

        SphereCollider& operator=(const SphereCollider& other);

        void SetColliderUserData(const UUID& uuid);
        // Setter functions for callbacks
        void SetOnCollideCallback(const std::function<void(Entity&, Entity&)>& callback) { OnCollideCallback = callback; }
        void SetOnTriggerCallback(const std::function<void(Entity&, Entity&)>& callback) { OnTriggerCallback = callback; }

        const UUID& GetRigidbodyUUID() const { return m_RigidbodyUUID; }

        void Release();

        void UpdateTransform(Transform& collider_transform, Transform& rigidbody_transform);

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

    struct BoxCollider : public Component {

    private:

        std::shared_ptr<PhysicsShape> m_Shape = nullptr;
        std::shared_ptr<PhysicsMaterial> m_Material = nullptr;

        bool m_IsTrigger = false;
        glm::vec3 m_Centre{ 0.0f, 0.0f, 0.0f };
        glm::vec3 m_BoxExtents{ 1.0f, 1.0f, 1.0f };

        std::function<void(Entity&, Entity&)> OnCollideCallback;
        std::function<void(Entity&, Entity&)> OnTriggerCallback;

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

        BoxCollider();
        BoxCollider(const glm::vec3& boxCentre, const glm::vec3& boxExtents);
        BoxCollider(const BoxCollider& other);
        BoxCollider(BoxCollider&& other) = default;
        ~BoxCollider();

        BoxCollider& operator=(const BoxCollider& other);

        void SetColliderUserData(const UUID& uuid);
        // Setter functions for callbacks
        void SetOnCollideCallback(const std::function<void(Entity&, Entity&)>& callback) { OnCollideCallback = callback; }
        void SetOnTriggerCallback(const std::function<void(Entity&, Entity&)>& callback) { OnTriggerCallback = callback; }

        const UUID& GetRigidbodyUUID() const { return m_RigidbodyUUID; }

        void Release();

        void UpdateTransform(Transform& collider_transform, Transform& rigidbody_transform);

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