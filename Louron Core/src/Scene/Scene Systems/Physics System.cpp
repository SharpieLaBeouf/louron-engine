#include "Physics System.h"

// Louron Core Headers

#include "../Scene.h"
#include "../Entity.h"
#include "../Components/Physics/Rigidbody.h"
#include "../Components/Physics/Collider.h"

#include "../../Debug/Profiler.h"

#include "../../Core/Time.h"

// C++ Standard Library Headers


// External Vendor Library Headers
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

namespace Louron {

#pragma region HelperFunctions

	static void DetachSpecificShape(PxShape* shape, PxRigidActor* actor) {

		for (PxU32 i = 0; i < actor->getNbShapes(); i++) {
			PxShape* temp_shape = nullptr;
			actor->getShapes(&temp_shape, 1, i);
			if (temp_shape == shape)
				actor->detachShape(*shape);
		}
	}

	template<typename ColliderType>
	void ProcessColliderChanges(Entity& entity) {

		ColliderType& collider = entity.GetComponent<ColliderType>();

		// 1. Current Entity Has Rigidbody and Shape Does Not Already Refer to Rigidbody
		if (collider.CheckFlag(ColliderFlag_RigidbodyUpdated)) {

			if (entity.HasComponent<RigidbodyComponent>()) {

				// Remove Collider from Previous Rigidbody
				if (auto old_rb_ref = collider.GetShape()->GetRigidbody(); old_rb_ref && *old_rb_ref)
					old_rb_ref->DetachShape(collider.GetShape());

				// Update Colliders Reference to New Rigidbody
				collider.UpdateRigidbody(entity.GetUUID());

				// Attach the Collider to the New Rigidbody
				if (auto new_rb_ref = collider.GetShape()->GetRigidbody(); new_rb_ref && *new_rb_ref)
					new_rb_ref->AttachShape(collider.GetShape(), entity.GetUUID());

				collider.AddFlag(ColliderFlag_ShapePropsUpdated);
				collider.AddFlag(ColliderFlag_TransformUpdated);
				collider.ClearFlag(ColliderFlag_RigidbodyUpdated);
			}
		}

		// 2. Check Parent(s) for Rigidbody
		if (collider.CheckFlag(ColliderFlag_RigidbodyUpdated)) {

			if (auto rb_ref = collider.GetComponentInParent<RigidbodyComponent>(); rb_ref && rb_ref->GetActor() && *rb_ref->GetActor()) {

				// Remove Collider from Previous Rigidbody
				if (auto old_rb_ref = collider.GetShape()->GetRigidbody(); old_rb_ref && *old_rb_ref)
					old_rb_ref->DetachShape(collider.GetShape());

				collider.UpdateRigidbody(rb_ref->GetEntity().GetUUID());

				// Attach the Collider to the New Rigidbody
				if (auto new_rb_ref = collider.GetShape()->GetRigidbody(); new_rb_ref && *new_rb_ref)
					new_rb_ref->AttachShape(collider.GetShape(), entity.GetUUID());

				collider.AddFlag(ColliderFlag_ShapePropsUpdated);
				collider.AddFlag(ColliderFlag_TransformUpdated);
				collider.ClearFlag(ColliderFlag_RigidbodyUpdated);
			}

		}

		// 3. No Rigidbodies Found - Static Collider
		// 
		//    This is the fall through for if there is not a rigidbody attached to the 
		//	  current entity
		//
		if (collider.CheckFlag(ColliderFlag_RigidbodyUpdated)) {

			// Remove Collider from Previous Rigidbody
			if (auto rb_ref = collider.GetShape()->GetRigidbody(); rb_ref && *rb_ref)
				rb_ref->DetachShape(collider.GetShape());

			collider.CreateStaticRigidbody();

			collider.AddFlag(ColliderFlag_ShapePropsUpdated);
			collider.AddFlag(ColliderFlag_TransformUpdated);
		}

		// 4. Update Shape Transform and Shape Properties
		if (collider.CheckFlag(ColliderFlag_ShapePropsUpdated) || collider.CheckFlag(ColliderFlag_TransformUpdated)) {

			if (auto rb_ref = collider.GetShape()->GetRigidbody(); rb_ref && *rb_ref)
				rb_ref->AddFlag(RigidbodyFlag_TransformUpdated);

			collider.UpdateTransform(entity.GetComponent<TransformComponent>(), entity.GetScene()->FindEntityByUUID(collider.GetRigidbodyUUID()).GetComponent<TransformComponent>());
		}

		collider.ClearFlags();

	}

	template void ProcessColliderChanges<SphereColliderComponent>(Entity& entity);
	template void ProcessColliderChanges<BoxColliderComponent>(Entity& entity);

#pragma endregion

#pragma region RigidbodyComponent

	void PhysicsSystem::RemoveRigidBody(Entity entity, Scene* scene) {

		auto& component = entity.GetComponent<RigidbodyComponent>();

		if (!entity.HasComponent<RigidbodyComponent>()) {
			L_CORE_WARN("Entity Does Not Have Rigidbody - Cannot Remove Component That Isn't There.");
			return;
		}

		if (component.GetActor()) {

			// 1. Remove Shapes from Actor
			std::map<std::weak_ptr<PhysicsShape>, UUID, std::owner_less<>> shape_ref_vector = component.GetActor()->m_ShapesRef;
			for (auto& shape_weak_ref : shape_ref_vector) {

				if (auto shape_ref = shape_weak_ref.first.lock(); shape_ref) {

					Entity shape_entity = scene->FindEntityByUUID(shape_weak_ref.second);

					if (!shape_entity)
						continue;

					if (shape_entity.HasComponent<SphereColliderComponent>()) {

						shape_entity.GetComponent<SphereColliderComponent>().ResetRigidbody();
						shape_entity.GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
					}
					if (shape_entity.HasComponent<BoxColliderComponent>()) {
						shape_entity.GetComponent<BoxColliderComponent>().ResetRigidbody();
						shape_entity.GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
					}
				}
			}

			// 2. We also want to check if there are any other shapes that may be connected to this rigidbody to be removed
			if (PxU32 num_shapes = component.GetActor()->GetActor()->getNbShapes(); num_shapes > 0) {

				L_CORE_WARN("Shapes Still Attached to Rigibody Not Stored in Map.");
				for (PxU32 j = 0; j < num_shapes; j++) {
					PxShape* temp_shape = nullptr;
					component.GetActor()->GetActor()->getShapes(&temp_shape, 1, j);
					component.GetActor()->GetActor()->detachShape(*temp_shape);
				}

			}

			// 3. Remove Current Rigidbody Actor from Scene
			PxScene* physScene = scene->GetPhysScene();
			physScene->removeActor(*component.GetActor()->GetActor());

			// 4. Delete Actor
			entity.GetComponent<RigidbodyComponent>().GetActor()->Release();
			entity.GetComponent<RigidbodyComponent>().m_RigidDynamic = nullptr;

		}

		// 5. Remove Rigidbody Component from Entity
		scene->m_Registry.remove_if_exists<RigidbodyComponent>(entity);

		L_CORE_INFO("Rigidbody Has Been Successfully Removed from Entity ({0}:{1}).", entity.GetName(), entity.GetUUID());
	}

#pragma endregion

#pragma region Colliders

	SphereColliderComponent& PhysicsSystem::AddSphereCollider(Entity entity, Scene* scene) {

		SphereColliderComponent& collider = scene->m_Registry.emplace<SphereColliderComponent>(entity);

		collider.SetColliderUserData(entity.GetUUID());

		collider.AddFlag(ColliderFlag_TransformUpdated);
		collider.AddFlag(ColliderFlag_RigidbodyUpdated);
		collider.AddFlag(ColliderFlag_ShapePropsUpdated);

		return collider;
		
	}

	BoxColliderComponent& PhysicsSystem::AddBoxCollider(Entity entity, Scene* scene) {

		BoxColliderComponent& collider = scene->m_Registry.emplace<BoxColliderComponent>(entity);

		collider.SetColliderUserData(entity.GetUUID());

		collider.AddFlag(ColliderFlag_TransformUpdated);
		collider.AddFlag(ColliderFlag_RigidbodyUpdated);
		collider.AddFlag(ColliderFlag_ShapePropsUpdated);

		return collider;

	}

	void PhysicsSystem::RemoveCollider(Entity entity, Scene* scene, PxGeometryType::Enum colliderType) {

		if (colliderType == PxGeometryType::eSPHERE) {

			if (!entity.HasComponent<SphereColliderComponent>()) {
				L_CORE_WARN("Entity Does Not Have Sphere Collider - Cannot Remove Component That Isn't There.");
				return;
			}

			SphereColliderComponent& collider = entity.GetComponent<SphereColliderComponent>();

			if(collider.GetShape()) {
				if (auto rb_ref = collider.GetShape()->GetRigidbody(); rb_ref && !collider.GetShape()->IsStatic()) {

					rb_ref->AddFlag(RigidbodyFlag_ShapesUpdated);

					rb_ref->DetachShape(collider.GetShape());
					collider.ResetRigidbody();
				}
				else if (rb_ref && collider.GetShape()->IsStatic()) {

					scene->GetPhysScene()->removeActor(*collider.GetShape()->GetRigidbody()->GetActor());
					collider.ResetRigidbody();
				}
			}

			collider.Release();
			scene->m_Registry.remove_if_exists<SphereColliderComponent>(entity);

			L_CORE_INFO("Sphere Collider Has Been Successfully Removed from Entity ({0}:{1}).", entity.GetName(), entity.GetUUID());
		}

		if (colliderType == PxGeometryType::eBOX) {

			if (!entity.HasComponent<BoxColliderComponent>()) {
				L_CORE_WARN("Entity Does Not Have Box Collider - Cannot Remove Component That Isn't There.");
				return;
			}

			BoxColliderComponent& collider = entity.GetComponent<BoxColliderComponent>();

			if (collider.GetShape()) {
				if (auto rb_ref = collider.GetShape()->GetRigidbody(); rb_ref && !collider.GetShape()->IsStatic()) {

					rb_ref->AddFlag(RigidbodyFlag_ShapesUpdated);

					rb_ref->DetachShape(collider.GetShape());
					collider.ResetRigidbody();
				}
				else if (rb_ref && collider.GetShape()->IsStatic()) {

					scene->GetPhysScene()->removeActor(*collider.GetShape()->GetRigidbody()->GetActor());
					collider.ResetRigidbody();
				}
			}

			collider.Release();
			scene->m_Registry.remove_if_exists<BoxColliderComponent>(entity);

			L_CORE_INFO("Box Collider Has Been Successfully Removed from Entity ({0}:{1}).", entity.GetName(), entity.GetUUID());
		}
	}

#pragma endregion

#pragma region Update Logic

	void PhysicsSystem::Update(std::shared_ptr<Scene> scene) {

		PxScene* physxScene = scene->GetPhysScene();
		
		if (!scene->m_IsPaused) {

			// Process all Deferred forces from last update
			auto view = scene->GetAllEntitiesWith<RigidbodyComponent>();
			for (auto& entity_handle : view) {
				auto& rb_ref = view.get<RigidbodyComponent>(entity_handle);
				if(rb_ref.GetActor()) {
					if (!rb_ref.m_DeferredForce.empty()) {
						for (const auto& action : rb_ref.m_DeferredForce) {
							PxVec3 force = { action.force.x, action.force.y, action.force.z };
							rb_ref.GetActor()->AddForce(force, action.forceMode);
						}
						rb_ref.m_DeferredForce.clear();
					}
					if (!rb_ref.m_DeferredTorque.empty()) {
						for (const auto& action : rb_ref.m_DeferredTorque) {
							PxVec3 torque = { action.torque.x, action.torque.y, action.torque.z };
							rb_ref.GetActor()->AddTorque(torque);
						}
						rb_ref.m_DeferredTorque.clear();
					}
				}
			}
									
			physxScene->simulate(Time::GetUnscaledFixedDeltaTime());
			physxScene->fetchResults(true);

			UpdateSimulationChanges(scene);
		}
	}


	void PhysicsSystem::UpdatePhysicsObjects(std::shared_ptr<Scene> scene) {
		
		// 1. Update Sphere Colliders
		auto sc_view = scene->GetAllEntitiesWith<SphereColliderComponent>();
		if (sc_view.begin() != sc_view.end()) {
			for (const auto& entity_handle : sc_view) {

				auto& collider = sc_view.get<SphereColliderComponent>(entity_handle);

				if (!collider.NoFlagsSet()) {
					Entity entity = { entity_handle, scene.get()};

					ProcessColliderChanges<SphereColliderComponent>(entity);
				}

			}

		}

		// 2. Update Box Colliders
		auto bc_view = scene->GetAllEntitiesWith<BoxColliderComponent>();
		if (bc_view.begin() != bc_view.end()) {
			for (const auto& entity_handle : bc_view) {

				auto& collider = bc_view.get<BoxColliderComponent>(entity_handle);

				if (!collider.NoFlagsSet()) {
					Entity entity = { entity_handle, scene.get()};

					ProcessColliderChanges<BoxColliderComponent>(entity);
				}

			}

		}

		auto rb_view = scene->GetAllEntitiesWith<RigidbodyComponent>();
		if (rb_view.begin() != rb_view.end()) {
			for (const auto& entity_handle : rb_view) {

				auto& rigidbody = rb_view.get<RigidbodyComponent>(entity_handle);

				// 1. Current Entity Has Rigidbody and Shape Does Not Already Refer to Rigidbody
				if(rigidbody.GetActor())
				{
					if (rigidbody.GetActor()->CheckFlag(RigidbodyFlag_ShapesUpdated)) {
						PxRigidBodyExt::setMassAndUpdateInertia(*rigidbody.GetActor()->GetActor(), rigidbody.GetMass());
					}

					if (rigidbody.GetActor()->CheckFlag(RigidbodyFlag_TransformUpdated)) {
						rigidbody.GetActor()->SetGlobalPose(rigidbody.GetEntity().GetComponent<TransformComponent>());
					}

					rigidbody.GetActor()->ClearFlags();
				}
			}
		}
	}

	void PhysicsSystem::UpdateSimulationChanges(std::shared_ptr<Scene> scene) {

		// 1. Update Rigidbodies
		// When we have hierarchy relationships of objects that have physics components,
		// the ordering of updating the GlobalPosition and GlobalRotation MATTERS! We 
		// need to update the global positions of our entities root down. 
		// 
		// If we don't, sometimes child entities may set the global transform first 
		// based off the parents OLD globaltransform, then when the parent updates
		// the child's global position is then recalculated - THIS IS BAD!
		// 
		// Whoever other than myself who may be reading this, I was stuck on this for the
		// better half of an entire day... This was honestly just a stab in the dark fix... 
		//
		// --------------------------------------------------------------------------------
		//					alas, the hail mary thus brought fruit!
		// --------------------------------------------------------------------------------

		auto root_view = scene->GetAllEntitiesWith<HierarchyComponent>();
		std::vector<Entity> root_entities;

		if (root_view.begin() != root_view.end()) {

			for (auto& entity_handle : root_view) {
				if (!root_view.get<HierarchyComponent>(entity_handle).HasParent()) {
					root_entities.push_back(root_view.get<HierarchyComponent>(entity_handle).GetEntity());
				}
			}
		}

		std::function<void(Entity)> update_physics = [&](Entity start_entity) -> void {
			
			if(start_entity && start_entity.HasComponent<RigidbodyComponent>()) {

				auto& transform = start_entity.GetComponent<TransformComponent>();
				auto& rigidbody = start_entity.GetComponent<RigidbodyComponent>();

				if(rigidbody.GetActor()) {
					if (!rigidbody.GetActor()->CheckFlag(RigidbodyFlag_TransformUpdated)) // Only update if we haven't manually updated
					{
						PxTransform physics_transform = rigidbody.GetActor()->GetGlobalPose();
						glm::quat quaternion(physics_transform.q.w, physics_transform.q.x, physics_transform.q.y, physics_transform.q.z);
						glm::vec3 rotation = glm::degrees(glm::eulerAngles(quaternion));

						transform.SetGlobalPosition(glm::vec3(physics_transform.p.x, physics_transform.p.y, physics_transform.p.z));
						transform.SetGlobalRotation(rotation);
					}
				}
			}
			for (auto& child_uuid : start_entity.GetComponent<HierarchyComponent>().GetChildren()) {

				update_physics(scene->FindEntityByUUID(child_uuid));

			}
			
		};

		for (auto& entity : root_entities) {

			update_physics(entity);

		}
	}

#pragma endregion

}