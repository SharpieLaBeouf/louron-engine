#include "CollisionCallback.h"

#include "../../Scene.h"
#include "../../Entity.h"

namespace Louron {

	void CollisionCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) {
		if (auto scene_ref = m_Scene.lock(); scene_ref && scene_ref->IsRunning()) {
			for (PxU32 i = 0; i < nbPairs; i++) {
				const PxContactPair& cp = pairs[i];
				if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) {

					// Retrieve the entities associated with the shapes
					// should i use cp.shapes or pairheader.pairs->shapes?
					if (!cp.shapes[0]->userData || !cp.shapes[1]->userData) {
						L_CORE_ERROR("Collision Callback - Collider Shapes Have Invalid UserData.");
						return;
					}
						
					std::array<Entity, 2> entities{
						scene_ref->FindEntityByUUID(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cp.shapes[0]->userData))),
						scene_ref->FindEntityByUUID(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cp.shapes[1]->userData)))
					};

					if (entities[0] && entities[1]) {

						for (int i = 0; i < entities.size(); i++) {
							int index_this = (i == 0) ? 0 : 1;
							int index_other = (i == 0) ? 1 : 0;

							if (entities[i].HasComponent<SphereCollider>()) {
								if (entities[i].GetComponent<SphereCollider>().OnCollideCallback) {
									entities[i].GetComponent<SphereCollider>().OnCollideCallback(entities[index_this], entities[index_other]);
								}
							}

							if (entities[i].HasComponent<BoxCollider>()) {
								if (entities[i].GetComponent<BoxCollider>().OnCollideCallback) {
									entities[i].GetComponent<BoxCollider>().OnCollideCallback(entities[index_this], entities[index_other]);
								}
							}
						}
					}
					else {
						L_CORE_ERROR("Collision Callback - Collision Detected With Entity Not Found In Scene.");
					}
				}
			}
		}
	}

	// TODO: IMPLEMENT CALLBACKS
	void CollisionCallback::onTrigger(PxTriggerPair*, PxU32) {

	}

	void CollisionCallback::onConstraintBreak(PxConstraintInfo*, PxU32)
	{
	}

	void CollisionCallback::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
	{
	}

	void CollisionCallback::onWake(PxActor**, PxU32)
	{
	}

	void CollisionCallback::onSleep(PxActor**, PxU32)
	{
	}

}