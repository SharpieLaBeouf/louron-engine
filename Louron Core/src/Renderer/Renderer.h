#pragma once

// Louron Core Headers
#include "../Scene/Entity.h"

#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/Skybox.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	struct RenderPassStats {

		// Draw Calls
		GLuint Individual_DrawCalls = 0;			// Total Individual Geometry Draw Calls
		GLuint Instanced_DrawCalls = 0;				// Total Instanced Geometry Draw Calls

		// Geometry - Depth
		GLuint Geometry_Depth_Rendered = 0;			// Geometry that has been rendered individually in depth pass
						
		GLuint Geometry_Depth_TriangleCount = 0;	// Total Depth Triangles Rendered
		GLuint Geometry_Depth_VerticeCount = 0;		// Total Depth Vertice Count
		GLuint Geometry_Depth_LineCount = 0;		// Total Depth Lines Rendered

		// Geometry - Colour
		GLuint Geometry_Colour_Rendered = 0;		// Geometry that has been rendered individually in colour pass
		GLuint Geometry_Colour_Instanced = 0;		// Geometry that has been rendered as instances in colour pass
				
		GLuint Geometry_Colour_TriangleCount = 0;	// Total Colour Triangles Rendered
		GLuint Geometry_Colour_VerticeCount = 0;	// Total Colour Vertice Count
		GLuint Geometry_Colour_LineCount = 0;		// Total Colour Lines Rendered

		// Entity Culling
		GLuint Entities_Culled_Frustum = 0;			// Entities Culled by Frustum Octree Culling
		GLuint Entities_Culled_Occlusion = 0;		// Entities Culled by Occlusion Queries
		GLuint Entities_Culled_Remaining = 0;		// Remaining Entities Post Culling

		// Debug
		GLuint Debug_Individual_DrawCalls = 0;		// Total Individual Debug Draw Calls
		GLuint Debug_Instanced_DrawCalls = 0;		// Total Instanced Debug Draw Calls

		GLuint Debug_Geometry_Colour_Rendered = 0;	// Debug Geometry that has been rendered individually in colour pass
		GLuint Debug_Geometry_Colour_Instanced = 0;	// Debug Geometry that has been rendered as instances in colour pass
		GLuint Debug_Geometry_Depth_Rendered = 0;	// Debug Geometry that has been rendered individually in depth pass

		GLuint Debug_Geometry_TriangleCount = 0;	// Total Debug Triangles
		GLuint Debug_Geometry_VerticeCount = 0;		// Total Debug Vertices
		GLuint Debug_Geometry_LineCount = 0;		// Total Debug Lines
	};

	class Renderer {

	private:

		static RenderPassStats s_RenderStats;

		friend class ForwardPlusPipeline;

	public:

		Renderer() = default;

		// This is used to init the debug vertex arrays for cubes and spheres
		static void Init();

		static void ClearColour(const glm::vec4 colour);
		static void ClearBuffer(GLbitfield mask);

		static void DrawDebugCubeTriangles(bool is_depth_pass = false);
		static void DrawDebugCubeLines();
		static void DrawDebugSphereTriangles(bool is_depth_pass = false);
		static void DrawDebugSphereLines();
		static void DrawInstancedDebugCube(std::vector<glm::mat4> transforms);

		static void DrawSubMesh(std::shared_ptr<SubMesh> sub_mesh, bool is_depth_pass = false);
		static void DrawSkybox(SkyboxComponent& skybox);
		static void DrawInstancedSubMesh(std::shared_ptr<SubMesh> mesh, std::vector<glm::mat4> transforms);

		static void CleanupRenderData();

		static void ClearRenderStats();
		static const RenderPassStats& GetFrameRenderStats();

	};

}