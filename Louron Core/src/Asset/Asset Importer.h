#pragma once

#include "Asset.h"
#include "Asset Manager.h"

#include "../OpenGL/Compute Shader Asset.h"

#include "../OpenGL/Texture.h"
#include "../OpenGL/Material.h"

#include "../Scene/Scene.h"
#include "../Scene/Prefab.h"

#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/Skybox.h"

namespace Louron {

	class Scene;
	class Prefab;
	class Texture2D;

	using AssetMap = std::map<AssetHandle, std::shared_ptr<Asset>>;
	using AssetRegistry = std::map<AssetHandle, AssetMetaData>;

	class AssetImporter {
	
	public:

		static std::shared_ptr<Asset> ImportAsset(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& metadata, const std::filesystem::path& project_asset_directory);

	};

	class SceneImporter {

	public:

		static std::shared_ptr<Scene> ImportScene(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory);
		static std::shared_ptr<Scene> LoadScene(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path);

	};

	class PrefabImporter {

	public:

		static std::shared_ptr<Prefab> ImportPrefab(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& metadata, const std::filesystem::path& project_asset_directory);
		static std::shared_ptr<Prefab> LoadPrefab(const std::filesystem::path& path);

	};

	class TextureImporter {

	public:

		static std::shared_ptr<Texture2D> ImportTexture2D(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory);
		static std::shared_ptr<Texture2D> LoadTexture2D(const std::filesystem::path& path);
	};

	class ModelImporter {

	public:

		static std::shared_ptr<Prefab> ImportModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory);
		static std::shared_ptr<Prefab> LoadModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& path);

	private:

		static void ProcessMesh(const aiScene* scene, aiMesh* mesh, std::shared_ptr<Prefab> model_prefab, entt::entity current_entity_handle, std::shared_ptr<AssetMesh> asset_mesh, AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle parent_asset_handle, const AssetMetaData& parent_meta_data, const std::filesystem::path& path);
		static void ProcessNode(const aiScene* scene, aiNode* node, std::shared_ptr<Prefab> model_prefab, entt::entity parent_entity_handle, AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle parent_asset_handle, const AssetMetaData& parent_meta_data, const std::filesystem::path& path);

	};

	class AudioImporter {

		//static std::shared_ptr<Audio> ImportAudio(AssetHandle handle, const AssetMetaData& meta_data);
		//static std::shared_ptr<Audio> LoadAudio(const std::filesystem::path& path);


	};

	class MaterialImporter {

	public:

		static std::shared_ptr<Material> ImportMaterial(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory);

		static std::shared_ptr<Material> LoadMaterialPBR(const std::filesystem::path& path);
		static std::shared_ptr<SkyboxMaterial> LoadMaterialSkybox(const std::filesystem::path& path);

	};

	class ShaderImporter
	{

	public:

		static std::shared_ptr<Shader> ImportShader(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory);
		static std::shared_ptr<Shader> LoadShader(const std::filesystem::path& path);

		static std::shared_ptr<ComputeShaderAsset> ImportComputeShader(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data, const std::filesystem::path& project_asset_directory);
		static std::shared_ptr<ComputeShaderAsset> LoadComputeShader(const std::filesystem::path& path);

	};

}