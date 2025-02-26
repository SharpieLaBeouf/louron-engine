#include "Asset Importer.h"

#include "Asset Manager API.h"

#include "../Project/Project.h"

#include "../Debug/Profiler.h"

#include <map>
#include <memory>
#include <functional>

// External Vendor Library Headers
#include <glm/gtx/string_cast.hpp>

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Louron {


#pragma region Asset Import

	using AssetImportFunction = std::function<std::shared_ptr<Asset>(AssetMap*, AssetRegistry*, AssetHandle, const AssetMetaData&, const std::filesystem::path&)>;
	
	static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {

		//{ AssetType::Audio, AudioImporter::ImportAudio },
		{ AssetType::Prefab,					PrefabImporter::ImportPrefab },
		{ AssetType::Scene,						SceneImporter::ImportScene },

		{ AssetType::Texture2D,					TextureImporter::ImportTexture2D },

		{ AssetType::Material_Standard,			MaterialImporter::ImportMaterial },
		{ AssetType::Material_Skybox,			MaterialImporter::ImportMaterial },

		{ AssetType::ModelImport,				ModelImporter::ImportModel },

		{ AssetType::Shader,					ShaderImporter::ImportShader },
		{ AssetType::Compute_Shader,			ShaderImporter::ImportComputeShader }
	};

	std::shared_ptr<Asset> AssetImporter::ImportAsset(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& metadata, const std::filesystem::path& project_asset_directory)
	{
		if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end())
		{
			L_CORE_ERROR("No importer available for asset type: {0}", AssetUtils::AssetTypeToString(metadata.Type));
			return nullptr;
		}

		return s_AssetImportFunctions.at(metadata.Type)(asset_map, asset_reg, handle, metadata, project_asset_directory);
	}

#pragma endregion

#pragma region Scene Import


	std::shared_ptr<Scene> SceneImporter::ImportScene(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory) 
	{
		auto project = Project::GetActiveProject();
		return LoadScene(asset_map, asset_reg, project->GetProjectDirectory() / project->GetConfig().AssetDirectory / meta_data.FilePath);
	}

	std::shared_ptr<Scene> SceneImporter::LoadScene(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path) 
	{

		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		SceneSerializer serializer(scene);
		serializer.Deserialize(path);
		return scene;
	}

#pragma endregion

#pragma region Prefab File Import

	std::shared_ptr<Prefab> PrefabImporter::ImportPrefab(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& metadata, const std::filesystem::path& project_asset_directory)
	{
		return LoadPrefab(metadata.IsCustomAsset ? metadata.FilePath : Project::GetActiveProject()->GetAssetDirectory() / metadata.FilePath);
	}

	std::shared_ptr<Prefab> PrefabImporter::LoadPrefab(const std::filesystem::path& path)
	{
		std::shared_ptr<Prefab> prefab = std::make_shared<Prefab>();
		prefab->SetMutable(false);
		if (prefab->Deserialize(path)) 
		{
			return prefab;
		}

		return nullptr;
	}

#pragma endregion

#pragma region Texture Import

	std::shared_ptr<Texture> TextureImporter::ImportTexture2D(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory) 
	{
		return LoadTexture2D(meta_data.IsCustomAsset ? meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / meta_data.FilePath);
	}

	std::shared_ptr<Texture> TextureImporter::LoadTexture2D(const std::filesystem::path& path) {
		return std::make_shared<Texture>(path);
	}


#pragma endregion

#pragma region Material Import

	std::shared_ptr<Material> MaterialImporter::ImportMaterial(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory) {

		if (meta_data.FilePath.extension() != ".lmat" && meta_data.FilePath.extension() != ".lmaterial" && meta_data.FilePath.extension() != ".lskybox") {

			L_CORE_WARN("Incompatible Material File Extension");
			L_CORE_WARN("Extension Used: {0}", meta_data.FilePath.extension().string());

			return nullptr;
		}

		YAML::Node data;

		auto project = Project::GetActiveProject();
		std::filesystem::path file_path = project->GetProjectDirectory() / project->GetConfig().AssetDirectory / meta_data.FilePath;

		if (!std::filesystem::exists(file_path))
			return nullptr;

		try {
			data = YAML::LoadFile(file_path.string());
		}
		catch (YAML::ParserException e) {
			L_CORE_ERROR("YAML-CPP Failed to Load Scene File: '{0}', {1}", meta_data.FilePath.string(), e.what());
			return nullptr;
		}

		if (!data["Material Asset Type"]) {
			L_CORE_ERROR("Material Type Node is Not Specified in File: '{0}'", meta_data.FilePath.string());
			return nullptr;
		}

		if (data["Material Asset Type"].as<std::string>() == AssetUtils::AssetTypeToString(AssetType::Material_Skybox)) {
			return LoadMaterialSkybox(meta_data.IsCustomAsset ? meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / meta_data.FilePath);
		}

		if (data["Material Asset Type"].as<std::string>() == AssetUtils::AssetTypeToString(AssetType::Material_Standard)) {
			return LoadMaterialPBR(meta_data.IsCustomAsset ? meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / meta_data.FilePath);
		}

		return nullptr;
	}

	std::shared_ptr<Material> MaterialImporter::LoadMaterialPBR(const std::filesystem::path& path) {

		std::shared_ptr<Material> material = std::make_shared<Material>();

		if (material->Deserialize(path))
			return material;

		return nullptr;
	}

	std::shared_ptr<SkyboxMaterial> MaterialImporter::LoadMaterialSkybox(const std::filesystem::path& path) {

		std::shared_ptr<SkyboxMaterial> material = std::make_shared<SkyboxMaterial>();

		if (material->Deserialize(path))
			return material;
		
		return nullptr;
	}

#pragma endregion

#pragma region Model Import

	std::shared_ptr<Prefab> ModelImporter::ImportModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory)
	{
		return LoadModel(asset_map, asset_reg, handle, meta_data, meta_data.IsCustomAsset ? meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / meta_data.FilePath);
	}

	std::shared_ptr<Prefab> ModelImporter::LoadModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& path)
	{
		if (!asset_map) {
			L_CORE_ERROR("Cannot Import Model - Asset Map Invalid.");
			return nullptr;
		}

		if (!asset_reg) {
			L_CORE_ERROR("Cannot Import Model - Asset Registry Invalid.");
			return nullptr;
		}

		if (!std::filesystem::exists(path)) {
			L_CORE_ERROR("Cannot Import Model - File Path Does Not Exist.");
			return nullptr;
		}

		std::string model_name = path.filename().replace_extension().string();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path.string(),
			aiProcess_GenUVCoords | \
			aiProcess_GenSmoothNormals | \
			aiProcess_GenBoundingBoxes | \
			aiProcess_CalcTangentSpace | \
			aiProcess_SplitLargeMeshes | \
			aiProcess_JoinIdenticalVertices | \
			aiProcess_ImproveCacheLocality | \
			aiProcess_RemoveRedundantMaterials | \
			aiProcess_Triangulate | \
			aiProcess_SortByPType | \
			aiProcess_FindDegenerates | \
			aiProcess_FindInvalidData | \
			aiProcess_LimitBoneWeights | \
			0
		);

		if (!scene) {
			L_CORE_ERROR("Cannot Import Model - Assimp Scene Invalid: {0}", importer.GetErrorString());
			return nullptr;
		}

		if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
			L_CORE_ERROR("Cannot Import Model - Assimp Scene Data Structure Incomplete. Potentially Corrupted File.");
			return nullptr;
		}

		if (!scene->mRootNode) {
			L_CORE_ERROR("Cannot Import Model - Assimp Scene Has No Root Node");
			return nullptr;
		}

		std::shared_ptr<Prefab> model_prefab = std::make_shared<Prefab>();
		model_prefab->SetMutable(false);

		ProcessNode(scene, scene->mRootNode, model_prefab, entt::null, asset_map, asset_reg, handle, meta_data, path);

		return model_prefab;
	}

	static std::filesystem::path ResolveAssimpTexturePath(const std::filesystem::path& asset_file_path, const aiString& assimp_texture_string)
	{
		std::filesystem::path absolute_texture_path = assimp_texture_string.C_Str(); // Assume is absolute

		if (absolute_texture_path.is_relative()) // Check if relative
		{
			absolute_texture_path = std::filesystem::absolute(asset_file_path.parent_path() / assimp_texture_string.C_Str());
		}
		else if (absolute_texture_path.filename() == absolute_texture_path) // Check if path stripped and is just in the same directory as model file
		{
			absolute_texture_path = std::filesystem::absolute(asset_file_path.parent_path() / absolute_texture_path);
		}
		else // Just format it to an absolute path if neither of the above work
		{
			absolute_texture_path = std::filesystem::absolute(absolute_texture_path);
		}

		return absolute_texture_path;
	}

	void ModelImporter::ProcessMesh(const aiScene* scene, aiMesh* mesh, std::shared_ptr<Prefab> model_prefab, entt::entity current_entity_handle, std::shared_ptr<AssetMesh> asset_mesh, AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle parent_asset_handle, const AssetMetaData& parent_meta_data, const std::filesystem::path& path)
	{
		// 1. Process Vertices
		std::vector<Vertex> mesh_vertices;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

			Vertex vertex{};
			vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

			if (mesh->mTextureCoords[0])
				vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

			if (mesh->mTangents)
				vertex.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);

			if (mesh->mBitangents)
				vertex.bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

			mesh_vertices.push_back(vertex);
		}

		// 2. Process Indices
		std::vector<GLuint> mesh_indices;
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				mesh_indices.push_back(face.mIndices[j]);
		}

		// 3. Create Individual Sub Mesh and Push to Mesh Asset vector
		asset_mesh->SubMeshes.push_back(std::make_shared<SubMesh>(mesh_vertices, mesh_indices));

		// 4. Create Material Asset
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);

		auto project = Project::GetActiveProject();
		AssetHandle asset_material_handle = static_cast<uint32_t>(std::hash<std::string>{}(
			AssetUtils::AssetTypeToString(AssetType::Material_Standard) + path.filename().string() + materialName.C_Str()
		));

		// If the Material has not already been loaded into the asset map, we 
		// want to create a new material. Once this is done, we won't need to do this again 
		// for this material.
		if (asset_map->count(asset_material_handle) == 0) 
		{
			AssetMetaData material_metadata;
			material_metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetAssetDirectory());
			material_metadata.Type = AssetType::Material_Standard;
			material_metadata.AssetName = materialName.C_Str();
			material_metadata.ParentAssetHandle = parent_asset_handle;
			material_metadata.IsCustomAsset = parent_meta_data.IsCustomAsset;

			std::shared_ptr<Material> asset_material = std::make_shared<Material>();

			asset_material->SetName(materialName.C_Str());

			// Load Material Values

			ai_real temp{};
			aiColor4D colour{};

			if (material->Get(AI_MATKEY_BASE_COLOR, colour) == aiReturn_SUCCESS)
				asset_material->SetAlbedoTintColour({ colour.r, colour.g, colour.b, colour.a });
			else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, colour) == aiReturn_SUCCESS)
				asset_material->SetAlbedoTintColour({ colour.r, colour.g, colour.b, colour.a });

			if (material->Get(AI_MATKEY_METALLIC_FACTOR, temp) == aiReturn_SUCCESS)
				asset_material->SetMetallic(temp);

			if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, temp) == aiReturn_SUCCESS)
				asset_material->SetRoughness(temp);

			// Load Material Textures

			auto load_texture = [&](aiTextureType texture_type) -> void
			{
					if (texture_type == aiTextureType_NONE || material->GetTextureCount(texture_type) == 0)
						return;

					aiString assimp_texture_string;
					material->GetTexture(texture_type, 0, &assimp_texture_string);

					std::filesystem::path absolute_texture_path = ResolveAssimpTexturePath(path, assimp_texture_string);

					AssetHandle texture_handle;
					AssetMetaData texture_meta_data;
					std::shared_ptr<Texture> texture_asset = nullptr;

					if (std::filesystem::exists(absolute_texture_path))
					{
						std::filesystem::path relative_texture_path = std::filesystem::relative(absolute_texture_path, Project::GetActiveProject()->GetAssetDirectory());
						bool is_relative_to_project = !relative_texture_path.string().starts_with("..");

						texture_meta_data.AssetName = relative_texture_path.stem().string();
						texture_meta_data.FilePath = is_relative_to_project ? relative_texture_path.string() : absolute_texture_path.string();
						texture_meta_data.Type = AssetType::Texture2D;

						// As this is not an embedded texture, this meta data will not have a parent asset 
						// as the texture is standalone and can be loaded independent of the model being loaded
						texture_meta_data.ParentAssetHandle = NULL_UUID;

						texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
							AssetUtils::AssetTypeToString(texture_meta_data.Type) + texture_meta_data.FilePath.string()
						));

						// Check if texture file already loaded.
						if (asset_map->count(texture_handle) == 0)
							texture_asset = std::make_shared<Texture>(absolute_texture_path);
						else
							texture_asset = static_pointer_cast<Texture>(asset_map->at(texture_handle));
					}
					else if (auto assimp_texture_ref = scene->GetEmbeddedTexture(assimp_texture_string.C_Str()))
					{
						texture_meta_data.AssetName = assimp_texture_string.C_Str();
						texture_meta_data.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetAssetDirectory());
						texture_meta_data.Type = AssetType::Texture2D;

						// This is an embedded texture which requires the model 
						// to be loaded so we can access this texture
						texture_meta_data.ParentAssetHandle = parent_asset_handle;

						texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
							AssetUtils::AssetTypeToString(texture_meta_data.Type) + path.filename().string() + texture_meta_data.AssetName
						));


						glm::ivec2 texture_size = { assimp_texture_ref->mWidth, assimp_texture_ref->mHeight };
						GLubyte* texture_data = reinterpret_cast<GLubyte*>(assimp_texture_ref->pcData);

						// Check if texture file already loaded.
						if (asset_map->count(texture_handle) == 0)
							texture_asset = std::make_shared<Texture>(texture_data, texture_size, GL_BGRA);
						else
							texture_asset = static_pointer_cast<Texture>(asset_map->at(texture_handle));
					}
					else if (!absolute_texture_path.empty())
					{
						L_CORE_ERROR("ModelImporter::ProcessMesh: Could Not Find Texture. Path:'{}', Assimp Texture String:'{}'.", absolute_texture_path.string(), assimp_texture_string.C_Str());
						texture_asset = nullptr;
					}

					if (texture_asset)
					{

						switch (texture_type)
						{
							case aiTextureType_DIFFUSE:
							case aiTextureType_BASE_COLOR:
							{
								asset_material->SetAlbedoTexture(texture_handle);
								break;
							}

							case aiTextureType_METALNESS:
							{
								asset_material->SetMetallicTexture(texture_handle);
								break;
							}

							case aiTextureType_NORMALS:
							{
								asset_material->SetNormalTexture(texture_handle);
								break;
							}
						}

						texture_asset->Handle = texture_handle;

						asset_map->operator[](texture_handle) = texture_asset;
						asset_reg->operator[](texture_handle) = texture_meta_data;
					}

			};

			aiTextureType texture_base_colour_type = material->GetTextureCount(aiTextureType_BASE_COLOR) > 0 ? aiTextureType_BASE_COLOR : material->GetTextureCount(aiTextureType_DIFFUSE) > 0 ? aiTextureType_DIFFUSE : aiTextureType_NONE;
			
			load_texture(texture_base_colour_type);
			load_texture(aiTextureType_METALNESS);
			load_texture(aiTextureType_NORMALS);
			
			if (asset_material) {

				auto& component = model_prefab->GetComponent<MeshRendererComponent>(current_entity_handle);
				component.MeshRendererMaterialHandles.push_back({ asset_material_handle, nullptr });

				asset_material->Handle = asset_material_handle;

				asset_map->operator[](asset_material_handle) = asset_material;
				asset_reg->operator[](asset_material_handle) = material_metadata;
			}

		}
		else 
		{
			model_prefab->GetComponent<MeshRendererComponent>(current_entity_handle).MeshRendererMaterialHandles.push_back({ asset_material_handle, nullptr });
		}

	}

	void ModelImporter::ProcessNode(const aiScene* scene, aiNode* node, std::shared_ptr<Prefab> model_prefab, entt::entity parent_entity_handle, AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle parent_asset_handle, const AssetMetaData& parent_meta_data, const std::filesystem::path& path)
	{
		entt::entity current_entity_handle = entt::null;

		// Check if the current node is the root node and has children,
		// if the root node has children, we want to have a root node 
		// that all children attach to which will be the root node.
		if (node == scene->mRootNode && node->mNumChildren > 1) 
		{
			current_entity_handle = model_prefab->GetRootEntity();
			model_prefab->SetPrefabName(path.stem().string());
		}

		if (node->mNumMeshes > 0) 
		{
			// If the Current Entity Handle has not been set
			if (current_entity_handle == entt::null) 
			{
				
				if (parent_entity_handle == entt::null) // If there is no parent, get the Root Entity of the Prefab
				{
					current_entity_handle = model_prefab->GetRootEntity();
					model_prefab->SetPrefabName(path.stem().string());
				}
				else // If there is a parent, we want to create a new sub entity and attach it to that parent in the Prefab registry
				{
					current_entity_handle = model_prefab->CreateEntity(node->mName.C_Str());
					model_prefab->GetComponent<HierarchyComponent>(current_entity_handle).m_Parent = (uint32_t)parent_entity_handle;
					model_prefab->GetComponent<HierarchyComponent>(parent_entity_handle).m_Children.push_back((uint32_t)current_entity_handle);
				}

			}

			// Generate Sub Asset Handle
			AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
				AssetUtils::AssetTypeToString(AssetType::Mesh) + path.filename().string() + node->mName.C_Str()
			));

			std::shared_ptr<AssetMesh> asset_mesh = std::make_shared<AssetMesh>();
			asset_mesh->Handle = handle;

			// If this Sub Asset Has not been loaded yet
			if (asset_map->count(handle) == 0) 
			{
				glm::mat4 local_transformation = glm::mat4(
					node->mTransformation.a1, node->mTransformation.b1, node->mTransformation.c1, node->mTransformation.d1,
					node->mTransformation.a2, node->mTransformation.b2, node->mTransformation.c2, node->mTransformation.d2,
					node->mTransformation.a3, node->mTransformation.b3, node->mTransformation.c3, node->mTransformation.d3,
					node->mTransformation.a4, node->mTransformation.b4, node->mTransformation.c4, node->mTransformation.d4
				);
				model_prefab->GetComponent<TransformComponent>(current_entity_handle).SetTransform(local_transformation);
				model_prefab->AddComponent<MeshFilterComponent>(current_entity_handle).MeshFilterAssetHandle = handle;
				model_prefab->AddComponent<MeshRendererComponent>(current_entity_handle);

				AssetMetaData metadata;

				metadata.Type = AssetType::Mesh;
				metadata.AssetName = node->mName.C_Str();
				metadata.ParentAssetHandle = parent_asset_handle;
				metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetAssetDirectory());
				metadata.IsCustomAsset = parent_meta_data.IsCustomAsset;

				// Process Meshes of the Node
				for (unsigned int i = 0; i < node->mNumMeshes; i++) {

					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
					ProcessMesh(scene, mesh, model_prefab, current_entity_handle, asset_mesh, asset_map, asset_reg, parent_asset_handle, parent_meta_data, path);

					// Calculate the the AABB of the mesh including any sub meshes
					// Update bounds to include the current submesh
					asset_mesh->MeshBounds.BoundsMax = glm::max(asset_mesh->MeshBounds.BoundsMax,
						glm::vec3{
							mesh->mAABB.mMax.x,
							mesh->mAABB.mMax.y,
							mesh->mAABB.mMax.z,
						});

					asset_mesh->MeshBounds.BoundsMin = glm::min(asset_mesh->MeshBounds.BoundsMin,
						glm::vec3{
							mesh->mAABB.mMin.x,
							mesh->mAABB.mMin.y,
							mesh->mAABB.mMin.z,
						});
				}

				if (asset_mesh) {
					asset_map->operator[](handle) = asset_mesh;
					asset_reg->operator[](handle) = metadata;
				}
				else {
					model_prefab->RemoveComponent<MeshFilterComponent>(current_entity_handle);
					model_prefab->RemoveComponent<MeshRendererComponent>(current_entity_handle);
				}
			}
		}

		// Process Any Children Nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			ProcessNode(scene, node->mChildren[i], model_prefab, current_entity_handle, asset_map, asset_reg, parent_asset_handle, parent_meta_data, path);
		}
	}

#pragma endregion

#pragma region Compute Shader Import

	std::shared_ptr<Shader> ShaderImporter::ImportShader(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory)
	{
		return LoadShader(meta_data.IsCustomAsset ? meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / meta_data.FilePath);
	}

	std::shared_ptr<Shader> ShaderImporter::LoadShader(const std::filesystem::path& path)
	{
		if(AssetManager::GetAssetTypeFromFileExtension(path.extension()) == AssetType::Compute_Shader)
			return std::make_shared<Shader>(path.string().c_str(), true);
		else if (AssetManager::GetAssetTypeFromFileExtension(path.extension()) == AssetType::Shader)
			return std::make_shared<Shader>(path.string().c_str(), false);

		return nullptr;
	}

	std::shared_ptr<ComputeShaderAsset> ShaderImporter::ImportComputeShader(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory)
	{
		return LoadComputeShader(meta_data.IsCustomAsset ? meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / meta_data.FilePath);
	}

	std::shared_ptr<ComputeShaderAsset> ShaderImporter::LoadComputeShader(const std::filesystem::path& path) 
	{
		return std::make_shared<ComputeShaderAsset>(path);
	}

#pragma endregion

}