#include "Transform System.h"

#include "../Scene.h"
#include "../Entity.h"

#include "../Components/Physics/Collider.h"
#include "../Components/Physics/Rigidbody.h"

#include "../../Debug/Profiler.h"

namespace Louron {

	void TransformSystem::Update(std::shared_ptr<Scene> scene) {

		L_PROFILE_SCOPE("Transform Update");

		auto view = scene->GetAllEntitiesWith<Transform>();
		for (auto& entityHandle : view) {

			Transform& transform = view.get<Transform>(entityHandle);

			if (transform.CheckFlag(TransformFlag_PositionUpdated) ||
				transform.CheckFlag(TransformFlag_RotationUpdated) ||
				transform.CheckFlag(TransformFlag_ScaleUpdated)) {

				transform.m_Transform =
					glm::translate(glm::mat4(1.0f), transform.m_Position) *
					glm::toMat4(glm::quat(glm::radians(transform.m_Rotation))) *
					glm::scale(glm::mat4(1.0f), transform.m_Scale);

				if (transform.entity->HasAnyComponent<Rigidbody, SphereCollider, BoxCollider>()) {

					if (transform.entity->HasComponent<Rigidbody>())
						transform.entity->GetComponent<Rigidbody>().GetActor()->AddFlag(RigidbodyFlag_TransformUpdated);

					if (transform.entity->HasComponent<SphereCollider>())
						transform.entity->GetComponent<SphereCollider>().GetShape()->AddFlag(ColliderFlag_TransformUpdated);

					if (transform.entity->HasComponent<BoxCollider>())
						transform.entity->GetComponent<BoxCollider>().GetShape()->AddFlag(ColliderFlag_TransformUpdated);

				}
			}
			transform.ClearFlags();

		}

	}

}