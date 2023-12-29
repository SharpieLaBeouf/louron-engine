#pragma once

#include "Camera.h"
#include "Light.h"
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

	class MeshRenderer;

	class MeshFilter {

	public:


		MeshFilter(Window& wnd, const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
		~MeshFilter() { delete m_VAO; }

		/// <summary>
		/// 1. DrawElements for the entire mesh using the singular material
		/// 
		///   Realistically this should only take the scene heirarchy and then 
		///   retrieve the relevant elements, or maybe be passed a struct of 
		///   vectors which contain relevent scene details required for renderering
		///   the current object.
		/// 
		/// </summary>
		void renderMeshFilter(Camera* mainCamera, Material* mat, OldLight* mainLight);

		void setMaterialIndex(GLuint index) { m_MaterialIndex = index; }
		GLuint getMaterialIndex() const { return m_MaterialIndex; }

	private:

		GLuint m_MaterialIndex;

		// OpenGL Data References
		VertexArray* m_VAO = nullptr;
		Window& m_Window;
	};

	class MeshRendererComponent {

	public:
		MeshRendererComponent() : m_ShaderName("material_shader_flat") { }
		MeshRendererComponent(const MeshRendererComponent&) = default;

		int loadModel(const char* filePath, const char* shaderName);

		/// <summary>
		/// 1. Sorts all meshfilters based on material
		/// 2. Loops all different types of materials
		///		-> Bind Material00 (set uniforms, etc.)
		///			-> Loop render all meshes with bound material00
		///		-> Bind Material01 (set uniforms, etc.)
		///			-> Loop render all meshes with bound material01
		///		-> etc ... foreach groupings of material
		/// </summary>
		void renderEntireMesh(Camera* mainCamera, OldLight* mainLight);

		std::vector<MeshFilter*> getMeshes();

		std::map<int, Material*>* getMaterials();
		Material* getMaterial(int index);

		void addMaterial(int index, Material* mat);

		bool active = true;
	private:
		const char* m_ShaderName;

		std::string m_Directory;

		GLuint m_MeshCount = 0;
		std::vector<MeshFilter*> m_Meshes;
		std::map<int, Material*> m_Materials;

		void processNode(aiNode* node, const aiScene* scene);
		MeshFilter* processMesh(aiMesh* mesh, const aiScene* scene);
	};
}