#pragma once

#include "../Scene/Components/UUID.h"

#include <string>
#include <filesystem>

namespace Louron {

	using AssetHandle = UUID;

	enum class AssetType {

		None = 0,

		Scene,
		Prefab,

		Texture2D,
		TextureCubeMap,

		Mesh,
		ModelImport,

		Audio,

		Material_Standard,
		Material_Skybox

	};

	std::string AssetTypeToString(AssetType type);
	AssetType AssetTypeFromString(const std::string& assetType);

	class Asset : public std::enable_shared_from_this<Asset> {

	public:
		AssetHandle Handle;

		virtual AssetType GetType() const = 0;
	};

	class AssetMetaData {

	public:

		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;
		std::string AssetName;

		// This is a pre-requisite Asset that needs to be loaded for this asset to be retrieved.
		// For Example, a Model will create Mesh Assets and Material Assets, thus the Model needs
		// to be loaded first before we can get the Asset that this metadata points to.
		AssetHandle ParentAssetHandle = NULL_UUID;

		operator bool() const { return Type != AssetType::None; }

	};

}