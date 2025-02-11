#pragma once

// Louron Core Headers
#include "../Scene/Entity.h"

#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/Skybox.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	struct RenderPassStats {

		GLuint DrawCalls = 0; // Total low level OpenGL draw calls for everything and anything

		GLuint Skybox_Rendered = 0; // Only Sub Meshes that have been rendered individually
		GLuint SubMeshes_Rendered = 0; // Only Sub Meshes that have been rendered individually
		GLuint SubMeshes_Instanced = 0; // Only Sub Meshes that have been instanced together

		GLuint Entities_Culled = 0; // Number of Entities that have been culled

		GLuint Primitives_VerticeCount = 0; // Total Vertice Count
		GLuint Primitives_TriangleCount = 0; // Total Triangles Rendered
		GLuint Primitives_LineCount = 0; // Total Lines Rendered

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

		static void DrawDebugCube();
		static void DrawDebugSphere();
		static void DrawInstancedDebugCube(std::vector<glm::mat4> transforms);

		static void DrawSubMesh(std::shared_ptr<SubMesh> sub_mesh);
		static void DrawSkybox(SkyboxComponent& skybox);
		static void DrawInstancedSubMesh(std::shared_ptr<SubMesh> mesh, std::vector<glm::mat4> transforms);

		static void CleanupRenderData();

		static void ClearRenderStats();
		static const RenderPassStats& GetFrameRenderStats();

	};

}