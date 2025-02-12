#include "Bounds.h"

#include <glm/gtx/norm.hpp>

namespace Louron {

#pragma region Sphere

	BoundsContainResult Bounds_Sphere::Contains(const Bounds_Sphere& other, float looseness) const {

		// Calculate the squared distance between the centers of the two spheres
		float distanceSquared = glm::length2(other.BoundsCentre - BoundsCentre);

		// Calculate the sum of the radii
		float radiiSum = BoundsRadius * looseness + other.BoundsRadius;

		// If the distance between the centers is greater than the sum of the radii, the spheres do not intersect
		if (distanceSquared > radiiSum * radiiSum) {
			return BoundsContainResult::DoesNotContain;
		}

		// If the distance between the centers plus the radius of the other sphere is less than or equal to the radius of this sphere,
		// this sphere contains the other sphere
		if (distanceSquared <= (BoundsRadius - other.BoundsRadius) * (BoundsRadius - other.BoundsRadius)) {
			return BoundsContainResult::Contains;
		}

		// Otherwise, the spheres intersect
		return BoundsContainResult::Intersects;
	}

	BoundsContainResult Bounds_Sphere::Contains(const Bounds_AABB& aabb, float looseness) const {
		// Calculate the squared distance from the sphere's center to the AABB
		float distanceSquared = 0.0f;

		float temp_radius = BoundsRadius * looseness;

		// For each axis, add the squared distance from the sphere's center to the AABB if the sphere's center is outside the AABB
		for (int i = 0; i < 3; ++i) {
			if (BoundsCentre[i] < aabb.BoundsMin[i]) {
				distanceSquared += (aabb.BoundsMin[i] - BoundsCentre[i]) * (aabb.BoundsMin[i] - BoundsCentre[i]);
			}
			else if (BoundsCentre[i] > aabb.BoundsMax[i]) {
				distanceSquared += (BoundsCentre[i] - aabb.BoundsMax[i]) * (BoundsCentre[i] - aabb.BoundsMax[i]);
			}
		}

		// If the distance squared is greater than the radius squared, the AABB is outside the sphere
		if (distanceSquared > temp_radius * temp_radius) {
			return BoundsContainResult::DoesNotContain;
		}

		// Check if the sphere contains the entire AABB
		bool contains = true;
		for (int i = 0; i < 3; ++i) {
			if (aabb.BoundsMin[i] < BoundsCentre[i] - temp_radius || aabb.BoundsMax[i] > BoundsCentre[i] + temp_radius) {
				contains = false;
				break;
			}
		}

		if (contains) {
			return BoundsContainResult::Contains;
		}

		return BoundsContainResult::Intersects;
	}

#pragma endregion

#pragma region AABB

	glm::vec3 Bounds_AABB::Center() const {
		return (BoundsMin + BoundsMax) * 0.5f;
	}

	glm::vec3 Bounds_AABB::Size() const {
		return BoundsMax - BoundsMin;
	}

	float Bounds_AABB::MaxExtent() const {
		glm::vec3 extent = (BoundsMax - BoundsMin) * 0.5f;
		return glm::length(extent);
	}

	glm::mat4 Bounds_AABB::GetGlobalBoundsMat4() const {
		// Create the transformation matrix
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, Center());
		transform = glm::scale(transform, Size());

		return transform;
	}

	BoundsContainResult Bounds_AABB::Contains(const Bounds_AABB& other, float looseness) const {

		// Calculate the center of the current AABB
		glm::vec3 center = Center();

		// Calculate the half-size of the current AABB
		glm::vec3 halfSize = Size() * 0.5f;

		// Adjust the size of the AABB by the looseness multiplier
		glm::vec3 adjustedHalfSize = halfSize * looseness;

		// Calculate the loose bounds
		glm::vec3 looseMin = center - adjustedHalfSize;
		glm::vec3 looseMax = center + adjustedHalfSize;

		// Check if this AABB completely contains the other AABB
		bool contains = (looseMin.x <= other.BoundsMin.x && looseMax.x >= other.BoundsMax.x) &&
			(looseMin.y <= other.BoundsMin.y && looseMax.y >= other.BoundsMax.y) &&
			(looseMin.z <= other.BoundsMin.z && looseMax.z >= other.BoundsMax.z);

		if (contains) {
			return BoundsContainResult::Contains;
		}

		// Check if this AABB intersects with the other AABB
		bool intersects = (looseMin.x <= other.BoundsMax.x && looseMax.x >= other.BoundsMin.x) &&
			(looseMin.y <= other.BoundsMax.y && looseMax.y >= other.BoundsMin.y) &&
			(looseMin.z <= other.BoundsMax.z && looseMax.z >= other.BoundsMin.z);

		if (intersects) {
			return BoundsContainResult::Intersects;
		}

		// If neither contains nor intersects, return DoesNotContain
		return BoundsContainResult::DoesNotContain;
	}

	BoundsContainResult Bounds_AABB::Contains(const Bounds_Sphere& sphere, float looseness) const {
		glm::vec3 center = Center();
		glm::vec3 halfSize = Size() * 0.5f * looseness;

		glm::vec3 clampedCenter = glm::clamp(sphere.BoundsCentre, BoundsMin - halfSize, BoundsMax + halfSize);
		float distanceSquared = glm::length2(clampedCenter - sphere.BoundsCentre);

		if (distanceSquared > sphere.BoundsRadius * sphere.BoundsRadius) {
			return BoundsContainResult::DoesNotContain;
		}

		bool contains = (BoundsMin.x - halfSize.x <= sphere.BoundsCentre.x - sphere.BoundsRadius) &&
			(BoundsMax.x + halfSize.x >= sphere.BoundsCentre.x + sphere.BoundsRadius) &&
			(BoundsMin.y - halfSize.y <= sphere.BoundsCentre.y - sphere.BoundsRadius) &&
			(BoundsMax.y + halfSize.y >= sphere.BoundsCentre.y + sphere.BoundsRadius) &&
			(BoundsMin.z - halfSize.z <= sphere.BoundsCentre.z - sphere.BoundsRadius) &&
			(BoundsMax.z + halfSize.z >= sphere.BoundsCentre.z + sphere.BoundsRadius);

		if (contains) {
			return BoundsContainResult::Contains;
		}

		return BoundsContainResult::Intersects;
	}

	BoundsContainResult Bounds_AABB::Contains(const glm::vec3& point, float looseness) const
	{
		glm::vec3 minBounds = BoundsMin * looseness;
		glm::vec3 maxBounds = BoundsMax * looseness;

		if (point.x > minBounds.x && point.x < maxBounds.x &&
			point.y > minBounds.y && point.y < maxBounds.y &&
			point.z > minBounds.z && point.z < maxBounds.z) 
		{
			return BoundsContainResult::Contains;
		}

		return BoundsContainResult::DoesNotContain;
	}

	glm::vec3 Bounds_AABB::ClosestPoint(const glm::vec3& point_location) const
	{
		glm::vec3 closest_point{};

		// Clamp each component of the point within the AABB's min and max bounds
		closest_point.x = glm::clamp(point_location.x, BoundsMin.x, BoundsMax.x);
		closest_point.y = glm::clamp(point_location.y, BoundsMin.y, BoundsMax.y);
		closest_point.z = glm::clamp(point_location.z, BoundsMin.z, BoundsMax.z);

		return closest_point;
	}

#pragma endregion

}