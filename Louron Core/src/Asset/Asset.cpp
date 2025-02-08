#include "Asset.h"

namespace Louron {

	std::string AssetUtils::AssetTypeToString(AssetType type)
	{
		std::string type_string = "AssetType::<Invalid>";
		switch (type) {

			case AssetType::None:					type_string = "AssetType::None";				 break;

			case AssetType::Scene:					type_string = "AssetType::Scene";				 break;
			case AssetType::Prefab:					type_string = "AssetType::Prefab";				 break;

			case AssetType::Texture2D:				type_string = "AssetType::Texture2D";			 break;
			case AssetType::TextureCubeMap:			type_string = "AssetType::TextureCubeMap";		 break;

			case AssetType::Mesh:					type_string = "AssetType::Mesh";				 break;
			case AssetType::ModelImport:			type_string = "AssetType::ModelImport";			 break;

			case AssetType::Audio:					type_string = "AssetType::Audio";				 break;

			case AssetType::Material_Standard:		type_string = "AssetType::Material_Standard";	 break;
			case AssetType::Material_Skybox:		type_string = "AssetType::Material_Skybox";		 break;

			case AssetType::Shader:					type_string = "AssetType::Shader";				 break;
			case AssetType::Compute_Shader:			type_string = "AssetType::Compute_Shader";		 break;

		}
		return type_string;
	}

	AssetType AssetUtils::AssetTypeFromString(const std::string& assetType)
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

		if (assetType == "AssetType::Shader")				return AssetType::Shader;
		if (assetType == "AssetType::Compute_Shader")		return AssetType::Compute_Shader;

		return AssetType::None;
	}
}
