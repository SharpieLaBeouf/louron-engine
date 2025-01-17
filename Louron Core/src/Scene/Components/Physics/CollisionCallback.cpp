#include "CollisionCallback.h"

#include "../../Scene.h"
#include "../../Entity.h"

#include "../../../Scripting/Script Manager.h"

namespace Louron {

	void CollisionCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) {

		if (auto scene_ref = m_Scene.lock(); scene_ref && scene_ref->IsRunning()) {
			for (PxU32 i = 0; i < nbPairs; i++) {
				const PxContactPair& cp = pairs[i];

				// Retrieve the entities associated with the shapes
				if (!cp.shapes[0]->userData || !cp.shapes[1]->userData) {
					L_CORE_ERROR("Collision Callback - Collider Shapes Have Invalid UserData.");
					continue;
				}
						
				std::array<Entity, 2> entities{
					scene_ref->FindEntityByUUID(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cp.shapes[0]->userData))),
					scene_ref->FindEntityByUUID(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cp.shapes[1]->userData)))
				};

				if (entities[0] && entities[1]) {

					for (int i = 0; i < entities.size(); i++) {
						int index_this = (i == 0) ? 0 : 1;
						int index_other = (i == 0) ? 1 : 0;

						if (entities[i].HasComponent<ScriptComponent>()) {
							if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
								ScriptManager::OnCollideEntity(entities[index_this], entities[index_other], _Collision_Type::CollideEnter);
							}
							if (cp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {
								ScriptManager::OnCollideEntity(entities[index_this], entities[index_other], _Collision_Type::CollideStay);
							}
							if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST) {
								ScriptManager::OnCollideEntity(entities[index_this], entities[index_other], _Collision_Type::CollideLeave);
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

	void CollisionCallback::onTrigger(PxTriggerPair* pairs, PxU32 count) {

		if (auto scene_ref = m_Scene.lock(); scene_ref && scene_ref->IsRunning()) {


			for (PxU32 i = 0; i < count; i++) {
				const PxTriggerPair& tp = pairs[i];

				// Retrieve the entities associated with the trigger and actor shapes
				if (!tp.triggerShape->userData || !tp.otherShape->userData) {
					L_CORE_ERROR("Trigger Callback - Collider Shapes Have Invalid UserData.");
					continue;
				}

				uint32_t triggerUUID = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tp.triggerShape->userData));
				uint32_t otherUUID = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tp.otherShape->userData));

				Entity triggerEntity = scene_ref->FindEntityByUUID(triggerUUID);
				Entity otherEntity = scene_ref->FindEntityByUUID(otherUUID);

				if (triggerEntity && otherEntity) {
					std::pair<uint32_t, uint32_t> triggerPair = { triggerUUID, otherUUID };

					if (triggerEntity.HasComponent<ScriptComponent>()) {
						if (tp.status & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
							ScriptManager::OnCollideEntity(triggerEntity, otherEntity, _Collision_Type::TriggerEnter);
							s_ActiveTriggers.insert(triggerPair);
						}

						if (tp.status & PxPairFlag::eNOTIFY_TOUCH_LOST) {
							ScriptManager::OnCollideEntity(triggerEntity, otherEntity, _Collision_Type::TriggerLeave);
							s_ActiveTriggers.erase(triggerPair);
						}
					}
				}
				else {
					L_CORE_ERROR("Collision Callback - Collision Detected With Entity Not Found In Scene.");
				}

			}
		}
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