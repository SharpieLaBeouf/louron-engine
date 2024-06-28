#pragma once

// Louron Core Headers
#include "../Scene/Entity.h"

#include "../Scene/Components/Camera.h"
#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/Skybox.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	class Renderer {

	public:

		Renderer() = default;

		static void ClearColour(const glm::vec4 colour);
		static void ClearBuffer(GLbitfield mask);

		static void DrawMesh(std::shared_ptr<Mesh> Mesh);
		static void DrawSkybox(SkyboxComponent& skybox);
		static void DrawInstancedMesh(std::shared_ptr<Mesh> Mesh, std::vector<Transform> Transforms);

		static void CleanupInstanceData();
	
	private:
				
	};

}