#pragma once

#include "Asset.h"
#include "../Project/Project.h"

namespace Louron {

	static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {

		{ ".lscene",	AssetType::Scene },
		{ ".lprefab",	AssetType::Prefab },

		{ ".png",		AssetType::Texture2D },
		{ ".jpg",		AssetType::Texture2D },
		{ ".jpeg",		AssetType::Texture2D },
		{ ".psd",		AssetType::Texture2D },
		{ ".tga",		AssetType::Texture2D },
		{ ".tif",		AssetType::Texture2D },

		{ ".obj",		AssetType::ModelImport },
		{ ".fbx",		AssetType::ModelImport },

		{ ".mp3",		AssetType::Audio },

		{ ".lmaterial",	AssetType::Material_Standard },
		{ ".lmat",		AssetType::Material_Standard },
		{ ".lskybox",	AssetType::Material_Skybox }

	};

	/// <summary>
	/// Static API Class.
	/// </summary>
	class AssetManager {

	public:

		static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension);
		static bool IsExtensionSupported(const std::filesystem::path& extension);
		static bool IsAssetTypeComposite(const AssetType& asset_type);

		static std::string NormalisePath(const std::filesystem::path& p) {

			std::filesystem::path temp_path = std::filesystem::weakly_canonical(p);

			std::string path_str = temp_path.string();

			std::replace(path_str.begin(), path_str.end(), '\\', '/');

			return path_str;
		}

		template <typename AssetType>
		static std::shared_ptr<AssetType> GetAsset(AssetHandle handle) {

			std::shared_ptr<Asset> asset = Project::GetStaticAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<AssetType>(asset);

		}

		static bool IsAssetHandleValid(AssetHandle handle)
		{
			return Project::GetActiveProject()->GetAssetManager()->IsAssetHandleValid(handle);
		}

		static bool IsAssetLoaded(AssetHandle handle)
		{
			return Project::GetActiveProject()->GetAssetManager()->IsAssetLoaded(handle);
		}

		static AssetType GetAssetType(AssetHandle handle)
		{
			return Project::GetActiveProject()->GetAssetManager()->GetAssetType(handle);
		}
	};
}