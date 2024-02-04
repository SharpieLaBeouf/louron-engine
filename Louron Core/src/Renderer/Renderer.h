#pragma once

#include "../Scene/Camera.h"
#include "../Scene/Entity.h"
#include "../Scene/Mesh.h"

namespace Louron {

	class Renderer {

	public:

		Renderer() = default;

		static void ClearColour(const glm::vec4 colour);
		static void ClearBuffer(GLbitfield mask);

		static void DrawMesh(std::shared_ptr<Mesh> Mesh);
		static void DrawInstancedMesh(std::shared_ptr<Mesh> Mesh, std::vector<Transform> Transforms);
	
	private:
				
	};

}