#include "MeshRenderer.h"

namespace Louron {

	/// <summary>
	/// MESH FILTER
	/// </summary>
	/// <param name="vertices"></param>
	/// <param name="indices"></param>
	MeshFilter::MeshFilter(Window& wnd, const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) : 
		m_Window(wnd), 
		m_MaterialIndex(-1) 
	{
		
		m_VAO = new VertexArray();
		VertexBuffer* vbo = new VertexBuffer(vertices, (GLuint)vertices.size());
		BufferLayout layout = {
			{ ShaderDataType::Float3, "aPos" },
			{ ShaderDataType::Float3, "aNormal" },
			{ ShaderDataType::Float2, "aTexCoord" }
		};
		vbo->SetLayout(layout);

		IndexBuffer* ebo = new IndexBuffer(indices, (GLuint)indices.size());

		m_VAO->AddVertexBuffer(vbo);
		m_VAO->SetIndexBuffer(ebo);
	}

	void MeshFilter::renderMeshFilter(Camera* mainCamera, Material* mat, OldLight* mainLight) {

		m_VAO->Bind();

		if (mat->Bind()) {			
			mat->SetUniforms();
			mat->GetShader()->SetMat4("model", glm::mat4(1.0f));
			mat->GetShader()->SetMat4("proj", glm::perspective(glm::radians(60.0f), (float)m_Window.GetWidth() / (float)m_Window.GetHeight(), 0.1f, 100.0f));
			mat->GetShader()->SetMat4("view", mainCamera->getViewMatrix());
			mat->GetShader()->SetVec3("u_Light.position", mainLight->position);
			mat->GetShader()->SetVec4("u_Light.ambient", mainLight->ambient);
			mat->GetShader()->SetVec4("u_Light.diffuse", mainLight->diffuse);
			mat->GetShader()->SetVec4("u_Light.specular", mainLight->specular);
			mat->GetShader()->SetVec3("u_CameraPos", mainCamera->getPosition());

			glDrawElements(GL_TRIANGLES, m_VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

			mat->UnBind();
		}

		m_VAO->Unbind();
	}

	int MeshRendererComponent::loadModel(const char* filePath, const char* shaderName) {

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath,
			aiProcess_CalcTangentSpace |
			aiProcess_GenSmoothNormals |
			aiProcess_Triangulate | 
			aiProcess_OptimizeMeshes |
			aiProcess_SortByPType
			);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << "[L20] Error Reading File: " << importer.GetErrorString() << std::endl;
			return GL_FALSE;
		}
		m_Directory = filePath;
		m_Directory = m_Directory.substr(0, m_Directory.find_last_of('/'));
		m_ShaderName = shaderName;


		std::string mesh_name = filePath;
		auto lastSlash = mesh_name.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = mesh_name.rfind('.');
		auto count = lastDot == std::string::npos ? mesh_name.size() - lastSlash : lastDot - lastSlash;
		mesh_name = mesh_name.substr(lastSlash, count);

		processNode(scene->mRootNode, scene);
		std::cout << "[L20] Loaded Mesh: " << mesh_name.c_str() << std::endl;

		return GL_TRUE;
	}

	std::vector<MeshFilter*> MeshRendererComponent::getMeshes() { return m_Meshes; }

	std::map<int, Material*>* MeshRendererComponent::getMaterials() { return &m_Materials; }
	Material* MeshRendererComponent::getMaterial(int index) { return m_Materials[index]; }
	void MeshRendererComponent::addMaterial(int index, Material* mat) { m_Materials[index] = mat; }

	void MeshRendererComponent::renderEntireMesh(Camera* mainCamera, OldLight* mainLight) {
		for (GLuint i = 0; i < m_Meshes.size(); i++) {
			m_Meshes[i]->renderMeshFilter(mainCamera, m_Materials[m_Meshes[i]->getMaterialIndex()], mainLight);
		}
	}

	void MeshRendererComponent::processNode(aiNode* node, const aiScene* scene) {
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(processMesh(mesh, scene));
			m_MeshCount++;
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}

	}

	MeshFilter* MeshRendererComponent::processMesh(aiMesh* mesh, const aiScene* scene) {

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

		// Create Material
		for (int i = 0; i < m_Materials.size(); i++) {
			if (m_Materials[i] != nullptr) {
				if (m_Materials[i]->GetMaterialIndex() == mesh->mMaterialIndex) {

					MeshFilter* temp_mesh = new MeshFilter(Engine::Get().GetWindow(), mesh_vertices, mesh_indices);
					temp_mesh->setMaterialIndex(mesh->mMaterialIndex);
					return temp_mesh;
				}

			}
		}

		Material* mesh_material = new Material(Engine::Get().GetShaderLibrary().GetShader(m_ShaderName),
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
			material->Get(AI_MATKEY_COLOR_AMBIENT, colour);
			mesh_material->SetAmbient(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
			material->Get(AI_MATKEY_COLOR_DIFFUSE, colour);
			mesh_material->SetDiffuse(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
			material->Get(AI_MATKEY_COLOR_SPECULAR, colour);
			mesh_material->SetSpecular(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
			material->Get(AI_MATKEY_SHININESS, shine);
			mesh_material->SetShine(32.0f);

			mesh_material->SetMaterialIndex(mesh->mMaterialIndex);
		}
		m_Materials[mesh->mMaterialIndex] = mesh_material;

		MeshFilter* temp_mesh = new MeshFilter(Engine::Get().GetWindow(), mesh_vertices, mesh_indices);
		temp_mesh->setMaterialIndex(mesh->mMaterialIndex);
		return temp_mesh;
	}
}