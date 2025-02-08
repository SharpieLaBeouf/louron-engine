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
		Material_Skybox,

		Compute_Shader,
		Shader

	};

	namespace AssetUtils {

		std::string AssetTypeToString(AssetType type);
		AssetType AssetTypeFromString(const std::string& assetType);
	}


	class Asset : public std::enable_shared_from_this<Asset> {

	public:
		AssetHandle Handle;

		virtual AssetType GetType() const = 0;
	};

	struct AssetMetaData {

		/// <summary>
		/// The Asset Type.
		/// </summary>
		AssetType Type = AssetType::None;

		/// <summary>
		/// The Relative file path of the Asset to the Project Asset Directory.
		/// </summary>
		std::filesystem::path FilePath = "";

		/// <summary>
		/// The name of the Asset.
		/// </summary>
		std::string AssetName = "";

		/// <summary>
		/// If the Asset is a child to a composite asset, e.g., this could be a material embedded in a Model File such as FBX.
		/// </summary>
		AssetHandle ParentAssetHandle = NULL_UUID;

		/// <summary>
		/// If the Asset has children dependencies, e.g., materials, textures, animations, etc.
		/// </summary>
		bool IsComposite = false;

		/// <summary>
		/// If the Asset is a Custom Asset that is added to the registry at runtime.
		/// </summary>
		bool IsCustomAsset = false;

		operator bool() const { return Type != AssetType::None; }
	};

}