#pragma once

#include "../Scene/Camera.h"
#include "../Scene/Entity.h"
#include "../Scene/Mesh.h"

namespace Louron {

	class Renderer {

	public:

		Renderer() = default;

		static void DrawMeshComponent(Entity* MeshEntity, Entity* CameraEntity, std::string optionalShader = "");
		static void DrawMeshFilter(std::shared_ptr<MeshFilter> Mesh);

		static void DrawMesh(std::shared_ptr<Mesh> Mesh);

	private:


	public:


	};

}