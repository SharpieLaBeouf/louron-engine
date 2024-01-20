#include "Mesh.h"

namespace Louron {

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
	{
		
		VAO = std::make_unique<VertexArray>();
		VertexBuffer* vbo = new VertexBuffer(vertices, (GLuint)vertices.size());
		BufferLayout layout = {
			{ ShaderDataType::Float3, "aPos" },
			{ ShaderDataType::Float3, "aNormal" },
			{ ShaderDataType::Float2, "aTexCoord" }
		};
		vbo->SetLayout(layout);

		IndexBuffer* ebo = new IndexBuffer(indices, (GLuint)indices.size());

		VAO->AddVertexBuffer(vbo);
		VAO->SetIndexBuffer(ebo);
	}

	void MeshFilter::LinkMeshFilterFromScene(const std::shared_ptr<MeshFilter>& meshFilter) {
		Meshes = meshFilter->Meshes;
	}

	/// <summary>
	/// Less efficient on memory as this will load the model multiple times into 
	/// memory for each instance of the MeshRenderer component.
	/// </summary>
	int MeshRenderer::LoadModelFromFile(const char* filePath, MeshFilter& meshFilter) {
		
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath,
			aiProcess_CalcTangentSpace |
			aiProcess_GenSmoothNormals |
			aiProcess_Triangulate | 
			aiProcess_OptimizeMeshes |
			aiProcess_SortByPType
			);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "[L20] Error Reading File: " << importer.GetErrorString() << std::endl;
			return GL_FALSE;
		}
		m_Directory = filePath;
		m_Directory = m_Directory.substr(0, m_Directory.find_last_of('/'));

		std::string mesh_name = filePath;
		auto lastSlash = mesh_name.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = mesh_name.rfind('.');
		auto count = lastDot == std::string::npos ? mesh_name.size() - lastSlash : lastDot - lastSlash;
		mesh_name = mesh_name.substr(lastSlash, count);

		ProcessNode(scene->mRootNode, scene, meshFilter);
		std::cout << "[L20] Loaded Mesh: " << mesh_name.c_str() << std::endl;

		return GL_TRUE;
	}

	/// <summary>
	/// More efficient than loading from file. This means that the scene resource manager
	/// will hold all the data in one place, and these components will reference the data
	/// contained within one place, opposed to duplicating data in memory.
	/// 
	/// This requires you to also Load the MeshFilter from the scene to the Entities
	/// MeshFilter component.
	/// </summary>
	void MeshRenderer::LinkMeshRendererFromScene(const std::shared_ptr<MeshRenderer>& meshRenderer)
	{
		Materials = meshRenderer->Materials;
	}

	/// <summary>
	/// Recursive function that loads all meshes and submeshes within a meshfilter.
	/// </summary>
	void MeshRenderer::ProcessNode(aiNode* node, const aiScene* scene, MeshFilter& meshFilter) {
		
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshFilter.Meshes->push_back(ProcessMesh(mesh, scene));
		}

		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, meshFilter);
		}

	}

	/// <summary>
	/// Loads the model data from ASSIMP into a Mesh and Material class.
	/// </summary>
	std::shared_ptr<Mesh> MeshRenderer::ProcessMesh(aiMesh* mesh, const aiScene* scene) const {

		// VERTICIES and INDICES

		// Process Vertices
		std::vector<Vertex> mesh_vertices;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex{ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec2(0.0f) };
			vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			if (mesh->mTextureCoords[0]) {
				vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			}
			else vertex.texCoords = glm::vec2(0.0f, 0.0f);
			mesh_vertices.push_back(vertex);
		}

		// Process Indices
		std::vector<GLuint> mesh_indices;
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				mesh_indices.push_back(face.mIndices[j]);
		}

		// MATERIALS

		// Check if Material has already been loaded, and apply that material index to the mesh
		if (Materials->size() > 0) {
			for (int i = 0; i < Materials->size(); i++) {
				if ((*Materials)[i] != nullptr && mesh->mMaterialIndex == i) {
					std::shared_ptr<Mesh> temp_mesh = std::make_shared<Mesh>(mesh_vertices, mesh_indices);
					temp_mesh->MaterialIndex = mesh->mMaterialIndex;
					return temp_mesh;
				}
			}
		}

		// Create new Material if not loaded, and gather material data from ASSIMP
		// Use Forward Plus shader, and Blank Texture as defaults
		std::shared_ptr<Material> mesh_material = std::make_shared<Material>(Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"),
			Engine::Get().GetTextureLibrary().GetTexture("blank_texture"));
		
		if (mesh->mMaterialIndex >= 0) {

			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// Load any applicable texture files
			aiString texture_str;
			Texture* texture = nullptr;

			std::string temp;
			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_str);
				temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
				texture = Engine::Get().GetTextureLibrary().loadTexture(m_Directory + "/" + temp);
				mesh_material->AddTextureMap(L20_TEXTURE_DIFFUSE_MAP, texture);
			}
			if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
				material->GetTexture(aiTextureType_SPECULAR, 0, &texture_str);
				temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
				texture = Engine::Get().GetTextureLibrary().loadTexture(m_Directory + "/" + temp);
				mesh_material->AddTextureMap(L20_TEXTURE_SPECULAR_MAP, texture);
			}

			if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
				material->GetTexture(aiTextureType_NORMALS, 0, &texture_str);
				temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
				texture = Engine::Get().GetTextureLibrary().loadTexture(m_Directory + "/" + temp);
				mesh_material->AddTextureMap(L20_TEXTURE_NORMAL_MAP, texture);
			}

			// Load relevant material values
			float shine = 0;
			aiColor3D colour;
			material->Get(AI_MATKEY_COLOR_DIFFUSE, colour);
			mesh_material->SetDiffuse(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
			material->Get(AI_MATKEY_COLOR_SPECULAR, colour);
			mesh_material->SetSpecular(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
			material->Get(AI_MATKEY_SHININESS, shine);
			mesh_material->SetShine(shine);
		}

		Materials->push_back(mesh_material);

		std::shared_ptr<Mesh> temp_mesh = std::make_shared<Mesh>(mesh_vertices, mesh_indices);
		temp_mesh->MaterialIndex = mesh->mMaterialIndex;
		return temp_mesh;
	}
	
}