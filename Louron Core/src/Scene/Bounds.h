#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Louron {

	enum class BoundsContainResult {
		DoesNotContain,
		Intersects,
		Contains
	};

	struct Bounds_Sphere;
	struct Bounds_AABB;

	struct Bounds_Sphere {

		glm::vec3 BoundsCentre = glm::vec3(0.0f);
		float BoundsRadius = 0.0f;

		Bounds_Sphere() = default;
		Bounds_Sphere(const glm::vec3& centre, const float& radius) : BoundsCentre(centre), BoundsRadius(radius) {}
		Bounds_Sphere(const Bounds_Sphere&) = default;
		Bounds_Sphere(Bounds_Sphere&&) = default;
		~Bounds_Sphere() = default;

		/// <summary>
		/// Determine if this sphere contains or intersects with another sphere.
		/// </summary>
		/// <param name="other">The other Bounds_Sphere to test against.</param>
		/// <param name="looseness">This is a multiplier. 1.0f = Normal Radius, 2.0f = Test Against Current Radius Expanded by 2.0f.</param>
		/// <returns>An enum holding the result of the function.</returns>
		BoundsContainResult Contains(const Bounds_Sphere& other, float looseness = 1.0f) const;
		
		/// <summary>
		/// Determine if this sphere contains or intersects with the given AABB.
		/// </summary>
		/// <param name="aabb">The Bounds_AABB to test against.</param>
		/// <param name="looseness">This is a multiplier. 1.0f = Normal Radius, 2.0f = Test Against Current Radius Expanded by 2.0f.</param>
		/// <returns>An enum holding the result of the function.</returns>
		BoundsContainResult Contains(const Bounds_AABB& aabb, float looseness = 1.0f) const;
	};

	struct Bounds_AABB {
		glm::vec3 BoundsMin = glm::vec3(FLT_MAX);
		glm::vec3 BoundsMax = glm::vec3(-FLT_MAX);

		Bounds_AABB() = default;
		Bounds_AABB(const glm::vec3& min, const glm::vec3& max) : BoundsMin(min), BoundsMax(max) {}

		Bounds_AABB(const Bounds_AABB&) = default;
		Bounds_AABB(Bounds_AABB&&) noexcept = default; // Explicitly declare move constructor
		Bounds_AABB& operator=(const Bounds_AABB&) = default;
		Bounds_AABB& operator=(Bounds_AABB&&) noexcept = default; // Explicitly declare move assignment operator
		~Bounds_AABB() = default;
		
		bool EqualsWithTolerance(const glm::vec3& v1, const glm::vec3& v2, float tolerance = 1e-5f) const {
			return glm::all(glm::lessThan(glm::abs(v1 - v2), glm::vec3(tolerance)));
		}

		bool operator==(const Bounds_AABB& other) const {
			return EqualsWithTolerance(BoundsMin, other.BoundsMin) &&
				EqualsWithTolerance(BoundsMax, other.BoundsMax);
		}

		Bounds_AABB operator*(float factor) const {
			// Calculate the center and size of the current AABB
			glm::vec3 center = Center();
			glm::vec3 halfSize = Size() * 0.5f;

			// Scale the halfSize
			glm::vec3 scaledHalfSize = halfSize * factor;

			// Calculate the new min and max bounds
			glm::vec3 newBoundsMin = center - scaledHalfSize;
			glm::vec3 newBoundsMax = center + scaledHalfSize;

			// Return a new Bounds_AABB with the scaled bounds
			return Bounds_AABB(newBoundsMin, newBoundsMax);
		}

		/// <summary>
		/// Calculate the Centre of the AABB
		/// </summary>
		/// <returns></returns>
		glm::vec3 Center() const;

		/// <summary>
		/// Calculate the Size of the AABB
		/// </summary>
		glm::vec3 Size() const;

		/// <summary>
		/// Compute the maximum extent (half the diagonal length)
		/// </summary>
		/// <returns></returns>
		float MaxExtent() const;

		/// <summary>
		/// Get the transformation matrix for this AABB
		/// </summary>
		glm::mat4 GetGlobalBoundsMat4() const;

		/// <summary>
		/// This will determine if another Bounds_AABB is contained or intersects
		/// with this Bounds_AABB.
		/// </summary>
		/// <param name="other">Other AABB to test against.</param>
		/// <param name="looseness">This is a multiplier. 1.0f = Normal AABB, 2.0f = Test Against Current AABB Expanded by 2.0f.</param>
		/// <returns>An enum holding the result of the function.</returns>
		BoundsContainResult Contains(const Bounds_AABB& other, float looseness = 1.0f) const;

		/// <summary>
		/// Determine if this AABB contains or intersects with the given Bounds_Sphere.
		/// </summary>
		/// <param name="sphere">The Bounds_Sphere to test against.</param>
		/// <param name="looseness">A multiplier to adjust the size of the AABB for the test.</param>
		/// <returns>An enum holding the result of the function.</returns>
		BoundsContainResult Contains(const Bounds_Sphere& sphere, float looseness = 1.0f) const;

		/// <summary>
		/// This will determine if another Bounds_AABB is contained or intersects
		/// with this Bounds_AABB.
		/// </summary>
		/// <param name="other">Other AABB to test against.</param>
		/// <param name="looseness">This is a multiplier. 1.0f = Normal AABB, 2.0f = Test Against Current AABB Expanded by 2.0f.</param>
		/// <returns>An enum holding the result of the function.</returns>
		BoundsContainResult Contains(const glm::vec3& point, float looseness = 1.0f) const;
	};

}