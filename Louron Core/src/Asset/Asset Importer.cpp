#include "Asset Importer.h"

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

	using AssetImportFunction = std::function<std::shared_ptr<Asset>(AssetMap*, AssetRegistry*, AssetHandle, const AssetMetaData&)>;
	static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {

		//{ AssetType::Audio, AudioImporter::ImportAudio },
		{ AssetType::Prefab,					PrefabImporter::ImportPrefab },
		{ AssetType::Scene,						SceneImporter::ImportScene },

		{ AssetType::Texture2D,					TextureImporter::ImportTexture2D },

		{ AssetType::Material_Standard,			MaterialImporter::ImportMaterial },
		{ AssetType::Material_Skybox,			MaterialImporter::ImportMaterial },

		{ AssetType::ModelImport,				ModelImporter::ImportModel }
	};

	std::shared_ptr<Asset> AssetImporter::ImportAsset(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& metadata)
	{
		if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end())
		{
			L_CORE_ERROR("No importer available for asset type: {0}", AssetTypeToString(metadata.Type));
			return nullptr;
		}

		return s_AssetImportFunctions.at(metadata.Type)(asset_map, asset_reg, handle, metadata);
	}

#pragma region Scene Import


	std::shared_ptr<Scene> SceneImporter::ImportScene(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data) {
		std::string profile_name = "Scene Import: " + meta_data.FilePath.string();
		L_PROFILE_SCOPE(profile_name.c_str());

		auto project = Project::GetActiveProject();
		return LoadScene(asset_map, asset_reg, project->GetProjectDirectory() / project->GetConfig().AssetDirectory / meta_data.FilePath);
	}

	std::shared_ptr<Scene> SceneImporter::LoadScene(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path) {

		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		SceneSerializer serializer(scene);
		serializer.Deserialize(path);
		return scene;
	}

	void SceneImporter::SaveScene(std::shared_ptr<Scene> scene, const std::filesystem::path& path) {

		SceneSerializer serializer(scene);
		auto project = Project::GetActiveProject();
		serializer.Serialize(project->GetProjectDirectory() / project->GetConfig().AssetDirectory / path);
	}

#pragma endregion

#pragma region Texture Import

	std::shared_ptr<Texture> TextureImporter::ImportTexture2D(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data) {
		L_PROFILE_SCOPE("Texture Import");

		auto project = Project::GetActiveProject();
		return LoadTexture2D(project->GetProjectDirectory() / project->GetConfig().AssetDirectory / meta_data.FilePath);
	}

	std::shared_ptr<Texture> TextureImporter::LoadTexture2D(const std::filesystem::path& path) {
		return std::make_shared<Texture>(path);
	}


#pragma endregion

#pragma region Material Import

	std::shared_ptr<Material> MaterialImporter::ImportMaterial(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data) {

		if (meta_data.FilePath.extension() == ".lmat" || meta_data.FilePath.extension() == ".lskybox") {

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

			if (data["Material Asset Type"].as<std::string>() == AssetTypeToString(AssetType::Material_Skybox)) {
				return LoadMaterialSkybox(asset_map, asset_reg, meta_data.FilePath);
			}

			if (data["Material Asset Type"].as<std::string>() == AssetTypeToString(AssetType::Material_Standard)) {
				return LoadMaterialPBR(data);
			}
		}

		L_CORE_WARN("Incompatible Material File Extension");
		L_CORE_WARN("Extension Used: {0}", meta_data.FilePath.extension().string());
		L_CORE_WARN("Extension Expected: .lmaterial");

		return nullptr;
	}

	std::shared_ptr<PBRMaterial> MaterialImporter::LoadMaterialPBR(const YAML::Node data) {

		std::shared_ptr<PBRMaterial> material = std::make_shared<PBRMaterial>();

		material->Deserialize(data);

		return material;
	}

	std::shared_ptr<SkyboxMaterial> MaterialImporter::LoadMaterialSkybox(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path) {

		std::shared_ptr<SkyboxMaterial> material = std::make_shared<SkyboxMaterial>();

		if (material->Deserialize(Project::GetActiveProject()->GetProjectDirectory() / "Assets" / path)) {
			return material;
		}

		return nullptr;
	}

#pragma endregion

#pragma region Prefab File Import

	std::shared_ptr<Prefab> PrefabImporter::ImportPrefab(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& metadata)
	{
		return LoadPrefab(metadata.FilePath);
	}

	std::shared_ptr<Prefab> PrefabImporter::LoadPrefab(const std::filesystem::path& path)
	{
		std::shared_ptr<Prefab> prefab = std::make_shared<Prefab>();
		if (prefab->Deserialize(Project::GetActiveProject()->GetProjectDirectory() / "Assets" / path)) {
			return prefab;
		}

		return nullptr;
	}

#pragma endregion

#pragma region Model Import

	// Function to resolve the texture path relative to the projects asset directory
	static std::string ResolveTexturePath(const std::string& texturePath, const std::string& modelFilePath, const std::string& projectDirectory) {
		namespace fs = std::filesystem;

		// 1. Get the directory of the model file
		fs::path modelDir = fs::path(modelFilePath).parent_path();

		// 2. Combine the model directory with the texture path
		fs::path textureFilePath = fs::absolute(modelDir / texturePath);

		// 3. If the texture path is relative, resolve it relative to the project directory
		if (textureFilePath.is_relative()) {
			textureFilePath = fs::absolute(fs::path(projectDirectory) / texturePath);
		}

		// 4. Check if the resolved texture path exists
		if (fs::exists(textureFilePath)) {
			return textureFilePath.string();
		}
		else {
			L_CORE_ERROR("Could Not Resolve Texture File Path - Cannot Load ASSIMP Model Texture Reference: {0}", texturePath.c_str());
			return ""; // Or some error indication
		}
	}

	std::shared_ptr<Prefab> ModelImporter::ImportModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data)
	{
		auto project = Project::GetActiveProject();
		return LoadModel(asset_map, asset_reg, handle, project->GetProjectDirectory() / project->GetConfig().AssetDirectory / meta_data.FilePath);
	}

	std::shared_ptr<Prefab> ModelImporter::LoadModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const std::filesystem::path& path)
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
			return GL_FALSE;
		}
		if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
			L_CORE_ERROR("Cannot Import Model - Assimp Scene Data Structure Incomplete. Potentially Corrupted File.");
			return GL_FALSE;
		}
		if (!scene->mRootNode) {
			L_CORE_ERROR("Cannot Import Model - Assimp Scene Has No Root Node");
			return GL_FALSE;
		}

		std::shared_ptr<Prefab> model_prefab = std::make_shared<Prefab>();
		model_prefab->SetMutable(false);

		ProcessNode(scene, scene->mRootNode, model_prefab, entt::null, asset_map, asset_reg, handle, path);

		return model_prefab;
	}

	void ModelImporter::ProcessMesh(const aiScene* scene, aiMesh* mesh, std::shared_ptr<Prefab> model_prefab, entt::entity current_entity_handle, std::shared_ptr<AssetMesh> asset_mesh, AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle parent_asset_handle, const std::filesystem::path& path)
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

			std::string(AssetTypeToString(AssetType::Material_Standard)) + AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory)) + materialName.C_Str()
			));

		// If the Material has not already been loaded into the asset map, we 
		// want to create a new material. Once this is done, we won't need to do this again 
		// for this material.
		if (asset_map->count(asset_material_handle) == 0) {

			AssetMetaData material_metadata;
			material_metadata.FilePath = AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory));
			material_metadata.Type = AssetType::Material_Standard;
			material_metadata.AssetName = materialName.C_Str();
			material_metadata.ParentAssetHandle = parent_asset_handle;

			std::shared_ptr<PBRMaterial> asset_material = std::make_shared<PBRMaterial>();

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
			aiTextureType texture_base_colour_type = material->GetTextureCount(aiTextureType_BASE_COLOR) > 0 ? aiTextureType_BASE_COLOR : material->GetTextureCount(aiTextureType_DIFFUSE) > 0 ? aiTextureType_DIFFUSE : aiTextureType_NONE;
			if (texture_base_colour_type != aiTextureType_NONE) {

				aiString assimp_texture_string;
				material->GetTexture(texture_base_colour_type, 0, &assimp_texture_string);
				std::filesystem::path resolved_texture_path = ResolveTexturePath(assimp_texture_string.C_Str(), path.string(), (project->GetProjectDirectory() / project->GetConfig().AssetDirectory).string());
				resolved_texture_path = std::filesystem::relative(resolved_texture_path, std::filesystem::current_path());

				if (auto material_texture_ref = scene->GetEmbeddedTexture(assimp_texture_string.C_Str())) {

					AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
						std::string(AssetTypeToString(AssetType::Texture2D)) + AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory)) + assimp_texture_string.C_Str()
						));

					if (asset_map->count(asset_texture_handle) == 0) {

						AssetMetaData texture_metadata;
						texture_metadata.FilePath = AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory));
						texture_metadata.Type = AssetType::Texture2D;
						texture_metadata.AssetName = assimp_texture_string.C_Str();
						texture_metadata.ParentAssetHandle = parent_asset_handle;

						glm::ivec2 texture_size = { material_texture_ref->mWidth, material_texture_ref->mHeight };
						GLubyte* texture_data = reinterpret_cast<GLubyte*>(material_texture_ref->pcData);

						std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(texture_data, texture_size, GL_BGRA);
						if (asset_texture) {

							asset_material->SetAlbedoTexture(asset_texture_handle);

							asset_texture->Handle = asset_texture_handle;

							asset_map->operator[](asset_texture_handle) = asset_texture;
							asset_reg->operator[](asset_texture_handle) = texture_metadata;
						}
					}
					else {
						asset_material->SetAlbedoTexture(asset_texture_handle);
					}

				}
				else if (std::filesystem::exists(resolved_texture_path)) {

					AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
						std::string(AssetTypeToString(AssetType::Texture2D)) + AssetManager::NormalisePath(std::filesystem::relative(resolved_texture_path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory))
						));

					if (asset_map->count(asset_texture_handle) == 0) {

						AssetMetaData texture_metadata;
						texture_metadata.FilePath = AssetManager::NormalisePath(std::filesystem::relative(resolved_texture_path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory));
						texture_metadata.Type = AssetType::Texture2D;
						texture_metadata.AssetName = resolved_texture_path.filename().replace_extension().string();
						texture_metadata.ParentAssetHandle = NULL_UUID; // We explictly make sure this is NULL as this type of asset does not need the model to be loaded in order to load this texture!

						std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(resolved_texture_path);
						if (asset_texture) {

							asset_material->SetAlbedoTexture(asset_texture_handle);

							asset_texture->Handle = asset_texture_handle;

							asset_map->operator[](asset_texture_handle) = asset_texture;
							asset_reg->operator[](asset_texture_handle) = texture_metadata;
						}
					}
					else {
						asset_material->SetAlbedoTexture(asset_texture_handle);
					}

				}

			}

			if (material->GetTextureCount(aiTextureType_METALNESS) > 0) {

				aiString assimp_texture_string;
				material->GetTexture(aiTextureType_METALNESS, 0, &assimp_texture_string);
				std::filesystem::path resolved_texture_path = ResolveTexturePath(assimp_texture_string.C_Str(), path.string(), (project->GetProjectDirectory() / project->GetConfig().AssetDirectory).string());
				resolved_texture_path = std::filesystem::relative(resolved_texture_path, std::filesystem::current_path());

				if (auto material_texture_ref = scene->GetEmbeddedTexture(assimp_texture_string.C_Str())) {

					AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
						std::string(AssetTypeToString(AssetType::Texture2D)) + AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory)) + assimp_texture_string.C_Str()
						));

					if (asset_map->count(asset_texture_handle) == 0) {

						AssetMetaData texture_metadata;
						texture_metadata.FilePath = AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory));
						texture_metadata.Type = AssetType::Texture2D;
						texture_metadata.AssetName = assimp_texture_string.C_Str();
						texture_metadata.ParentAssetHandle = parent_asset_handle;

						glm::ivec2 texture_size = { material_texture_ref->mWidth, material_texture_ref->mHeight };
						GLubyte* texture_data = reinterpret_cast<GLubyte*>(material_texture_ref->pcData);

						std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(texture_data, texture_size, GL_BGRA);
						if (asset_texture) {

							asset_material->SetMetallicTexture(asset_texture_handle);

							asset_texture->Handle = asset_texture_handle;

							asset_map->operator[](asset_texture_handle) = asset_texture;
							asset_reg->operator[](asset_texture_handle) = texture_metadata;
						}
					}
					else {
						asset_material->SetMetallicTexture(asset_texture_handle);
					}

				}
				else if (std::filesystem::exists(resolved_texture_path)) {

					AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
						std::string(AssetTypeToString(AssetType::Texture2D)) + AssetManager::NormalisePath(std::filesystem::relative(resolved_texture_path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory))
						));

					if (asset_map->count(asset_texture_handle) == 0) {

						AssetMetaData texture_metadata;
						texture_metadata.FilePath = AssetManager::NormalisePath(std::filesystem::relative(resolved_texture_path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory));
						texture_metadata.Type = AssetType::Texture2D;
						texture_metadata.AssetName = resolved_texture_path.filename().replace_extension().string();
						texture_metadata.ParentAssetHandle = NULL_UUID; // We explictly make sure this is NULL as this type of asset does not need the model to be loaded in order to load this texture!

						std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(resolved_texture_path);
						if (asset_texture) {

							asset_material->SetMetallicTexture(asset_texture_handle);

							asset_texture->Handle = asset_texture_handle;

							asset_map->operator[](asset_texture_handle) = asset_texture;
							asset_reg->operator[](asset_texture_handle) = texture_metadata;
						}
					}
					else {
						asset_material->SetMetallicTexture(asset_texture_handle);
					}

				}
			}

			if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {

				aiString assimp_texture_string;
				material->GetTexture(aiTextureType_NORMALS, 0, &assimp_texture_string);
				std::filesystem::path resolved_texture_path = ResolveTexturePath(assimp_texture_string.C_Str(), path.string(), (project->GetProjectDirectory() / project->GetConfig().AssetDirectory).string());
				resolved_texture_path = std::filesystem::relative(resolved_texture_path, std::filesystem::current_path());

				if (auto material_texture_ref = scene->GetEmbeddedTexture(assimp_texture_string.C_Str())) {

					AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
						std::string(AssetTypeToString(AssetType::Texture2D)) + AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory)) + assimp_texture_string.C_Str()
						));

					if (asset_map->count(asset_texture_handle) == 0) {

						AssetMetaData texture_metadata;
						texture_metadata.FilePath = AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory));
						texture_metadata.Type = AssetType::Texture2D;
						texture_metadata.AssetName = assimp_texture_string.C_Str();
						texture_metadata.ParentAssetHandle = parent_asset_handle;

						glm::ivec2 texture_size = { material_texture_ref->mWidth, material_texture_ref->mHeight };
						GLubyte* texture_data = reinterpret_cast<GLubyte*>(material_texture_ref->pcData);

						std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(texture_data, texture_size, GL_BGRA);
						if (asset_texture) {

							asset_material->SetNormalTexture(asset_texture_handle);

							asset_texture->Handle = asset_texture_handle;

							asset_map->operator[](asset_texture_handle) = asset_texture;
							asset_reg->operator[](asset_texture_handle) = texture_metadata;
						}
					}
					else {
						asset_material->SetNormalTexture(asset_texture_handle);
					}

				}
				else if (std::filesystem::exists(resolved_texture_path)) {
					AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
						std::string(AssetTypeToString(AssetType::Texture2D)) + AssetManager::NormalisePath(std::filesystem::relative(resolved_texture_path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory))
						));

					if (asset_map->count(asset_texture_handle) == 0) {

						AssetMetaData texture_metadata;
						texture_metadata.FilePath = AssetManager::NormalisePath(std::filesystem::relative(resolved_texture_path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory));
						texture_metadata.Type = AssetType::Texture2D;
						texture_metadata.AssetName = resolved_texture_path.filename().replace_extension().string();
						texture_metadata.ParentAssetHandle = NULL_UUID; // We explictly make sure this is NULL as this type of asset does not need the model to be loaded in order to load this texture!

						std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(resolved_texture_path);
						if (asset_texture) {

							asset_material->SetNormalTexture(asset_texture_handle);

							asset_texture->Handle = asset_texture_handle;

							asset_map->operator[](asset_texture_handle) = asset_texture;
							asset_reg->operator[](asset_texture_handle) = texture_metadata;
						}
					}
					else {
						asset_material->SetNormalTexture(asset_texture_handle);
					}

				}
			}

			if (asset_material) {

				model_prefab->GetComponent<AssetMeshRenderer>(current_entity_handle).MeshRendererMaterialHandles.push_back(asset_material_handle);

				asset_material->Handle = asset_material_handle;

				asset_map->operator[](asset_material_handle) = asset_material;
				asset_reg->operator[](asset_material_handle) = material_metadata;
			}

		}
		else {

			model_prefab->GetComponent<AssetMeshRenderer>(current_entity_handle).MeshRendererMaterialHandles.push_back(asset_material_handle);

		}

	}

	void ModelImporter::ProcessNode(const aiScene* scene, aiNode* node, std::shared_ptr<Prefab> model_prefab, entt::entity parent_entity_handle, AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle parent_asset_handle, const std::filesystem::path& path)
	{
		entt::entity current_entity_handle = entt::null;

		// Check if the current note is the root node, if so we will just get the Root Entity of the current prefab and update it's name to the mesh
		if (node == scene->mRootNode && node->mNumChildren > 1) {
			current_entity_handle = model_prefab->GetRootEntity();
			model_prefab->SetPrefabName(path.filename().replace_extension().string());
		}

		if (node->mNumMeshes > 0) {

			if (current_entity_handle == entt::null && parent_entity_handle == entt::null) {
				current_entity_handle = model_prefab->GetRootEntity();
				model_prefab->SetPrefabName(path.filename().replace_extension().string());
			}

			if (current_entity_handle == entt::null)
				current_entity_handle = model_prefab->CreateEntity(node->mName.C_Str());


			if (parent_entity_handle != entt::null)
			{
				model_prefab->GetComponent<HierarchyComponent>(model_prefab->FindEntityByUUID((uint32_t)current_entity_handle)).m_Parent = (uint32_t)parent_entity_handle;
				model_prefab->GetComponent<HierarchyComponent>(model_prefab->FindEntityByUUID((uint32_t)parent_entity_handle)).m_Children.push_back((uint32_t)current_entity_handle);
			}

			std::shared_ptr<Asset> asset_mesh = std::make_shared<AssetMesh>();
			std::shared_ptr<AssetMesh> asset_mesh_casted = std::static_pointer_cast<AssetMesh>(asset_mesh);

			auto project = Project::GetActiveProject();
			AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
				std::string(AssetTypeToString(AssetType::Mesh)) + AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory)) + node->mName.C_Str()
				));

			if (asset_map->count(handle) == 0) {

				model_prefab->AddComponent<AssetMeshFilter>(current_entity_handle).MeshFilterAssetHandle = handle;
				model_prefab->AddComponent<AssetMeshRenderer>(current_entity_handle);

				AssetMetaData metadata;


				if (std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory).empty())
					metadata.FilePath = path;
				else
					metadata.FilePath = AssetManager::NormalisePath(std::filesystem::relative(path, project->GetProjectDirectory() / project->GetConfig().AssetDirectory));

				metadata.Type = AssetType::Mesh;
				metadata.AssetName = node->mName.C_Str();
				metadata.ParentAssetHandle = parent_asset_handle;

				// Process Meshes of the Node
				for (unsigned int i = 0; i < node->mNumMeshes; i++) {

					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
					ProcessMesh(scene, mesh, model_prefab, current_entity_handle, asset_mesh_casted, asset_map, asset_reg, parent_asset_handle, path);

					// Calculate the the AABB of the mesh including any sub meshes
					if (i == 0) { // Set the initial bounds based on the first submesh
						asset_mesh_casted->MeshBounds.BoundsMax =
						{
							mesh->mAABB.mMax.x,
							mesh->mAABB.mMax.y,
							mesh->mAABB.mMax.z,
						};

						asset_mesh_casted->MeshBounds.BoundsMin =
						{
							mesh->mAABB.mMin.x,
							mesh->mAABB.mMin.y,
							mesh->mAABB.mMin.z,
						};
					}
					else { // Update bounds to include the current submesh

						asset_mesh_casted->MeshBounds.BoundsMax = glm::max(asset_mesh_casted->MeshBounds.BoundsMax,
							glm::vec3{
								mesh->mAABB.mMax.x,
								mesh->mAABB.mMax.y,
								mesh->mAABB.mMax.z,
							});

						asset_mesh_casted->MeshBounds.BoundsMin = glm::min(asset_mesh_casted->MeshBounds.BoundsMin,
							glm::vec3{
								mesh->mAABB.mMin.x,
								mesh->mAABB.mMin.y,
								mesh->mAABB.mMin.z,
							});
					}
				}

				if (asset_mesh) {
					asset_mesh->Handle = handle;
					asset_map->operator[](handle) = asset_mesh;
					asset_reg->operator[](handle) = metadata;
				}
				else {
					model_prefab->GetComponent<AssetMeshFilter>(current_entity_handle).MeshFilterAssetHandle = NULL_UUID;
				}
			}
		}

		// Process Any Children Nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			ProcessNode(scene, node->mChildren[i], model_prefab, current_entity_handle, asset_map, asset_reg, parent_asset_handle, path);
		}
	}

#pragma endregion

}