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

	Rigidbody::Rigidbody(Transform* transform, PxScene* scene) : m_PhysScene(scene)
	{
		if (!&PxGetPhysics() || !m_PhysScene) {
			L_CORE_ERROR("PhysXPhysics or PhysXScene Not Initialized.");
			return;
		}

		if(!transform) {
			L_CORE_ERROR("Rigidbody Constructor - Transform Pointer Invalid.");
			return;
		}

		glm::vec3 position = transform->GetGlobalPosition();
		glm::quat quaternion = glm::quat(glm::radians(transform->GetGlobalRotation()));
		m_RigidDynamic = std::make_shared<RigidDynamic>(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

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

	Rigidbody::Rigidbody(const Rigidbody& other) {

		m_Mass = other.m_Mass;
		m_Drag = other.m_Drag;
		m_AngularDrag = other.m_AngularDrag;

		m_AutomaticCentreOfMass = other.m_AutomaticCentreOfMass;
		m_UseGravity = other.m_UseGravity;
		m_IsKinematic = other.m_IsKinematic;

		m_PositionConstraint = other.m_PositionConstraint;
		m_RotationConstraint = other.m_RotationConstraint;

		Entity entity = GetEntity();
		if (entity && entity.GetScene())
		{
			if (!m_RigidDynamic)
			{
				m_PhysScene = entity.GetScene()->GetPhysScene();
				auto& transform = entity.GetComponent<Transform>();
				glm::vec3 position = transform.GetGlobalPosition();
				glm::quat quaternion = glm::quat(glm::radians(transform.GetGlobalRotation()));
				m_RigidDynamic = std::make_shared<RigidDynamic>(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

				m_PhysScene->addActor(*m_RigidDynamic->GetActor());
			}

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
		else
		{
			m_PhysScene = nullptr;
			m_RigidDynamic = nullptr;
		}
	}

	Rigidbody& Rigidbody::operator=(const Rigidbody& other) {

		m_Mass = other.m_Mass;
		m_Drag = other.m_Drag;
		m_AngularDrag = other.m_AngularDrag;

		m_AutomaticCentreOfMass = other.m_AutomaticCentreOfMass;
		m_UseGravity = other.m_UseGravity;
		m_IsKinematic = other.m_IsKinematic;

		m_PositionConstraint = other.m_PositionConstraint;
		m_RotationConstraint = other.m_RotationConstraint;

		Entity entity = GetEntity();
		if (entity && entity.GetScene())
		{

			if(!m_RigidDynamic)
			{
				m_PhysScene = entity.GetScene()->GetPhysScene();
				auto& transform = entity.GetComponent<Transform>();
				glm::vec3 position = transform.GetGlobalPosition();
				glm::quat quaternion = glm::quat(glm::radians(transform.GetGlobalRotation()));
				m_RigidDynamic = std::make_shared<RigidDynamic>(PxTransform(position.x, position.y, position.z, PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w)));

				m_PhysScene->addActor(*m_RigidDynamic->GetActor());
			}

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
		else
		{
			m_PhysScene = nullptr;
			m_RigidDynamic = nullptr;
		}
		return *this;
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
			Entity entity = GetEntity();
			if (entity.GetScene()->IsPhysicsSimulating())
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
			Entity entity = GetEntity();
			if (entity.GetScene()->IsPhysicsSimulating())
				m_DeferredTorque.push_back({ torque });
			else
				m_RigidDynamic->AddTorque(PxVec3(torque.x, torque.y, torque.z));
		}
	}

	void Rigidbody::Serialize(YAML::Emitter& out)
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

	bool Rigidbody::Deserialize(const YAML::Node data)
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

			glm::vec3 temp{};
			temp.x = positionNode[0].as<float>();
			temp.y = positionNode[1].as<float>();
			temp.z = positionNode[2].as<float>();

			SetPositionConstraint(temp);
		}

		// Deserialize the RotationConstraint sequence
		if (data["RotationConstraint"] && data["RotationConstraint"].IsSequence()) {
			const YAML::Node& rotationNode = data["RotationConstraint"];

			glm::vec3 temp{};
			temp.x = rotationNode[0].as<float>();
			temp.y = rotationNode[1].as<float>();
			temp.z = rotationNode[2].as<float>();

			SetRotationConstraint(temp);
		}
		
		return true;
	}

}

