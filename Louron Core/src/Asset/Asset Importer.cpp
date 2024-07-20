#include "Asset Importer.h"

#include "../Project/Project.h"

#include "../Debug/Profiler.h"

#include <map>
#include <memory>
#include <functional>

// External Vendor Library Headers
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

		//{ AssetType::Model3D, ModelImporter::ImportModel3D },
		//{ AssetType::Audio, AudioImporter::ImportAudio },
		//{ AssetType::Prefab, PrefabImporter::ImportPrefab },
		{ AssetType::Scene,						SceneImporter::ImportScene },

		{ AssetType::Texture2D,					TextureImporter::ImportTexture2D },

		{ AssetType::Material_Standard,			MaterialImporter::ImportMaterial },

		{ AssetType::ModelImport,				MeshImporter::ImportModel },
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

		return LoadScene(asset_map, asset_reg, Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath);
	}

	std::shared_ptr<Scene> SceneImporter::LoadScene(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path) {

		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		SceneSerializer serializer(scene);
		serializer.Deserialize(path);
		return scene;
	}

	void SceneImporter::SaveScene(std::shared_ptr<Scene> scene, const std::filesystem::path& path) {

		SceneSerializer serializer(scene);
		serializer.Serialize(Project::GetActiveProject()->GetConfig().AssetDirectory / path);
	}

#pragma endregion

#pragma region Texture Import

	std::shared_ptr<Texture> TextureImporter::ImportTexture2D(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data) {
		L_PROFILE_SCOPE("Texture Import");

		return LoadTexture2D(Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath);
	}

	std::shared_ptr<Texture> TextureImporter::LoadTexture2D(const std::filesystem::path& path) {
		return std::make_shared<Texture>(path);
	}


#pragma endregion

#pragma region Material Import

	std::shared_ptr<Material> MaterialImporter::ImportMaterial(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data) {

		if (meta_data.FilePath.extension() != ".lmaterial") {

			L_CORE_WARN("Incompatible Material File Extension");
			L_CORE_WARN("Extension Used: {0}", meta_data.FilePath.extension().string());
			L_CORE_WARN("Extension Expected: .lmaterial");
		}
		else
		{
			YAML::Node data;

			try {
				data = YAML::LoadFile((Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath).string());
			}
			catch (YAML::ParserException e) {
				L_CORE_ERROR("YAML-CPP Failed to Load Scene File: '{0}', {1}", meta_data.FilePath.string(), e.what());
				return nullptr;
			}

			if (!data["Material Type"]) {
				L_CORE_ERROR("Material Type Node is Not Specified in File: '{0}'", meta_data.FilePath.string());
				return nullptr;
			}

			if (data["Material Type"].as<std::string>() == "SkyboxMaterial") {
				return LoadMaterialSkybox(asset_map, asset_reg, Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath);
			}

			if (data["Material Type"].as<std::string>() == "BPMaterial") {
				return LoadMaterialBP(asset_map, asset_reg, Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath);
			}

			if (data["Material Type"].as<std::string>() == "PBRMaterial") {
				return LoadMaterialPBR(asset_map, asset_reg, Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath);
			}

		}
		return nullptr;
	}

	std::shared_ptr<BPMaterial> MaterialImporter::LoadMaterialBP(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path) {

		std::shared_ptr<BPMaterial> material = std::make_shared<BPMaterial>();



		return std::shared_ptr<BPMaterial>();
	}

	std::shared_ptr<PBRMaterial> MaterialImporter::LoadMaterialPBR(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path) {

		std::shared_ptr<PBRMaterial> material = std::make_shared<PBRMaterial>();



		return std::shared_ptr<PBRMaterial>();
	}

	std::shared_ptr<SkyboxMaterial> MaterialImporter::LoadMaterialSkybox(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path) {

		std::shared_ptr<SkyboxMaterial> material = std::make_shared<SkyboxMaterial>();
		if (material->Deserialize(path)) {
			return material;
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
			//textureFilePath = fs::relative(textureFilePath, projectDirectory);
			return textureFilePath.string();
		}
		else {
			L_CORE_ERROR("Could Not Resolve Texture File Path - Cannot Load ASSIMP Model Texture Reference: {0}", texturePath.c_str());
			return ""; // Or some error indication
		}
	}

	std::shared_ptr<Prefab> MeshImporter::ImportModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data)
	{
		return LoadModel(asset_map, asset_reg, Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath);
	}

	std::shared_ptr<Prefab> MeshImporter::LoadModel(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path)
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
			L_CORE_ERROR("Cannot Import Model - Assimp Import Error: {0}", importer.GetErrorString());
			return GL_FALSE;
		}

		std::shared_ptr<Prefab> model_prefab = std::make_shared<Prefab>();

		std::function<std::shared_ptr<Texture>()> load_texture;

		std::function<void(aiMesh*, std::shared_ptr<AssetMesh>)> process_mesh = [&](aiMesh* mesh, std::shared_ptr<AssetMesh> asset_mesh) -> void
			{

				// 1. Process Vertices
				std::vector<Vertex> mesh_vertices;
				for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

					Vertex vertex{};
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

				// 3. Create Individual Mesh Asset
				asset_mesh->m_SubMeshes.push_back({ mesh_vertices, mesh_indices });

				// 4. Create Material Asset
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				aiString materialName;
				material->Get(AI_MATKEY_NAME, materialName);

				AssetHandle asset_material_handle = static_cast<uint32_t>(std::hash<std::string>{}(
					std::string(AssetTypeToString(AssetType::Material_Standard)) + path.string() + materialName.C_Str()
				));

				// If the Material has not already been loaded into the asset map, we 
				// want to create a new material. Once this is done, we won't need to do this again 
				// for this material.
				if(asset_map->count(asset_material_handle) == 0) {

					AssetMetaData material_metadata;
					material_metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetConfig().AssetDirectory);
					material_metadata.Type = AssetType::Material_Standard;

					std::shared_ptr<PBRMaterial> asset_material = std::make_shared<PBRMaterial>();

					asset_material->SetName(materialName.C_Str());

					// Load Material Values
					
					ai_real temp{};
					aiColor4D colour{};

					if(material->Get(AI_MATKEY_BASE_COLOR, colour) == aiReturn_SUCCESS)
						asset_material->SetAlbedoTintColour({ colour.r, colour.g, colour.b, colour.a });
					else if(material->Get(AI_MATKEY_COLOR_DIFFUSE, colour) == aiReturn_SUCCESS)
						asset_material->SetAlbedoTintColour({ colour.r, colour.g, colour.b, colour.a });

					if(material->Get(AI_MATKEY_METALLIC_FACTOR, temp) == aiReturn_SUCCESS)
						asset_material->SetMetallic(temp);

					if(material->Get(AI_MATKEY_ROUGHNESS_FACTOR, temp) == aiReturn_SUCCESS)
						asset_material->SetRoughness(temp);

					// Load Material Textures
					aiTextureType texture_base_colour_type = material->GetTextureCount(aiTextureType_BASE_COLOR) > 0 ? aiTextureType_BASE_COLOR : material->GetTextureCount(aiTextureType_DIFFUSE) > 0 ? aiTextureType_DIFFUSE : aiTextureType_NONE;
					if(texture_base_colour_type != aiTextureType_NONE) {

						aiString assimp_texture_string;
						material->GetTexture(texture_base_colour_type, 0, &assimp_texture_string);
						std::filesystem::path resolved_texture_path = ResolveTexturePath(assimp_texture_string.C_Str(), path.string(), Project::GetActiveProject()->GetConfig().AssetDirectory.string());
						resolved_texture_path = std::filesystem::relative(resolved_texture_path, std::filesystem::current_path());

						if (auto material_texture_ref = scene->GetEmbeddedTexture(assimp_texture_string.C_Str())) {

							AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
								std::string(AssetTypeToString(AssetType::Texture2D)) + path.string() + assimp_texture_string.C_Str()
							));

							if (asset_map->count(asset_texture_handle) == 0) {

								AssetMetaData texture_metadata;
								texture_metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetConfig().AssetDirectory);
								texture_metadata.Type = AssetType::Texture2D;

								glm::ivec2 texture_size = { material_texture_ref->mWidth, material_texture_ref->mHeight };
								GLubyte* texture_data = reinterpret_cast<GLubyte*>(material_texture_ref->pcData);

								std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(texture_data, texture_size, GL_BGRA);
								asset_material->SetAlbedoTexture(asset_texture);

								asset_texture->Handle = asset_texture_handle;

								asset_map->operator[](asset_texture_handle) = asset_texture;
								asset_reg->operator[](asset_texture_handle) = texture_metadata;
							}

						} 
						else if (std::filesystem::exists(resolved_texture_path)) {

							AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
								std::string(AssetTypeToString(AssetType::Texture2D)) + std::filesystem::relative(resolved_texture_path, Project::GetActiveProject()->GetConfig().AssetDirectory).string()
							));

							if (asset_map->count(asset_texture_handle) == 0) {

								AssetMetaData texture_metadata;
								texture_metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetConfig().AssetDirectory);
								texture_metadata.Type = AssetType::Texture2D;

								std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(resolved_texture_path);
								asset_material->SetAlbedoTexture(asset_texture);

								asset_texture->Handle = asset_texture_handle;

								asset_map->operator[](asset_texture_handle) = asset_texture;
								asset_reg->operator[](asset_texture_handle) = texture_metadata;
							}

						}

					}

					if (material->GetTextureCount(aiTextureType_METALNESS) > 0) {

						aiString assimp_texture_string;
						material->GetTexture(aiTextureType_METALNESS, 0, &assimp_texture_string);
						std::filesystem::path resolved_texture_path = ResolveTexturePath(assimp_texture_string.C_Str(), path.string(), Project::GetActiveProject()->GetConfig().AssetDirectory.string());
						resolved_texture_path = std::filesystem::relative(resolved_texture_path, std::filesystem::current_path());

						if (auto material_texture_ref = scene->GetEmbeddedTexture(assimp_texture_string.C_Str())) {

							AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
								std::string(AssetTypeToString(AssetType::Texture2D)) + path.string() + assimp_texture_string.C_Str()
								));

							if (asset_map->count(asset_texture_handle) == 0) {

								AssetMetaData texture_metadata;
								texture_metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetConfig().AssetDirectory);
								texture_metadata.Type = AssetType::Texture2D;

								glm::ivec2 texture_size = { material_texture_ref->mWidth, material_texture_ref->mHeight };
								GLubyte* texture_data = reinterpret_cast<GLubyte*>(material_texture_ref->pcData);

								std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(texture_data, texture_size, GL_BGRA);
								asset_material->SetMetallicTexture(asset_texture);

								asset_texture->Handle = asset_texture_handle;

								asset_map->operator[](asset_texture_handle) = asset_texture;
								asset_reg->operator[](asset_texture_handle) = texture_metadata;
							}

						}
						else if (std::filesystem::exists(resolved_texture_path)) {

							AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
								std::string(AssetTypeToString(AssetType::Texture2D)) + std::filesystem::relative(resolved_texture_path, Project::GetActiveProject()->GetConfig().AssetDirectory).string()
								));

							if (asset_map->count(asset_texture_handle) == 0) {

								AssetMetaData texture_metadata;
								texture_metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetConfig().AssetDirectory);
								texture_metadata.Type = AssetType::Texture2D;

								std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(resolved_texture_path);
								asset_material->SetMetallicTexture(asset_texture);

								asset_texture->Handle = asset_texture_handle;

								asset_map->operator[](asset_texture_handle) = asset_texture;
								asset_reg->operator[](asset_texture_handle) = texture_metadata;
							}

						}
					}

					if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {

						aiString assimp_texture_string;
						material->GetTexture(aiTextureType_NORMALS, 0, &assimp_texture_string);
						std::filesystem::path resolved_texture_path = ResolveTexturePath(assimp_texture_string.C_Str(), path.string(), Project::GetActiveProject()->GetConfig().AssetDirectory.string());
						resolved_texture_path = std::filesystem::relative(resolved_texture_path, std::filesystem::current_path());

						if (auto material_texture_ref = scene->GetEmbeddedTexture(assimp_texture_string.C_Str())) {

							AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
								std::string(AssetTypeToString(AssetType::Texture2D)) + path.string() + assimp_texture_string.C_Str()
								));

							if (asset_map->count(asset_texture_handle) == 0) {

								AssetMetaData texture_metadata;
								texture_metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetConfig().AssetDirectory);
								texture_metadata.Type = AssetType::Texture2D;

								glm::ivec2 texture_size = { material_texture_ref->mWidth, material_texture_ref->mHeight };
								GLubyte* texture_data = reinterpret_cast<GLubyte*>(material_texture_ref->pcData);

								std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(texture_data, texture_size, GL_BGRA);
								asset_material->SetNormalTexture(asset_texture);

								asset_texture->Handle = asset_texture_handle;

								asset_map->operator[](asset_texture_handle) = asset_texture;
								asset_reg->operator[](asset_texture_handle) = texture_metadata;
							}

						}
						else if (std::filesystem::exists(resolved_texture_path)) {

							AssetHandle asset_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
								std::string(AssetTypeToString(AssetType::Texture2D)) + std::filesystem::relative(resolved_texture_path, Project::GetActiveProject()->GetConfig().AssetDirectory).string()
								));

							if (asset_map->count(asset_texture_handle) == 0) {

								AssetMetaData texture_metadata;
								texture_metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetConfig().AssetDirectory);
								texture_metadata.Type = AssetType::Texture2D;

								std::shared_ptr<Texture> asset_texture = std::make_shared<Texture>(resolved_texture_path);
								asset_material->SetNormalTexture(asset_texture);

								asset_texture->Handle = asset_texture_handle;

								asset_map->operator[](asset_texture_handle) = asset_texture;
								asset_reg->operator[](asset_texture_handle) = texture_metadata;
							}

						}
					}

					asset_material->Handle = asset_material_handle;

					asset_map->operator[](asset_material_handle) = asset_material;
					asset_reg->operator[](asset_material_handle) = material_metadata;
				}

			};

		std::function<void(aiNode*)> process_node = [&](aiNode* node) -> void
			{
				if(node->mParent) {

					std::shared_ptr<Asset> asset_mesh = std::make_shared<AssetMesh>();

					AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
						std::string(AssetTypeToString(AssetType::Mesh)) + path.string() + node->mName.C_Str()
					));

					AssetMetaData metadata;
					metadata.FilePath = std::filesystem::relative(path, Project::GetActiveProject()->GetConfig().AssetDirectory);
					metadata.Type = AssetType::Mesh;

					// process all the node's meshes (if any)
					for (unsigned int i = 0; i < node->mNumMeshes; i++) {
						aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
						process_mesh(mesh, std::static_pointer_cast<AssetMesh>(asset_mesh));
					}

					if(asset_mesh) {
						asset_mesh->Handle = handle;
						asset_map->operator[](handle) = asset_mesh;
						asset_reg->operator[](handle) = metadata;
					}
				}

				// then do the same for each of its children
				for (unsigned int i = 0; i < node->mNumChildren; i++) {
					process_node(node->mChildren[i]);
				}
			};

		process_node(scene->mRootNode);

		return model_prefab;
	}


#pragma endregion


}