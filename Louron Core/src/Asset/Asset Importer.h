#pragma once

#include "Asset.h"
#include "Asset Manager.h"

#include "../OpenGL/Texture.h"
#include "../OpenGL/Material.h"

#include "../Scene/Scene.h"
#include "../Scene/Prefab.h"

#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/Skybox.h"

namespace Louron {

	class Scene;
	class Prefab;
	class Texture;


	using AssetMap = std::map<AssetHandle, std::shared_ptr<Asset>>;
	using AssetRegistry = std::map<AssetHandle, AssetMetaData>;

	class AssetImporter {
	
	public:

		static std::shared_ptr<Asset> ImportAsset(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& metadata);

	};

	class SceneImporter {

	public:

		static std::shared_ptr<Scene> ImportScene(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data);
		static std::shared_ptr<Scene> LoadScene(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path);
		static void SaveScene(std::shared_ptr<Scene> scene, const std::filesystem::path& path);

	};

	class PrefabImporter {

		//static std::shared_ptr<Prefab> ImportPrefab(AssetHandle handle, const AssetMetaData& meta_data);
		//static std::shared_ptr<Prefab> LoadPrefab(const std::filesystem::path& path);

	};

	class TextureImporter {

	public:

		static std::shared_ptr<Texture> ImportTexture2D(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data);
		static std::shared_ptr<Texture> LoadTexture2D(const std::filesystem::path& path);
	};

	class ModelImporter {

	public:

		static std::shared_ptr<Prefab> ImportModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data);
		static std::shared_ptr<Prefab> LoadModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const std::filesystem::path& path);

	private:

		static void ProcessMesh(const aiScene* scene, aiMesh* mesh, std::shared_ptr<Prefab> model_prefab, entt::entity current_entity_handle, std::shared_ptr<AssetMesh> asset_mesh, AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle parent_asset_handle, const std::filesystem::path& path);
		static void ProcessNode(const aiScene* scene, aiNode* node, std::shared_ptr<Prefab> model_prefab, entt::entity parent_entity_handle, AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle parent_asset_handle, const std::filesystem::path& path);

	};

	class AudioImporter {

		//static std::shared_ptr<Audio> ImportAudio(AssetHandle handle, const AssetMetaData& meta_data);
		//static std::shared_ptr<Audio> LoadAudio(const std::filesystem::path& path);


	};

	class MaterialImporter {

	public:

		static std::shared_ptr<Material> ImportMaterial(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data);

		static std::shared_ptr<BPMaterial> LoadMaterialBP(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path);
		static std::shared_ptr<PBRMaterial> LoadMaterialPBR(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path);
		static std::shared_ptr<SkyboxMaterial> LoadMaterialSkybox(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path);

	};

	static std::string normalise_path(const std::filesystem::path& p) {

		std::filesystem::path temp_path = std::filesystem::weakly_canonical(p);

		std::string path_str = temp_path.string();

		std::replace(path_str.begin(), path_str.end(), '\\', '/');

		return path_str;
	}

}