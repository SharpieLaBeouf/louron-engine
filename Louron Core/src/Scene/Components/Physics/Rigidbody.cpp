#include "Rigidbody.h"

// Louron Core Headers
#include "PhysicsWrappers.h"

#include "../Components.h"
#include "../../Entity.h"
#include "../../../Core/Logging.h"

// C++ Standard Library Headers


// External Vendor Library Headers

namespace Louron {

	Rigidbody::Rigidbody(Transform& transform, PxScene* scene) : m_PhysScene(scene)
	{
		if (!&PxGetPhysics() || !m_PhysScene) {
			L_CORE_ERROR("PhysXPhysics or PhysXScene Not Initialized!");
			return;
		}

		glm::vec3 position = transform.GetGlobalPosition();
		glm::quat quaternion = glm::quat(glm::radians(transform.GetGlobalRotation()));
		m_RigidDynamic = std::make_shared<RigidDynamic>(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

		if (!m_RigidDynamic && !*m_RigidDynamic) {
			L_CORE_ERROR("Could Not Create Rigid Dynamic Actor!");
			return;
		}

		PxRigidBodyExt::updateMassAndInertia(*m_RigidDynamic->GetActor(), 1.0f);
		m_PhysScene->addActor(*m_RigidDynamic->GetActor());

		// Set initial properties
		SetMass(m_Mass);
		SetDrag(m_Drag);
		SetAngularDrag(m_AngularDrag);
		SetGravity(m_UseGravity);
		SetKinematic(m_IsKinematic);
	}

	std::shared_ptr<RigidDynamic> Rigidbody::GetActor() { return m_RigidDynamic; }

	const float& Rigidbody::GetMass() const { return m_Mass; }
	const float& Rigidbody::GetDrag() const { return m_Drag; }
	const float& Rigidbody::GetAngularDrag() const { return m_AngularDrag; }
	const bool& Rigidbody::IsAutomaticCentreOfMassEnabled() const { return m_AutomaticCentreOfMass; }
	const bool& Rigidbody::IsGravityEnabled() const { return m_UseGravity; }
	const bool& Rigidbody::IsKinematicEnabled() const { return m_IsKinematic; }

	const glm::bvec3& Rigidbody::GetPositionConstraint() const { return m_PositionConstraint; }
	const glm::bvec3& Rigidbody::GetRotationConstraint() const { return m_RotationConstraint; }

	void Rigidbody::SetMass(const float& mass) {
		m_Mass = mass;
		if (m_RigidDynamic) m_RigidDynamic->SetMass(m_Mass);
	}

	void Rigidbody::SetDrag(const float& drag) {
		m_Drag = drag;
		if (m_RigidDynamic) m_RigidDynamic->SetLinearDamping(m_Drag);
	}

	void Rigidbody::SetAngularDrag(const float& angularDrag) {
		m_AngularDrag = angularDrag;
		if (m_RigidDynamic) m_RigidDynamic->SetAngularDamping(m_AngularDrag);
	}

	void Rigidbody::SetAutomaticCentreOfMass(const bool& automaticCentreOfMass) {
		// TODO: have option for custom centre of gravity

		m_AutomaticCentreOfMass = automaticCentreOfMass;
	}

	void Rigidbody::SetGravity(const bool& useGravity) {
		m_UseGravity = useGravity;
		if (m_RigidDynamic) m_RigidDynamic->SetGravity(useGravity);
	}

	void Rigidbody::SetKinematic(const bool& isKinematic) {
		m_IsKinematic = isKinematic;
		if (m_RigidDynamic) m_RigidDynamic->SetKinematic(isKinematic);
	}

	void Rigidbody::SetPositionConstraint(const glm::bvec3& positionConstraint) {

		// TODO: apply Physx Constraint
		m_PositionConstraint = positionConstraint;
	}
	void Rigidbody::SetRotationConstraint(const glm::bvec3& rotationConstraint) {

		// TODO: apply Physx Constraint
		m_RotationConstraint = rotationConstraint;
	}

	// Apply force to the rigid body
	// If the scene is currently in simulation, we want to defer 
	// this force to next frames physics simulation 
	void Rigidbody::ApplyForce(const glm::vec3& force, PxForceMode::Enum forceMode) {
		if (m_RigidDynamic) {
			if (entity->GetScene()->IsPhysicsSimulating())
				m_DeferredForce.push_back({ force, forceMode });
			else
				m_RigidDynamic->AddForce(PxVec3(force.x, force.y, force.z), forceMode);
		}
	}

	// Apply torque to the rigid body
	// If the scene is currently in simulation, we want to defer 
	// this torque to next frames physics simulation 
	void Rigidbody::ApplyTorque(const glm::vec3& torque) {
		if (m_RigidDynamic) {
			if (entity->GetScene()->IsPhysicsSimulating())
				m_DeferredTorque.push_back({ torque });
			else
				m_RigidDynamic->AddTorque(PxVec3(torque.x, torque.y, torque.z));
		}
	}

}

