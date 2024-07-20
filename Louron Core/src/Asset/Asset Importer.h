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

	using AssetMap = std::map<AssetHandle, std::shared_ptr<Asset>>;
	using AssetRegistry = std::map<AssetHandle, AssetMetaData>;

	class AssetImporter
	{
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

	class MeshImporter {

	public:

		static std::shared_ptr<Prefab> ImportModel(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& meta_data);
		static std::shared_ptr<Prefab> LoadModel(AssetMap* asset_map, AssetRegistry* asset_reg, const std::filesystem::path& path);

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

}