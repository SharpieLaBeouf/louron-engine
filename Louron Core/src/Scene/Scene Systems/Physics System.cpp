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
		if (collider.GetShape()->CheckFlag(ColliderFlag_RigidbodyUpdated)) {

			if (entity.HasComponent<Rigidbody>()) {

				// Remove Collider from Previous Rigidbody
				if (auto old_rb_ref = collider.GetShape()->GetRigidbody(); old_rb_ref && *old_rb_ref)
					old_rb_ref->DetachShape(collider.GetShape());

				// Update Colliders Reference to New Rigidbody
				collider.UpdateRigidbody(entity.GetUUID());

				// Attach the Collider to the New Rigidbody
				if (auto new_rb_ref = collider.GetShape()->GetRigidbody(); new_rb_ref && *new_rb_ref)
					new_rb_ref->AttachShape(collider.GetShape(), entity.GetUUID());

				collider.GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);
				collider.GetShape()->AddFlag(ColliderFlag_TransformUpdated);
				collider.GetShape()->ClearFlag(ColliderFlag_RigidbodyUpdated);
			}
		}

		// 2. Check Parent(s) for Rigidbody
		if (collider.GetShape()->CheckFlag(ColliderFlag_RigidbodyUpdated)) {

			if (auto rb_ref = collider.GetComponentInParent<Rigidbody>(); rb_ref && rb_ref->GetActor() && *rb_ref->GetActor()) {

				// Remove Collider from Previous Rigidbody
				if (auto old_rb_ref = collider.GetShape()->GetRigidbody(); old_rb_ref && *old_rb_ref)
					old_rb_ref->DetachShape(collider.GetShape());

				collider.UpdateRigidbody(rb_ref->entity->GetUUID());

				// Attach the Collider to the New Rigidbody
				if (auto new_rb_ref = collider.GetShape()->GetRigidbody(); new_rb_ref && *new_rb_ref)
					new_rb_ref->AttachShape(collider.GetShape(), entity.GetUUID());

				collider.GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);
				collider.GetShape()->AddFlag(ColliderFlag_TransformUpdated);
				collider.GetShape()->ClearFlag(ColliderFlag_RigidbodyUpdated);
			}

		}

		// 3. No Rigidbodies Found - Static Collider
		// 
		//    This is the fall through for if there is not a rigidbody attached to the 
		//	  current entity
		//
		if (collider.GetShape()->CheckFlag(ColliderFlag_RigidbodyUpdated)) {

			// Remove Collider from Previous Rigidbody
			if (auto rb_ref = collider.GetShape()->GetRigidbody(); rb_ref && *rb_ref)
				rb_ref->DetachShape(collider.GetShape());

			collider.CreateStaticRigidbody();

			collider.GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);
			collider.GetShape()->AddFlag(ColliderFlag_TransformUpdated);
		}

		// 4. Update Shape Transform and Shape Properties
		if (collider.GetShape()->CheckFlag(ColliderFlag_ShapePropsUpdated) || collider.GetShape()->CheckFlag(ColliderFlag_TransformUpdated)) {

			if (auto rb_ref = collider.GetShape()->GetRigidbody(); rb_ref && *rb_ref)
				rb_ref->AddFlag(RigidbodyFlag_TransformUpdated);

			collider.UpdateTransform(entity.GetComponent<Transform>(), entity.GetScene()->FindEntityByUUID(collider.GetRigidbodyUUID()).GetComponent<Transform>());
		}

		collider.GetShape()->ClearFlags();

	}

	template void ProcessColliderChanges<SphereCollider>(Entity& entity);
	template void ProcessColliderChanges<BoxCollider>(Entity& entity);

#pragma endregion

#pragma region Rigidbody

	Rigidbody& PhysicsSystem::AddRigidBody(Entity entity, Scene* scene) {

		// We want to first update all flags of colliders within this entity 
		// and it's children that do not have a rigidbody. If there is a child 
		// of this entity that has a rigidbody, we won't update their children 
		// as their children will refer to that rigidbody and that would be 
		// unchanged for them
		{
			std::function<void(Entity)> update_child_collider_flags =

			[&](Entity start_entity) -> void {

				if (!start_entity.HasComponent<Rigidbody>()) {

					// Check if current entity has a collider component, so we 
					// can link the collider to this rigidbody.
					if (start_entity.HasComponent<SphereCollider>()) {

						auto& collider = entity.GetComponent<SphereCollider>();

						collider.GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
						collider.GetShape()->AddFlag(ColliderFlag_TransformUpdated);
						collider.GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);
					}
					if (start_entity.HasComponent<BoxCollider>()) {

						auto& collider = entity.GetComponent<BoxCollider>();

						collider.GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
						collider.GetShape()->AddFlag(ColliderFlag_TransformUpdated);
						collider.GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);
					}

					if (start_entity.HasComponent<HierarchyComponent>()) {

						for (const auto& child_uuid : start_entity.GetComponent<HierarchyComponent>().GetChildren()) {

							update_child_collider_flags(start_entity.GetScene()->FindEntityByUUID(child_uuid));
						}

					}

				}

			};

			update_child_collider_flags(entity);
		}

		Rigidbody& component = scene->m_Registry.emplace<Rigidbody>(entity, entity.GetComponent<Transform>(), scene->GetPhysScene());

		component.GetActor()->AddFlag(RigidbodyFlag_ShapesUpdated);
		component.GetActor()->AddFlag(RigidbodyFlag_TransformUpdated);

		return component;
	}

	void PhysicsSystem::RemoveRigidBody(Entity entity, Scene* scene) {

		auto& component = entity.GetComponent<Rigidbody>();

		if (!entity.HasComponent<Rigidbody>()) {
			L_CORE_WARN("Entity Does Not Have Rigidbody - Cannot Remove Component That Isn't There.");
			return;
		}

		// 1. Remove Shapes from Actor
		std::map<std::weak_ptr<PhysicsShape>, UUID, std::owner_less<>> shape_ref_vector = component.GetActor()->m_ShapesRef;
		for (auto& shape_weak_ref : shape_ref_vector) {

			if (auto shape_ref = shape_weak_ref.first.lock(); shape_ref) {

				Entity shape_entity = scene->FindEntityByUUID(shape_weak_ref.second);

				if (!shape_entity)
					continue;

				if (shape_entity.HasComponent<SphereCollider>()) {

					shape_entity.GetComponent<SphereCollider>().ResetRigidbody();
					shape_entity.GetComponent<SphereCollider>().GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
				}
				if (shape_entity.HasComponent<BoxCollider>()) {

					shape_entity.GetComponent<BoxCollider>().ResetRigidbody();
					shape_entity.GetComponent<BoxCollider>().GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
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
		entity.GetComponent<Rigidbody>().GetActor()->Release();
		entity.GetComponent<Rigidbody>().m_RigidDynamic = nullptr;

		// 5. Remove Rigidbody Component from Entity
		scene->m_Registry.remove_if_exists<Rigidbody>(entity);

		L_CORE_INFO("Rigidbody Has Been Successfully Removed from Entity ({0}:{1}).", entity.GetName(), entity.GetUUID());
	}

#pragma endregion

#pragma region Colliders

	SphereCollider& PhysicsSystem::AddSphereCollider(Entity entity, Scene* scene) {

		SphereCollider& collider = scene->m_Registry.emplace<SphereCollider>(entity);

		collider.SetColliderUserData(entity.GetUUID());

		collider.GetShape()->AddFlag(ColliderFlag_TransformUpdated);
		collider.GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
		collider.GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);

		return collider;
		
	}

	BoxCollider& PhysicsSystem::AddBoxCollider(Entity entity, Scene* scene) {

		BoxCollider& collider = scene->m_Registry.emplace<BoxCollider>(entity);

		collider.SetColliderUserData(entity.GetUUID());

		collider.GetShape()->AddFlag(ColliderFlag_TransformUpdated);
		collider.GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
		collider.GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);

		return collider;

	}

	void PhysicsSystem::RemoveCollider(Entity entity, Scene* scene, PxGeometryType::Enum colliderType) {

		if (colliderType == PxGeometryType::eSPHERE) {

			if (!entity.HasComponent<SphereCollider>()) {
				L_CORE_WARN("Entity Does Not Have Sphere Collider - Cannot Remove Component That Isn't There.");
				return;
			}

			SphereCollider& collider = entity.GetComponent<SphereCollider>();

			if (auto rb_ref = collider.GetShape()->GetRigidbody(); rb_ref && !collider.GetShape()->IsStatic()) {

				rb_ref->AddFlag(RigidbodyFlag_ShapesUpdated);

				rb_ref->DetachShape(collider.GetShape());
				collider.ResetRigidbody();
			}
			else if (rb_ref && collider.GetShape()->IsStatic()) {

				scene->GetPhysScene()->removeActor(*collider.GetShape()->GetRigidbody()->GetActor());
				collider.ResetRigidbody();
			}

			collider.Release();
			scene->m_Registry.remove_if_exists<SphereCollider>(entity);

			L_CORE_INFO("Sphere Collider Has Been Successfully Removed from Entity ({0}:{1}).", entity.GetName(), entity.GetUUID());
		}

		if (colliderType == PxGeometryType::eBOX) {

			if (!entity.HasComponent<BoxCollider>()) {
				L_CORE_WARN("Entity Does Not Have Box Collider - Cannot Remove Component That Isn't There.");
				return;
			}

			BoxCollider& collider = entity.GetComponent<BoxCollider>();

			if (auto rb_ref = collider.GetShape()->GetRigidbody(); rb_ref && !collider.GetShape()->IsStatic()) {

				rb_ref->AddFlag(RigidbodyFlag_ShapesUpdated);

				rb_ref->DetachShape(collider.GetShape());
				collider.ResetRigidbody();
			}
			else if (rb_ref && collider.GetShape()->IsStatic()) {

				scene->GetPhysScene()->removeActor(*collider.GetShape()->GetRigidbody()->GetActor());
				collider.ResetRigidbody();
			}

			collider.Release();
			scene->m_Registry.remove_if_exists<BoxCollider>(entity);

			L_CORE_INFO("Box Collider Has Been Successfully Removed from Entity ({0}:{1}).", entity.GetName(), entity.GetUUID());
		}
	}

#pragma endregion

#pragma region Update Logic

	void PhysicsSystem::Update(std::shared_ptr<Scene> scene) {

		PxScene* physxScene = scene->GetPhysScene();
		
		if (!scene->m_IsPaused) {

			// Process all Deferred forces from last update
			auto view = scene->GetAllEntitiesWith<Rigidbody>();
			for (auto& entity_handle : view) {
				auto& rb_ref = view.get<Rigidbody>(entity_handle);
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
									
			physxScene->simulate(Time::GetUnscaledFixedDeltaTime());
			physxScene->fetchResults(true);

			UpdateSimulationChanges(scene);
		}
	}


	void PhysicsSystem::UpdatePhysicsObjects(std::shared_ptr<Scene> scene) {
		
		// 1. Update Sphere Colliders
		auto sc_view = scene->GetAllEntitiesWith<SphereCollider>();
		if (sc_view.begin() != sc_view.end()) {
			for (const auto& entity_handle : sc_view) {

				auto& collider = sc_view.get<SphereCollider>(entity_handle);

				if (!collider.GetShape()->NoFlagsSet()) {
					Entity entity = { entity_handle, scene.get() };

					ProcessColliderChanges<SphereCollider>(entity);
				}

			}

		}

		// 2. Update Box Colliders
		auto bc_view = scene->GetAllEntitiesWith<BoxCollider>();
		if (bc_view.begin() != bc_view.end()) {
			for (const auto& entity_handle : bc_view) {

				auto& collider = bc_view.get<BoxCollider>(entity_handle);

				if (!collider.GetShape()->NoFlagsSet()) {
					Entity entity = { entity_handle, scene.get() };

					ProcessColliderChanges<BoxCollider>(entity);
				}

			}

		}

		auto rb_view = scene->GetAllEntitiesWith<Rigidbody>();
		if (rb_view.begin() != rb_view.end()) {
			for (const auto& entity_handle : rb_view) {

				auto& rigidbody = rb_view.get<Rigidbody>(entity_handle);

				// 1. Current Entity Has Rigidbody and Shape Does Not Already Refer to Rigidbody
				if (rigidbody.GetActor()->CheckFlag(RigidbodyFlag_ShapesUpdated)) {
					PxRigidBodyExt::setMassAndUpdateInertia(*rigidbody.GetActor()->GetActor(), 1.0f);
				}

				if (rigidbody.GetActor()->CheckFlag(RigidbodyFlag_TransformUpdated)) {
					rigidbody.GetActor()->SetGlobalPose(rigidbody.entity->GetComponent<Transform>());
				}

				rigidbody.GetActor()->ClearFlags();
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
					root_entities.push_back(*root_view.get<HierarchyComponent>(entity_handle).entity);
				}
			}
		}

		std::function<void(Entity)> update_physics = [&](Entity start_entity) -> void {
			
			if(start_entity && start_entity.HasComponent<Rigidbody>()) {

				auto& transform = start_entity.GetComponent<Transform>();
				auto& rigidbody = start_entity.GetComponent<Rigidbody>();

				PxTransform physics_transform = rigidbody.GetActor()->GetGlobalPose();
				glm::quat quaternion(physics_transform.q.w, physics_transform.q.x, physics_transform.q.y, physics_transform.q.z);
				glm::vec3 rotation = glm::degrees(glm::eulerAngles(quaternion));

				transform.SetGlobalPosition(glm::vec3(physics_transform.p.x, physics_transform.p.y, physics_transform.p.z));
				transform.SetGlobalRotation(rotation);

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