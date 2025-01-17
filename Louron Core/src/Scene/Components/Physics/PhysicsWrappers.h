#pragma once

// Louron Core Headers
#include "../UUID.h"

// C++ Standard Library Headers
#include <map>
#include <memory>

// External Vendor Library Headers
#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>

using namespace physx;

namespace Louron {

	class PhysicsShape;
	class UUID;
	struct TransformComponent;

	/// <summary>
	/// Flags that are set to determine what state changes have occured
	/// each frame so the PhysicsSystem may process these changes.
	/// </summary>
	enum RigidbodyFlags : uint8_t {

		RigidbodyFlag_None				= 0,

		RigidbodyFlag_TransformUpdated	= 1U << 0,	// Only add this flag where there have been changes made to the entities transform manually
		RigidbodyFlag_ShapesUpdated		= 1U << 1,	// Only add this flag where there has been a shape change to the Rigidbody

	};
	
	class RigidDynamic {

		friend class PhysicsSystem;

	private:

		PxRigidDynamic* m_Actor = nullptr;
		std::map<std::weak_ptr<PhysicsShape>, UUID, std::owner_less<>> m_ShapesRef;

		RigidbodyFlags m_StateFlags = RigidbodyFlag_None;

	public:

		void Init(const PxTransform& transform);
		void Shutdown();

		RigidDynamic() = default;
		RigidDynamic(RigidDynamic&& other) noexcept;
		~RigidDynamic() = default;

		RigidDynamic(const RigidDynamic&) = delete;
		RigidDynamic& operator=(const RigidDynamic&) = delete;
		RigidDynamic& operator=(RigidDynamic&& other) noexcept;
		explicit operator bool() const;

		PxRigidDynamic* GetActor() const;
		void Release();

		bool AttachShape(std::shared_ptr<PhysicsShape> shape, const UUID& shapeEntityUUID);
		void DetachShape(std::shared_ptr<PhysicsShape> shape);
		bool IsShapeAttached(std::shared_ptr<PhysicsShape> shape);

		void WakeUp();
		void PutToSleep();

		// FLAGS
		void AddFlag(RigidbodyFlags flag);
		void RemoveFlag(RigidbodyFlags flag);
		bool CheckFlag(RigidbodyFlags flag) const;
		bool NoFlagsSet() const;
		void ClearFlags();
		RigidbodyFlags GetFlags() const;

		#pragma region Getters and Setters

		// Getter methods
		PxTransform GetGlobalPose() const;
		PxVec3 GetLinearVelocity() const;
		PxVec3 GetAngularVelocity() const;

		// Setter methods
		void SetGlobalPose(const PxTransform& pose);
		void SetGlobalPose(TransformComponent& transform);
		void SetGlobalPose(const glm::vec3& position, const glm::vec3& rotation);

		void SetKinematic(bool isKinematicEnabled);
		void SetKinematicTarget(const PxTransform& pose);

		void SetGravity(bool isGravityEnabled);

		void AddForce(const PxVec3& force, PxForceMode::Enum mode = PxForceMode::eFORCE);
		void AddTorque(const PxVec3& torque, PxForceMode::Enum mode = PxForceMode::eFORCE);
		void AddForceAtPosition(const PxVec3& force, const PxVec3& position, PxForceMode::Enum mode = PxForceMode::eFORCE);
		void AddForceAtLocalPosition(const PxVec3& force, const PxVec3& position, PxForceMode::Enum mode = PxForceMode::eFORCE);

		void SetLinearDamping(PxReal damping);
		void SetLinearVelocity(const glm::vec3& velocity);
		void SetMaxAngularVelocity(PxReal maxAngularVelocity);

		void SetAngularDamping(PxReal damping);
		void SetAngularVelocity(const PxVec3& velocity);
		void SetMaxDepenetrationVelocity(PxReal maxDepenetrationVelocity);

		void SetMass(PxReal mass);
		void SetMassSpaceInertiaTensor(const PxVec3& inertia);

		void SetCMassLocalPose(const PxTransform& pose);
		void SetRigidBodyFlag(PxRigidBodyFlag::Enum flag, bool value);
		void SetSolverIterationCounts(PxU32 minPositionIters, PxU32 minVelocityIters);
		void SetRigidDynamicLockFlag(PxRigidDynamicLockFlag::Enum flag, bool value);

		#pragma endregion

	};

	class PhysicsMaterial {

	public:

		void Init();
		void Shutdown();

		PhysicsMaterial() = default;
		PhysicsMaterial(float dynamicFriction, float staticFriction, float bounciness);
		PhysicsMaterial(const PhysicsMaterial& other);
		~PhysicsMaterial() = default;

		PxMaterial* GetMaterial() const;

		// GETTERS
		float GetDynamicFriction() const;
		float GetStaticFriction() const;
		float GetBounciness() const;

		// SETTERS
		void SetDynamicFriction(float dynamicFriction);
		void SetStaticFriction(float staticFriction);
		void SetBounciness(float bounciness);

	private:

		PxMaterial* m_Material = nullptr;
		float m_DynamicFriction = 0.6f;
		float m_StaticFriction = 0.6f;
		float m_Bounciness = 0.0f;
	};

	class PhysicsShape {

	private:

		PxShape* m_Shape = nullptr;

		std::shared_ptr<RigidDynamic> m_StaticBody = nullptr; // This is only used when there are no other Rigidbodies to attach to.
		std::weak_ptr<RigidDynamic> m_RigidbodyRef;

	public:

		PhysicsShape() = delete;
		PhysicsShape(const PxGeometry& geometry, const PxMaterial& material, bool isExclusive = false, PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE);
		PhysicsShape(PhysicsShape&& other) noexcept;
		~PhysicsShape();

		PhysicsShape(const PhysicsShape&) = delete;
		PhysicsShape& operator=(const PhysicsShape&) = delete;
		PhysicsShape& operator=(PhysicsShape&& other) noexcept;
		explicit operator bool() const;

		PxShape* GetShape() const;
		void Release();

		bool IsStatic() const;
		std::shared_ptr<RigidDynamic> GetRigidbody();

		#pragma region Getters and Setters

		// Getter methods
		PxGeometryType::Enum GetGeometryType() const;
		PxMaterial* GetMaterial() const;
		PxTransform GetLocalPose() const;
		PxFilterData GetSimulationFilterData() const;

		// Setter methods
		void SetLocalPose(const PxTransform& pose);
		void SetLocalPose(const glm::vec3& local_position, const glm::vec3& local_rotation);
		void SetLocalPose(TransformComponent& transform);
		
		void SetGeometry(const PxGeometry& geom);
		
		void SetSimulationFilterData(const PxFilterData& filterData);
		void SetFlag(PxShapeFlag::Enum flag, bool value);
		void SetFlags(PxShapeFlags flags);
		void SetMaterials(PxMaterial* const* materials, PxU32 materialCount);

		#pragma endregion

	private:

		friend struct SphereColliderComponent;
		friend struct BoxColliderComponent;
	};

}