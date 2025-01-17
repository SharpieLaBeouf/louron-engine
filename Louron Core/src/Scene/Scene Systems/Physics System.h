#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <memory>
#include <optional>

// External Vendor Library Headers
#include <physx/PxPhysicsAPI.h>

using namespace physx;

namespace Louron {

	class Scene;
	class Entity;

	struct RigidbodyComponent;
	struct SphereColliderComponent;
	struct BoxColliderComponent;

	class PhysicsSystem {


	public:

		static RigidbodyComponent& AddRigidBody(Entity entity, Scene* scene);
		static void RemoveRigidBody(Entity entity, Scene* scene);

		static SphereColliderComponent& AddSphereCollider(Entity entity, Scene* scene);
		static BoxColliderComponent& AddBoxCollider(Entity entity, Scene* scene);
		static void RemoveCollider(Entity entity, Scene* scene, PxGeometryType::Enum colliderType);

		static void Update(std::shared_ptr<Scene> scene);

		static void UpdatePhysicsObjects(std::shared_ptr<Scene> scene);


	private:

		static void UpdateSimulationChanges(std::shared_ptr<Scene> scene);

		PhysicsSystem() = delete;
		PhysicsSystem(const PhysicsSystem&) = delete;
		~PhysicsSystem() = delete;
	};


}