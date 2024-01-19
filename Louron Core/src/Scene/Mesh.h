#pragma once

#include "../OpenGL/Texture.h"
#include "../OpenGL/Material.h"
#include "../OpenGL/Vertex Array.h"

#include "../Core/Engine.h"

#include <string>
#include <vector>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Louron {

	class MeshFilter {

	public:

		std::unique_ptr<VertexArray> VAO;
		GLuint MaterialIndex = -1;

		MeshFilter() = default;
		MeshFilter(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
		~MeshFilter() {

		}

	};
}