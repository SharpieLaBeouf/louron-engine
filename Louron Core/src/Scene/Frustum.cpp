#include "Frustum.h"

#include "Bounds.h"

namespace Louron {

	void Frustum::RecalculateFrustum(const glm::mat4& viewProjectionMatrix)
	{		// Left plane
		planes[0].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0];
		planes[0].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0];
		planes[0].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0];
		planes[0].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0];

		// Right plane
		planes[1].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0];
		planes[1].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0];
		planes[1].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0];
		planes[1].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0];

		// Bottom plane
		planes[2].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1];
		planes[2].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1];
		planes[2].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1];
		planes[2].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1];

		// Top plane
		planes[3].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1];
		planes[3].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1];
		planes[3].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1];
		planes[3].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1];

		// Near plane
		planes[4].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][2];
		planes[4].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][2];
		planes[4].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][2];
		planes[4].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][2];

		// Far plane
		planes[5].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2];
		planes[5].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2];
		planes[5].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2];
		planes[5].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2];

		// Normalize the planes
		for (int i = 0; i < 6; ++i) {
			planes[i].normalize();
		}
	}

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