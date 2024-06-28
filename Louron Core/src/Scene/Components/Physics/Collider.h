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

namespace Louron {

    struct Rigidbody;
    class RigidStatic;
    class RigidDynamic;
    class PhysicsShape;
    class PhysicsMaterial;

    struct SphereCollider : public Component {

    private:

        std::shared_ptr<PhysicsShape> m_Shape = nullptr;
        std::shared_ptr<PhysicsMaterial> m_Material = nullptr;

        float m_Radius = 0.5f;
        bool m_IsTrigger = false;
        glm::vec3 m_Centre{ 0.0f, 0.0f, 0.0f };

    public:

        SphereCollider();
        SphereCollider(glm::vec3 SphereCentre, float SphereRadius);
        ~SphereCollider();

        void Release();

        void UpdateTransform(Transform& collider_transform, Transform& rigidbody_transform);

        // GETTERS
        const bool& GetIsTrigger() const;
        std::shared_ptr<PhysicsMaterial> GetMaterial() const;

        const glm::vec3& GetCentre() const;
        const float& GetRadius() const;

        std::shared_ptr<PhysicsShape> GetShape() const;

        // SETTERS
        void SetIsTrigger(bool isTrigger);
        void SetMaterial(std::shared_ptr<PhysicsMaterial> material);

        void SetCentre(const glm::vec3& centre);
        void SetRadius(float radius);

    private:

        friend class Scene;
        friend class Entity;
        friend class PhysicsSystem;
        friend class TransformSystem;

    };

    struct BoxCollider : public Component {

    private:

        std::shared_ptr<PhysicsShape> m_Shape = nullptr;
        std::shared_ptr<PhysicsMaterial> m_Material = nullptr;

        bool m_IsTrigger = false;
        glm::vec3 m_Centre{ 0.0f, 0.0f, 0.0f };
        glm::vec3 m_BoxExtents{ 1.0f, 1.0f, 1.0f };

    public:

        BoxCollider();
        BoxCollider(const glm::vec3& boxCentre, const glm::vec3& boxExtents);
        ~BoxCollider();

        void Release();

        void UpdateTransform(Transform& collider_transform, Transform& rigidbody_transform);

        // GETTERS
        const bool& GetIsTrigger() const;
        std::shared_ptr<PhysicsMaterial> GetMaterial() const;

        const glm::vec3& GetCentre() const;
        const glm::vec3& GetSize() const;

        std::shared_ptr<PhysicsShape> GetShape() const;

        // SETTERS
        void SetIsTrigger(bool isTrigger);
        void SetMaterial(std::shared_ptr<PhysicsMaterial> material);

        void SetCentre(const glm::vec3& centre);
        void SetSize(const glm::vec3& boxExtents);

    private:

        friend class Scene;
        friend class Entity;
        friend class PhysicsSystem;
        friend class TransformSystem;

    };

 

}