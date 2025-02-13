#include "Frustum.h"

#include "Bounds.h"
#include <vector>

namespace Louron {

	void Frustum::RecalculateFrustum(const glm::mat4& view_proj_matrix)
	{		// Left plane
		planes[0].normal.x = view_proj_matrix[0][3] + view_proj_matrix[0][0];
		planes[0].normal.y = view_proj_matrix[1][3] + view_proj_matrix[1][0];
		planes[0].normal.z = view_proj_matrix[2][3] + view_proj_matrix[2][0];
		planes[0].distance = view_proj_matrix[3][3] + view_proj_matrix[3][0];

		// Right plane
		planes[1].normal.x = view_proj_matrix[0][3] - view_proj_matrix[0][0];
		planes[1].normal.y = view_proj_matrix[1][3] - view_proj_matrix[1][0];
		planes[1].normal.z = view_proj_matrix[2][3] - view_proj_matrix[2][0];
		planes[1].distance = view_proj_matrix[3][3] - view_proj_matrix[3][0];

		// Bottom plane
		planes[2].normal.x = view_proj_matrix[0][3] + view_proj_matrix[0][1];
		planes[2].normal.y = view_proj_matrix[1][3] + view_proj_matrix[1][1];
		planes[2].normal.z = view_proj_matrix[2][3] + view_proj_matrix[2][1];
		planes[2].distance = view_proj_matrix[3][3] + view_proj_matrix[3][1];

		// Top plane
		planes[3].normal.x = view_proj_matrix[0][3] - view_proj_matrix[0][1];
		planes[3].normal.y = view_proj_matrix[1][3] - view_proj_matrix[1][1];
		planes[3].normal.z = view_proj_matrix[2][3] - view_proj_matrix[2][1];
		planes[3].distance = view_proj_matrix[3][3] - view_proj_matrix[3][1];

		// Near plane
		planes[4].normal.x = view_proj_matrix[0][3] + view_proj_matrix[0][2];
		planes[4].normal.y = view_proj_matrix[1][3] + view_proj_matrix[1][2];
		planes[4].normal.z = view_proj_matrix[2][3] + view_proj_matrix[2][2];
		planes[4].distance = view_proj_matrix[3][3] + view_proj_matrix[3][2];

		// Far plane
		planes[5].normal.x = view_proj_matrix[0][3] - view_proj_matrix[0][2];
		planes[5].normal.y = view_proj_matrix[1][3] - view_proj_matrix[1][2];
		planes[5].normal.z = view_proj_matrix[2][3] - view_proj_matrix[2][2];
		planes[5].distance = view_proj_matrix[3][3] - view_proj_matrix[3][2];

		// Normalize the planes
		for (int i = 0; i < 6; ++i) {
			planes[i].normalize();
		}
	}

    std::vector<glm::vec4> Frustum::GetWorldCorners(const glm::mat4& view_proj_matrix)
    {
        const glm::mat4& inv = glm::inverse(view_proj_matrix);

        std::vector<glm::vec4> frustum_corner;
        for (unsigned int x = 0; x < 2; ++x)
        {
            for (unsigned int y = 0; y < 2; ++y)
            {
                for (unsigned int z = 0; z < 2; ++z)
                {
                    const glm::vec4 corner = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                    frustum_corner.push_back(corner / corner.w);
                }
            }
        }

        return frustum_corner;
    }

    Bounds_AABB Frustum::GetWorldSpaceTightBoundingBox(const glm::mat4& view_proj_matrix) {

        // Get WorldSpace Corners of View Projection Matrix
        std::vector<glm::vec4> corners = GetWorldCorners(view_proj_matrix);

        Bounds_AABB bounding_box{};
        for (auto& corner : corners) {

            // Perform perspective divide
            if (corner.w != 0.0f) 
                corner /= corner.w;
            
            bounding_box.BoundsMin = glm::min(bounding_box.BoundsMin, glm::vec3(corner));
            bounding_box.BoundsMax = glm::max(bounding_box.BoundsMax, glm::vec3(corner));
        }

        return bounding_box;
    }

	Bounds_AABB Frustum::CalculateLightSpaceBoundingBox(const glm::mat4& view_proj_matrix, glm::mat4& light_view_matrix, const glm::vec3& light_direction)
	{
        // Get WorldSpace Corners of View Projection Matrix
        std::vector<glm::vec4> corners = GetWorldCorners(view_proj_matrix);

        // Compute the center of the frustum corners
        glm::vec3 center(0.0f);
        for (const auto& corner : corners) {
            center += glm::vec3(corner);
        }
        center /= corners.size();

        // Compute the light view matrix using the frustum center
        light_view_matrix = glm::lookAt(
            center - glm::normalize(light_direction), // Light position
            center,                            // Target position
            glm::vec3(0.0f, 1.0f, 0.0f)        // Up direction
        );

        // Initialize min/max bounds for AABB
        Bounds_AABB bounding_box;
        bounding_box.BoundsMin = glm::vec3(FLT_MAX);
        bounding_box.BoundsMax = glm::vec3(-FLT_MAX);

        // Transform frustum corners into light space and compute the bounding box
        for (const auto& corner : corners) {
            glm::vec4 light_space_corner = light_view_matrix * corner;

            bounding_box.BoundsMin = glm::min(bounding_box.BoundsMin, glm::vec3(light_space_corner));
            bounding_box.BoundsMax = glm::max(bounding_box.BoundsMax, glm::vec3(light_space_corner));
        }

        return bounding_box;
	}

    static std::array<float, 5> CalculateHybridCascadeSplits(float nearPlane, float farPlane)
    {
        std::array<float, 5> cascadeSplits{};

        // Calculate the logarithmic factor for depth distribution
        float logFactor = log2(farPlane / nearPlane);

        for (int i = 0; i < 5; i++)
        {
            // Fraction of total range (0.0 to 1.0) for the current cascade
            float fraction = (i + 1) / 5.0f;

            // Logarithmic split for more depth precision at the far end
            float logSplit = nearPlane * pow(farPlane / nearPlane, fraction);

            // Uniform split for evenly distributed cascades
            float uniformSplit = nearPlane + (farPlane - nearPlane) * fraction;

            // Weighted average of both logarithmic and uniform splits
            cascadeSplits[i] = (logSplit * 0.75f + uniformSplit * 0.25f); // 75% log, 25% uniform blend
        }

        return cascadeSplits;
    }

    std::array<glm::mat4, 5> Frustum::CalculateCascadeLightSpaceMatrices(float fov, float aspect_ratio, float near_plane, float far_plane, const glm::mat4& view_matrix, const glm::vec3& light_direction, std::array<float, 5>& shadow_cascade_plane_distances)
    {
        std::array<glm::mat4, 5> light_space_matrices{};
                
        shadow_cascade_plane_distances = CalculateHybridCascadeSplits(near_plane, far_plane);

        for (int i = 0; i < 5; i++) 
        {
            glm::mat4 light_proj = glm::mat4(1.0f), light_view = glm::mat4(1.0f);

            glm::mat4 segmented_projection = glm::perspective(fov, aspect_ratio, i == 0 ? near_plane : shadow_cascade_plane_distances[i-1], shadow_cascade_plane_distances[i]);
            Bounds_AABB light_view_bounds = Frustum::CalculateLightSpaceBoundingBox(segmented_projection * view_matrix, light_view, light_direction);
            
            // Dynamic depth scaling - ensures no cutoff between cascades
            constexpr float zMult = 10.0f;
            if (light_view_bounds.BoundsMin.z < 0)
            {
                light_view_bounds.BoundsMin.z *= zMult;
            }
            else
            {
                light_view_bounds.BoundsMin.z /= zMult;
            }
            if (light_view_bounds.BoundsMax.z < 0)
            {
                light_view_bounds.BoundsMax.z /= zMult;
            }
            else
            {
                light_view_bounds.BoundsMax.z *= zMult;
            }

            light_proj = glm::ortho(
                light_view_bounds.BoundsMin.x, light_view_bounds.BoundsMax.x,
                light_view_bounds.BoundsMin.y, light_view_bounds.BoundsMax.y,
                light_view_bounds.BoundsMin.z, light_view_bounds.BoundsMax.z
            );

            light_space_matrices[i] = light_proj * light_view;
        }

        return light_space_matrices;
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