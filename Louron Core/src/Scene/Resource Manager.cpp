#include "Resource Manager.h"

#include <iomanip>

namespace Louron {


	int ResourceManager::LoadMesh(const char* filePath, const std::shared_ptr<Shader>& shader) {

		std::string mesh_name = filePath;

		auto lastSlash = mesh_name.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = mesh_name.rfind('.');
		auto count = lastDot == std::string::npos ? mesh_name.size() - lastSlash : lastDot - lastSlash;
		mesh_name = mesh_name.substr(lastSlash, count);

		if (Meshes[mesh_name].first != nullptr) {
			std::cout << "[L20] INFO: Mesh Already Loaded!" << std::endl;
			return GL_FALSE;
		}

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
			std::cerr << "[L20] ERROR: " << importer.GetErrorString() << std::endl;
			assert(false);
			return GL_FALSE;
		}

		std::string directory = filePath;
		directory = directory.substr(0, directory.find_last_of('/'));

		std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<MeshRenderer>> meshGroup;
		meshGroup.first = std::make_shared<MeshFilter>();
		meshGroup.second = std::make_shared<MeshRenderer>();

		ProcessNode(scene->mRootNode, scene, meshGroup, directory, shader, mesh_name);

		Meshes[mesh_name] = meshGroup;

		std::cout << "[L20] Loaded Mesh: " << mesh_name.c_str() << std::endl;

		return GL_TRUE;
	}

	void ResourceManager::ProcessNode(aiNode* node, const aiScene* scene, std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<MeshRenderer>>& modelComponentGroup, const std::string& directory, const std::shared_ptr<Shader>& shader, const std::string& modelName) {

		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh, scene, modelComponentGroup, directory, shader, modelName);

		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, modelComponentGroup, directory, shader, modelName);
		}
	}

	void ResourceManager::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<MeshRenderer>>& modelComponentGroup, const std::string& directory, const std::shared_ptr<Shader>& shader, const std::string& modelName) {

		// 1. Process Vertices
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

		// 2. Process Indices
		std::vector<GLuint> mesh_indices;
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				mesh_indices.push_back(face.mIndices[j]);
		}

		// TODO: Add Tangent and Bitangent reading here


		// 3. Create Material and Create Unique Material Name
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);

		std::stringstream materialNameString;
		materialNameString << modelName << "." << materialName.C_Str() << "." << std::setw(3) << std::setfill('0') << mesh->mMaterialIndex;

		// 4. Check if Material Already Exists in Scene Resources Manager
		if (Materials[materialNameString.str()]) {
			std::shared_ptr<Mesh> temp_mesh = std::make_shared<Mesh>(mesh_vertices, mesh_indices);
			temp_mesh->MaterialIndex = mesh->mMaterialIndex;
			modelComponentGroup.first->Meshes->push_back(temp_mesh);
			return;
		}

		// 5. If the Material does not exist, create a new one
		std::shared_ptr<Material> temp_material = std::make_shared<Material>(shader, Engine::Get().GetTextureLibrary().GetTexture("blank_texture"));
		if (mesh->mMaterialIndex >= 0) {

			// Load Textures
			aiString texture_str;
			Texture* texture = nullptr;

			{
				std::string temp;
				if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {

					material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_str);
					temp = texture_str.C_Str();

					if (temp.rfind("..\\", 0) == 0)
						temp.erase(0, 3);

					texture = Engine::Get().GetTextureLibrary().loadTexture(directory + "/" + temp);
					temp_material->AddTextureMap(L20_TEXTURE_DIFFUSE_MAP, texture);
				}
				if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
					material->GetTexture(aiTextureType_SPECULAR, 0, &texture_str);
					temp = texture_str.C_Str();

					if (temp.rfind("..\\", 0) == 0)
						temp.erase(0, 3);

					texture = Engine::Get().GetTextureLibrary().loadTexture(directory + "/" + temp);
					temp_material->AddTextureMap(L20_TEXTURE_SPECULAR_MAP, texture);
				}

				if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
					material->GetTexture(aiTextureType_NORMALS, 0, &texture_str);
					temp = texture_str.C_Str();

					if (temp.rfind("..\\", 0) == 0)
						temp.erase(0, 3);

					texture = Engine::Get().GetTextureLibrary().loadTexture(directory + "/" + temp);
					temp_material->AddTextureMap(L20_TEXTURE_NORMAL_MAP, texture);
				}
			}
			
			// Load Material Values
			{
				float shine = 0;
				aiColor3D colour;
				material->Get(AI_MATKEY_COLOR_DIFFUSE, colour);
				temp_material->SetDiffuse(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
				material->Get(AI_MATKEY_COLOR_SPECULAR, colour);
				temp_material->SetSpecular(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
				material->Get(AI_MATKEY_SHININESS, shine);
				temp_material->SetShine(shine);
			}

			temp_material->SetName(materialNameString.str());
			
			// Push Material into Resource Manager 
			Materials[temp_material->GetName()] = temp_material;
		}

		// 6. Create Mesh and Set MaterialIndex
		std::shared_ptr<Mesh> temp_mesh = std::make_shared<Mesh>(mesh_vertices, mesh_indices);
		temp_mesh->MaterialIndex = mesh->mMaterialIndex;

		// 7. Add Mesh and Material to Respective Components
		modelComponentGroup.first->Meshes->push_back(temp_mesh);
		modelComponentGroup.second->Materials->operator[]((GLuint)mesh->mMaterialIndex) = temp_material;
	}


	int ResourceManager::LoadShader(const char* filePath) {
		return 0;
	}

	int ResourceManager::LoadTexture(const char* filePath) {
		return 0;
	}

	void ResourceManager::LinkShader(std::shared_ptr<Shader> shader)
	{
		Shaders[shader->GetName()] = shader;
	}

	std::shared_ptr<MeshFilter> ResourceManager::GetMeshFilter(const std::string& name)
	{
		if (Meshes[name].first == nullptr) {
			std::cerr << "[L20] ERROR: Mesh Not Loaded to Scene Resource Manager!" << std::endl;
			assert(false);
		}

		return Meshes[name].first;
	}

	std::shared_ptr<MeshRenderer> ResourceManager::GetMeshRenderer(const std::string& name)
	{
		if (Meshes[name].second == nullptr) {
			std::cerr << "[L20] ERROR: Mesh Not Loaded to Scene Resource Manager!" << std::endl;
			assert(false);
		}

		return Meshes[name].second;
	}

}