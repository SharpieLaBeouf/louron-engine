#pragma once

#include <PxPhysicsAPI.h>
#include <memory>

#include <unordered_set>

namespace Louron {

	using namespace physx;

	class Scene;

	namespace CollisionUtils {
		struct pair_hash {
			template <class T1, class T2>
			std::size_t operator()(const std::pair<T1, T2>& pair) const {
				return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
			}
		};
	}

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

		inline static std::unordered_set<std::pair<uint32_t, uint32_t>, CollisionUtils::pair_hash> s_ActiveTriggers;

	private:
		std::weak_ptr<Scene> m_Scene;

	};


}