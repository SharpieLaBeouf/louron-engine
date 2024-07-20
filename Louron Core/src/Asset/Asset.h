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

	std::string_view AssetTypeToString(AssetType type);
	AssetType AssetTypeFromString(std::string_view assetType);

	class Asset : public std::enable_shared_from_this<Asset> {

	public:
		AssetHandle Handle;

		virtual AssetType GetType() const = 0;
	};

	class AssetMetaData {

	public:

		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;

		operator bool() const { return Type != AssetType::None; }

	};

}