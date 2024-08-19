#include "Frustum.h"

#include "Bounds.h"

namespace Louron {

	FrustumContainResult Frustum::Contains(const Bounds_AABB& bounds) const {

        bool allPointsInside = true;
        bool anyPointInside = false;

        for (const auto& plane : planes) {
            glm::vec3 positiveVertex = bounds.BoundsMin;
            glm::vec3 negativeVertex = bounds.BoundsMax;

            if (plane.normal.x >= 0) {
                positiveVertex.x = bounds.BoundsMax.x;
                negativeVertex.x = bounds.BoundsMin.x;
            }
            if (plane.normal.y >= 0) {
                positiveVertex.y = bounds.BoundsMax.y;
                negativeVertex.y = bounds.BoundsMin.y;
            }
            if (plane.normal.z >= 0) {
                positiveVertex.z = bounds.BoundsMax.z;
                negativeVertex.z = bounds.BoundsMin.z;
            }

            float distanceToPositiveVertex = glm::dot(plane.normal, positiveVertex) + plane.distance;
            float distanceToNegativeVertex = glm::dot(plane.normal, negativeVertex) + plane.distance;

            if (distanceToPositiveVertex < 0 && distanceToNegativeVertex < 0) {
                return FrustumContainResult::DoesNotContain;
            }

            if (distanceToPositiveVertex >= 0 || distanceToNegativeVertex >= 0) {
                anyPointInside = true;
            }

            if (!(distanceToPositiveVertex >= 0 && distanceToNegativeVertex >= 0)) {
                allPointsInside = false;
            }
        }

        if (allPointsInside) {
            return FrustumContainResult::Contains;
        }

        if (anyPointInside) {
            return FrustumContainResult::Intersects;
        }

        return FrustumContainResult::DoesNotContain;
	}

}