#pragma once

#include <PxPhysicsAPI.h>
#include <memory>

namespace Louron {

	using namespace physx;

	class Scene;

	class CollisionCallback : public PxSimulationEventCallback {

	public:

		CollisionCallback() = default;
		CollisionCallback(std::shared_ptr<Scene> scene): m_Scene(scene) {}

		void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override;
		void onTrigger(PxTriggerPair*, PxU32) override;
		void onConstraintBreak(PxConstraintInfo*, PxU32) override;
		void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override;
		void onWake(PxActor**, PxU32) override;
		void onSleep(PxActor**, PxU32) override;

	private:
		std::weak_ptr<Scene> m_Scene;

	};


}