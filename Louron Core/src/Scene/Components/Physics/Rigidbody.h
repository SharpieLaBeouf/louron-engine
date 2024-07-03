#pragma once

// Louron Core Headers
#include "../Components.h"
#include "PhysicsWrappers.h"


// C++ Standard Library Headers
#include <memory>
#include <map>

// External Vendor Library Headers
#include <physx/PxPhysicsAPI.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace physx;

namespace Louron {

	struct Rigidbody : public Component {

	private:

		std::shared_ptr<RigidDynamic> m_RigidDynamic = nullptr;

		PxScene* m_PhysScene = nullptr;

		float m_Mass = 1.0f;
		float m_Drag = 0.0f;
		float m_AngularDrag = 0.05f;

		bool m_AutomaticCentreOfMass = true;
		bool m_UseGravity = true;
		bool m_IsKinematic = false;

		glm::bvec3 m_PositionConstraint = { false, false, false };
		glm::bvec3 m_RotationConstraint = { false, false, false };

	public:

		Rigidbody() = default;
		Rigidbody(Transform& transform, PxScene* scene);
		Rigidbody(const Rigidbody&) = default;
		~Rigidbody() = default;

		/// <summary>
		/// Only use this method for direct access to the actor instance.
		/// </summary>
		/// <returns></returns>
		std::shared_ptr<RigidDynamic> GetActor();

		friend class PhysicsSystem;
	
	public:

		// GETTERS
		const float& GetMass() const;
		const float& GetDrag() const;
		const float& GetAngularDrag() const;
		const bool& IsAutomaticCentreOfMassEnabled() const;
		const bool& IsGravityEnabled() const;
		const bool& IsKinematicEnabled() const;

		const glm::bvec3& GetPositionConstraint() const;
		const glm::bvec3& GetRotationConstraint() const;

		// SETTERS
		void SetMass(const float& mass);
		void SetDrag(const float& drag);
		void SetAngularDrag(const float& angularDrag);
		void SetAutomaticCentreOfMass(const bool& automaticCentreOfMass);
		void SetGravity(const bool& useGravity);
		void SetKinematic(const bool& isKinematic);
		void SetPositionConstraint(const glm::bvec3& positionConstraint);
		void SetRotationConstraint(const glm::bvec3& rotationConstraint);

		void ApplyForce(const glm::vec3& force, PxForceMode::Enum forceMode = PxForceMode::eFORCE);
		void ApplyTorque(const glm::vec3& torque);

		struct DeferredForce {
			glm::vec3 force;
			PxForceMode::Enum forceMode;
		};
		struct DeferredTorque {
			glm::vec3 torque;
		};

		// This ensures that any force that is applied is not applied 
		// whilst simulate is processing physics changes
		std::vector<DeferredForce> m_DeferredForce;
		std::vector<DeferredTorque> m_DeferredTorque;
	};

}