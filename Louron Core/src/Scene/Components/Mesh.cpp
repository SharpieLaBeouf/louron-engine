#include "Mesh.h"

// Louron Core Headers
#include "Components.h"
#include "../../Renderer/Renderer.h"

// C++ Standard Library Headers
#include <iomanip>

// External Vendor Library Headers

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

	void MeshFilter::LinkMeshFilter(const std::shared_ptr<MeshFilter>& meshFilter) {
		Meshes = meshFilter->Meshes;
	}

	/// <summary>
	/// Less efficient on memory as this will load the model multiple times into 
	/// memory for each instance of the MeshRenderer component.
	/// </summary>
	int MeshRenderer::LoadModelFromFile(const char* filePath, MeshFilter& meshFilter) {
		
		m_FilePath = filePath;
		std::string mesh_name = m_FilePath;

		auto lastSlash = mesh_name.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = mesh_name.rfind('.');
		auto count = lastDot == std::string::npos ? mesh_name.size() - lastSlash : lastDot - lastSlash;
		mesh_name = mesh_name.substr(lastSlash, count);

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath,
			aiProcess_CalcTangentSpace | \
			aiProcess_GenSmoothNormals | \
			aiProcess_JoinIdenticalVertices | \
			aiProcess_ImproveCacheLocality | \
			aiProcess_LimitBoneWeights | \
			aiProcess_RemoveRedundantMaterials | \
			aiProcess_SplitLargeMeshes | \
			aiProcess_Triangulate | \
			aiProcess_GenUVCoords | \
			aiProcess_SortByPType | \
			aiProcess_FindDegenerates | \
			aiProcess_FindInvalidData | \
			0
		);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			L_CORE_ASSERT(false, importer.GetErrorString());
			return GL_FALSE;
		}

		std::string directory = filePath;
		directory = directory.substr(0, directory.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene, meshFilter, directory, Engine::Get().GetShaderLibrary().GetShader("material_shader_flat"), mesh_name);
		std::cout << "[L20] Mesh Loaded: " << mesh_name.c_str() << std::endl;

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
	void MeshRenderer::LinkMeshRenderer(const std::shared_ptr<MeshRenderer>& meshRenderer)
	{
		Materials = meshRenderer->Materials;
		m_FilePath = meshRenderer->m_FilePath;
	}

	/// <summary>
	/// Only use this when you need to manually draw a mesh that is NOT within a Scene. This
	/// means that no lighting will affect this model. If you attempt to ManualDraw an object
	/// that has been loaded into the Scene, this will result in undefined behaviour.
	/// </summary>
	void MeshRenderer::ManualDraw(const MeshFilter& meshFilter, const Camera& camera, const Transform& transform) {

		for (const auto& material : (*Materials)) {
			if (material.second->Bind()) {
				material.second->UpdateUniforms(camera);
				material.second->GetShader()->SetMat4("u_VertexIn.Model", transform.GetTransform());

				for (const auto& mesh : (*meshFilter.Meshes)) 
					if (mesh->MaterialIndex == material.first) 
						Renderer::DrawMesh(mesh);
			}
		}

	}

	void MeshRenderer::SetPath(const std::string& filePath)	{
		m_FilePath = filePath;
	}

	std::string MeshRenderer::GetPath() const {
		return m_FilePath;
	}

	/// <summary>
	/// Recursive function that loads all meshes and submeshes within a meshfilter.
	/// </summary>
	void MeshRenderer::ProcessNode(aiNode* node, const aiScene* scene, MeshFilter& meshFilter, const std::string& directory, std::shared_ptr<Shader> shader, const std::string& modelName)
	{
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh, scene, meshFilter, directory, shader, modelName);
		}

		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, meshFilter, directory, shader, modelName);
		}

	}

	/// <summary>
	/// Loads the model data from ASSIMP into a Mesh and Material class.
	/// </summary>
	void MeshRenderer::ProcessMesh(aiMesh* mesh, const aiScene* scene, MeshFilter& meshFilter, const std::string& directory, std::shared_ptr<Shader> shader, const std::string& modelName)
	{
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
		if (Materials) {
			if (Materials->operator[](mesh->mMaterialIndex)) {
				std::shared_ptr<Mesh> temp_mesh = std::make_shared<Mesh>(mesh_vertices, mesh_indices);
				temp_mesh->MaterialIndex = mesh->mMaterialIndex;
				meshFilter.Meshes->push_back(temp_mesh);
			}
		}

		// Create new Material if not loaded, and gather material data from ASSIMP
		// Use Forward Plus shader, and Blank Texture as defaults
		std::shared_ptr<Material> temp_material = std::make_shared<Material>(shader, Engine::Get().GetTextureLibrary().GetTexture("blank_texture"));

		if (mesh->mMaterialIndex >= 0) {

			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// Load any applicable texture files
			aiString texture_str;
			std::shared_ptr<Texture> texture = nullptr;

			std::string temp;
			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_str);
				temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
				texture = Engine::Get().GetTextureLibrary().LoadTexture(m_Directory + "/" + temp);
				temp_material->AddTextureMap(L20_TEXTURE_DIFFUSE_MAP, texture);
			}
			if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
				material->GetTexture(aiTextureType_SPECULAR, 0, &texture_str);
				temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
				texture = Engine::Get().GetTextureLibrary().LoadTexture(m_Directory + "/" + temp);
				temp_material->AddTextureMap(L20_TEXTURE_SPECULAR_MAP, texture);
			}

			if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
				material->GetTexture(aiTextureType_NORMALS, 0, &texture_str);
				temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
				texture = Engine::Get().GetTextureLibrary().LoadTexture(m_Directory + "/" + temp);
				temp_material->AddTextureMap(L20_TEXTURE_NORMAL_MAP, texture);
			}

			// Load relevant material values
			float shine = 0;
			aiColor3D colour;
			material->Get(AI_MATKEY_COLOR_DIFFUSE, colour);
			temp_material->SetDiffuse(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
			material->Get(AI_MATKEY_COLOR_SPECULAR, colour);
			temp_material->SetSpecular(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
			material->Get(AI_MATKEY_SHININESS, shine);
			temp_material->SetShine(shine);
			
			aiString materialName;
			material->Get(AI_MATKEY_NAME, materialName);

			std::stringstream materialNameString;
			materialNameString << modelName << "." << materialName.C_Str() << "." << std::setw(3) << std::setfill('0') << mesh->mMaterialIndex;

			temp_material->SetName(materialNameString.str());

			Materials->operator[]((GLuint)mesh->mMaterialIndex) = temp_material;
		}

		std::shared_ptr<Mesh> temp_mesh = std::make_shared<Mesh>(mesh_vertices, mesh_indices);
		temp_mesh->MaterialIndex = mesh->mMaterialIndex;
		meshFilter.Meshes->push_back(temp_mesh);
	}	
}