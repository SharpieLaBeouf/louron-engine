#pragma once

#include <array>
#include <vector>

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

		Frustum() = default;

		Frustum(const glm::mat4& view_proj_matrix) {
			RecalculateFrustum(view_proj_matrix);
		}

		std::array<Plane, 6> planes{};

		void RecalculateFrustum(const glm::mat4& view_proj_matrix);

		static std::vector<glm::vec4> GetWorldCorners(const glm::mat4& view_proj_matrix);

		static Bounds_AABB GetWorldSpaceTightBoundingBox(const glm::mat4& view_proj_matrix);
		static Bounds_AABB CalculateLightSpaceBoundingBox(const glm::mat4& view_proj_matrix, glm::mat4& light_view_matrix, const glm::vec3& light_direction);
		static std::array<glm::mat4, 5> CalculateCascadeLightSpaceMatrices(const glm::mat4& projection_matrix, const glm::mat4& view_matrix, const glm::vec3& light_direction, std::array<float, 5>& shadow_cascade_plane_distances);

		FrustumContainResult Contains(const Bounds_AABB& bounds) const;
	};

}