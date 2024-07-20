#include "Asset.h"

namespace Louron {

	std::string_view Louron::AssetTypeToString(AssetType type)
	{
		std::string type_string = "AssetType::<Invalid>";
		switch (type) {

			case AssetType::None:					return "AssetType::None";	

			case AssetType::Scene:					return "AssetType::Scene";	
			case AssetType::Prefab:					return "AssetType::Prefab";	

			case AssetType::Texture2D:				return "AssetType::Texture2D";
			case AssetType::TextureCubeMap:			return "AssetType::TextureCubeMap";

			case AssetType::Mesh:					return "AssetType::Mesh";
			case AssetType::ModelImport:			return "AssetType::ModelImport";

			case AssetType::Audio:					return "AssetType::Audio";	

			case AssetType::Material_Standard:		return "AssetType::Material_Standard";
			case AssetType::Material_Skybox:		return "AssetType::Material_Skybox";

		}
		return "AssetType::<Invalid>";
	}

	AssetType AssetTypeFromString(std::string_view assetType)
	{
		if (assetType == "AssetType::None")					return AssetType::None;

		if (assetType == "AssetType::Scene")				return AssetType::Scene;
		if (assetType == "AssetType::Prefab")				return AssetType::Prefab;

		if (assetType == "AssetType::Texture2D")			return AssetType::Texture2D;
		if (assetType == "AssetType::TextureCubeMap")		return AssetType::TextureCubeMap;

		if (assetType == "AssetType::Mesh")					return AssetType::Mesh;
		if (assetType == "AssetType::ModelImport")			return AssetType::ModelImport;

		if (assetType == "AssetType::Audio")				return AssetType::Audio;

		if (assetType == "AssetType::Material_Standard")	return AssetType::Material_Standard;
		if (assetType == "AssetType::Material_Skybox")		return AssetType::Material_Skybox;

		return AssetType::None;
	}
}
