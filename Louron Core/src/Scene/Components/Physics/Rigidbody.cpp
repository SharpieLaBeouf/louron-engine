#include "Rigidbody.h"

// Louron Core Headers
#include "PhysicsWrappers.h"

#include "../Components.h"
#include "../../Entity.h"
#include "../../../Core/Logging.h"

// C++ Standard Library Headers


// External Vendor Library Headers

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron {

	void RigidbodyComponent::Init(TransformComponent* transform, PxScene* scene) {

		if (!&PxGetPhysics() || !scene) {
			L_CORE_ERROR("PhysXPhysics or PhysXScene Not Initialized.");
			return;
		}

		if (!transform) {
			L_CORE_ERROR("Rigidbody Constructor - Transform Pointer Invalid.");
			return;
		}

		m_PhysScene = scene;

		glm::vec3 position = transform->GetGlobalPosition();
		glm::quat quaternion = glm::quat(glm::radians(transform->GetGlobalRotation()));
		
		PxTransform trans = { position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w) };
		m_RigidDynamic = std::make_shared<RigidDynamic>();
		
		m_RigidDynamic->Init(trans);

		if (!m_RigidDynamic && !*m_RigidDynamic) {
			L_CORE_ERROR("Could Not Create Rigid Dynamic Actor.");
			return;
		}

		m_PhysScene->addActor(*m_RigidDynamic->GetActor());

		// Set initial properties
		SetMass(m_Mass);
		SetDrag(m_Drag);
		SetAngularDrag(m_AngularDrag);
		SetGravity(m_UseGravity);
		SetKinematic(m_IsKinematic);

		PxRigidBodyExt::setMassAndUpdateInertia(*m_RigidDynamic->GetActor(), m_Mass);

		m_RigidDynamic->AddFlag(RigidbodyFlag_ShapesUpdated);
		m_RigidDynamic->AddFlag(RigidbodyFlag_TransformUpdated);
	}

	void RigidbodyComponent::Shutdown() {

		if(m_PhysScene)
			m_PhysScene->removeActor(*m_RigidDynamic->GetActor());

		if(m_RigidDynamic) {
			m_RigidDynamic->Shutdown();
			m_RigidDynamic = nullptr;
		}

		m_PhysScene = nullptr;

	}

	RigidbodyComponent::RigidbodyComponent(const RigidbodyComponent& other) {

		m_Mass = other.m_Mass;
		m_Drag = other.m_Drag;
		m_AngularDrag = other.m_AngularDrag;

		m_AutomaticCentreOfMass = other.m_AutomaticCentreOfMass;
		m_UseGravity = other.m_UseGravity;
		m_IsKinematic = other.m_IsKinematic;

		m_PositionConstraint = other.m_PositionConstraint;
		m_RotationConstraint = other.m_RotationConstraint;

		m_PhysScene = nullptr;
		m_RigidDynamic = nullptr;
	}

	RigidbodyComponent& RigidbodyComponent::operator=(const RigidbodyComponent& other) {

		m_Mass = other.m_Mass;
		m_Drag = other.m_Drag;
		m_AngularDrag = other.m_AngularDrag;

		m_AutomaticCentreOfMass = other.m_AutomaticCentreOfMass;
		m_UseGravity = other.m_UseGravity;
		m_IsKinematic = other.m_IsKinematic;

		m_PositionConstraint = other.m_PositionConstraint;
		m_RotationConstraint = other.m_RotationConstraint;

		m_PhysScene = nullptr;
		m_RigidDynamic = nullptr;

		return *this;
	}

	std::shared_ptr<RigidDynamic> RigidbodyComponent::GetActor() { return m_RigidDynamic; }

	const float& RigidbodyComponent::GetMass() const { return m_Mass; }
	const float& RigidbodyComponent::GetDrag() const { return m_Drag; }
	const float& RigidbodyComponent::GetAngularDrag() const { return m_AngularDrag; }
	const bool& RigidbodyComponent::IsAutomaticCentreOfMassEnabled() const { return m_AutomaticCentreOfMass; }
	const bool& RigidbodyComponent::IsGravityEnabled() const { return m_UseGravity; }
	const bool& RigidbodyComponent::IsKinematicEnabled() const { return m_IsKinematic; }

	const glm::bvec3& RigidbodyComponent::GetPositionConstraint() const { return m_PositionConstraint; }
	const glm::bvec3& RigidbodyComponent::GetRotationConstraint() const { return m_RotationConstraint; }

	void RigidbodyComponent::SetMass(const float& mass) {
		m_Mass = mass;
		if (m_RigidDynamic) m_RigidDynamic->SetMass(m_Mass);
	}

	void RigidbodyComponent::SetDrag(const float& drag) {
		m_Drag = drag;
		if (m_RigidDynamic) m_RigidDynamic->SetLinearDamping(m_Drag);
	}

	void RigidbodyComponent::SetAngularDrag(const float& angularDrag) {
		m_AngularDrag = angularDrag;
		if (m_RigidDynamic) m_RigidDynamic->SetAngularDamping(m_AngularDrag);
	}

	void RigidbodyComponent::SetAutomaticCentreOfMass(const bool& automaticCentreOfMass) {
		// TODO: have option for custom centre of gravity

		m_AutomaticCentreOfMass = automaticCentreOfMass;
	}

	void RigidbodyComponent::SetGravity(const bool& useGravity) {
		m_UseGravity = useGravity;
		if (m_RigidDynamic) m_RigidDynamic->SetGravity(useGravity);
	}

	void RigidbodyComponent::SetKinematic(const bool& isKinematic) {
		m_IsKinematic = isKinematic;
		if (m_RigidDynamic) m_RigidDynamic->SetKinematic(isKinematic);
	}

	void RigidbodyComponent::SetPositionConstraint(const glm::bvec3& positionConstraint) {

		// TODO: apply Physx Constraint
		m_PositionConstraint = positionConstraint;
	}
	void RigidbodyComponent::SetRotationConstraint(const glm::bvec3& rotationConstraint) {

		// TODO: apply Physx Constraint
		m_RotationConstraint = rotationConstraint;
	}

	// Apply force to the rigid body
	// If the scene is currently in simulation, we want to defer 
	// this force to next frames physics simulation 
	void RigidbodyComponent::ApplyForce(const glm::vec3& force, PxForceMode::Enum forceMode) {
		if (m_RigidDynamic) {
			Entity entity = GetEntity();
			if (entity.GetScene()->IsPhysicsCalculating())
				m_DeferredForce.push_back({ force, forceMode });
			else
				m_RigidDynamic->AddForce(PxVec3(force.x, force.y, force.z), forceMode);
		}
	}

	// Apply torque to the rigid body
	// If the scene is currently in simulation, we want to defer 
	// this torque to next frames physics simulation 
	void RigidbodyComponent::ApplyTorque(const glm::vec3& torque) {
		if (m_RigidDynamic) {
			Entity entity = GetEntity();
			if (entity.GetScene()->IsPhysicsCalculating())
				m_DeferredTorque.push_back({ torque });
			else
				m_RigidDynamic->AddTorque(PxVec3(torque.x, torque.y, torque.z));
		}
	}

	void RigidbodyComponent::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "RigidbodyComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "Mass" << YAML::Value << m_Mass;
		out << YAML::Key << "Drag" << YAML::Value << m_Drag;
		out << YAML::Key << "AngularDrag" << YAML::Value << m_AngularDrag;

		out << YAML::Key << "AutomaticCentreOfMass" << YAML::Value << m_AutomaticCentreOfMass;
		out << YAML::Key << "UseGravity" << YAML::Value << m_UseGravity;
		out << YAML::Key << "IsKinematic" << YAML::Value << m_IsKinematic;

		out << YAML::Key << "PositionConstraint" << YAML::Value << YAML::Flow
			<< YAML::BeginSeq
			<< m_PositionConstraint.x
			<< m_PositionConstraint.y
			<< m_PositionConstraint.z
			<< YAML::EndSeq;

		out << YAML::Key << "RotationConstraint" << YAML::Value << YAML::Flow
			<< YAML::BeginSeq
			<< m_RotationConstraint.x
			<< m_RotationConstraint.y
			<< m_RotationConstraint.z
			<< YAML::EndSeq;

		out << YAML::EndMap;
	}

	bool RigidbodyComponent::Deserialize(const YAML::Node data)
	{
		// Deserialize the Mass value
		if (data["Mass"]) {
			SetMass(data["Mass"].as<float>());
		}

		// Deserialize the Drag value
		if (data["Drag"]) {
			SetDrag(data["Drag"].as<float>());
		}

		// Deserialize the AngularDrag value
		if (data["AngularDrag"]) {
			SetAngularDrag(data["AngularDrag"].as<float>());
		}

		// Deserialize the AutomaticCentreOfMass value
		if (data["AutomaticCentreOfMass"]) {
			SetAutomaticCentreOfMass(data["AutomaticCentreOfMass"].as<bool>());
		}

		// Deserialize the UseGravity value
		if (data["UseGravity"]) {
			SetGravity(data["UseGravity"].as<bool>());
		}

		// Deserialize the IsKinematic value
		if (data["IsKinematic"]) {
			SetKinematic(data["IsKinematic"].as<bool>());
		}

		// Deserialize the PositionConstraint sequence
		if (data["PositionConstraint"] && data["PositionConstraint"].IsSequence()) {
			const YAML::Node& positionNode = data["PositionConstraint"];

			glm::bvec3 temp{};
			temp.x = positionNode[0].as<bool>();
			temp.y = positionNode[1].as<bool>();
			temp.z = positionNode[2].as<bool>();

			SetPositionConstraint(temp);
		}

		// Deserialize the RotationConstraint sequence
		if (data["RotationConstraint"] && data["RotationConstraint"].IsSequence()) {
			const YAML::Node& rotationNode = data["RotationConstraint"];

			glm::bvec3 temp{};
			temp.x = rotationNode[0].as<bool>();
			temp.y = rotationNode[1].as<bool>();
			temp.z = rotationNode[2].as<bool>();

			SetRotationConstraint(temp);
		}
		
		return true;
	}

}

