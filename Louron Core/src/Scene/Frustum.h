#pragma once

#include <array>

#include <glm/glm.hpp>

namespace Louron {

	struct Bounds_AABB;

	struct Plane {
		glm::vec3 normal;
		float distance;

		// Normalize the plane
		void normalize() {
			float length = glm::length(normal);
			normal /= length;
			distance /= length;
		}

		float DistanceToPoint(const glm::vec3& point) const {
			return glm::dot(normal, point) + distance;
		}
	};

	enum class FrustumContainResult {
		DoesNotContain,
		Intersects,
		Contains
	};

	struct Frustum {
		std::array<Plane, 6> planes;

		void RecalculateFrustum(const glm::mat4& viewProjectionMatrix);

		FrustumContainResult Contains(const Bounds_AABB& bounds) const;
	};

}