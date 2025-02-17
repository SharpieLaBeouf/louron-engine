#pragma once

#include "Asset.h"
#include "../Project/Project.h"
#include "../OpenGL/Compute Shader Asset.h"

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
		{ ".lskybox",	AssetType::Material_Skybox },

		{ ".comp",		AssetType::Compute_Shader },
		{ ".compute",	AssetType::Compute_Shader },
		{ ".glsl",		AssetType::Shader },
		{ ".shader",	AssetType::Shader },
		{ ".lshader",	AssetType::Shader }

	};

	/// <summary>
	/// Static API Class.
	/// </summary>
	class AssetManager {

	public:

		static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension);
		static bool IsExtensionSupported(const std::filesystem::path& extension);
		static bool IsAssetTypeComposite(const AssetType& asset_type);

		template <typename AssetType>
		static std::shared_ptr<AssetType> GetAsset(AssetHandle handle) {

			auto project_ref = Project::GetActiveProject();
			if (!project_ref)
				return nullptr;

			std::shared_ptr<Asset> asset = project_ref->GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<AssetType>(asset);

		}

		static std::shared_ptr<Shader> GetInbuiltShader(const std::string& default_shader_name, bool is_compute = false)
		{
			auto project_ref = Project::GetActiveProject();
			if (!project_ref)
				return nullptr;

			return project_ref->GetAssetManager()->GetInbuiltShader(default_shader_name, is_compute);
		}

		template <typename TAssetType>
		static std::shared_ptr<TAssetType> GetInbuiltAsset(const std::string& inbuilt_asset_name, AssetType type = AssetType::None)
		{
			auto project_ref = Project::GetActiveProject();
			if (!project_ref)
				return nullptr;

			AssetType expectedType = type;
			if (expectedType == AssetType::None)
			{
				if constexpr (std::is_same_v<TAssetType, Texture2D>) {
					expectedType = AssetType::Texture2D;
				}
				else if constexpr (std::is_same_v<TAssetType, AssetMesh>) {
					expectedType = AssetType::Mesh;
				}
				else if constexpr (std::is_same_v<TAssetType, Prefab>) {
					expectedType = AssetType::Prefab;
				}
				else if constexpr (std::is_same_v<TAssetType, SkyboxMaterial>) {
					expectedType = AssetType::Material_Skybox; // Check this first so we can return Material_Skybox opposed to Material_Standard
				}
				else if constexpr (std::is_base_of_v<Material, TAssetType>) {
					expectedType = AssetType::Material_Standard; // Material is a base class that may have derived classes
				}
				else {
					// If TAssetType does not match any known type, return nullptr
					return nullptr;
				}
			}

			AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
				AssetUtils::AssetTypeToString(expectedType) + "InBuiltAsset" + inbuilt_asset_name
			));

			std::shared_ptr<Asset> asset = project_ref->GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<TAssetType>(asset);
		}

		static bool IsAssetHandleValid(AssetHandle handle)
		{
			auto project_ref = Project::GetActiveProject();
			if (!project_ref)
				return false;

			return project_ref->GetAssetManager()->IsAssetHandleValid(handle);
		}

		static bool IsAssetLoaded(AssetHandle handle)
		{
			auto project_ref = Project::GetActiveProject();
			if (!project_ref)
				return false;

			return project_ref->GetAssetManager()->IsAssetLoaded(handle);
		}

		static AssetType GetAssetType(AssetHandle handle)
		{
			auto project_ref = Project::GetActiveProject();
			if (!project_ref)
				return AssetType::None;

			return project_ref->GetAssetManager()->GetAssetType(handle);
		}

		template <typename TAssetType>
		static AssetType GetAssetTypeFromTypeName()
		{
			if constexpr (std::is_same_v<TAssetType, Texture2D>) {
				return AssetType::Texture2D;
			}
			else if constexpr (std::is_same_v<TAssetType, AssetMesh>) {
				return AssetType::Mesh;
			}
			else if constexpr (std::is_same_v<TAssetType, Prefab>) {
				return AssetType::Prefab;
			}
			else if constexpr (std::is_same_v<TAssetType, SkyboxMaterial>) {
				return AssetType::Material_Skybox; // Check this first so we can return Material_Skybox opposed to Material_Standard
			}
			else if constexpr (std::is_base_of_v<Material, TAssetType>) {
				return AssetType::Material_Standard; // Material is a base class that may have derived classes
			}
			// If TAssetType does not match any known type, return nullptr
			return AssetType::None;
		}

		template <typename TAssetType>
		static AssetHandle AddRuntimeAsset(std::shared_ptr<TAssetType> asset, const std::string& asset_name)
		{
			if (!asset || GetAssetTypeFromTypeName<TAssetType>() == AssetType::None)
				return NULL_UUID;

			auto project_ref = Project::GetActiveProject();
			if (!project_ref)
				return NULL_UUID;
			
			AssetMetaData meta_data{};
			meta_data.AssetName = asset_name;
			meta_data.Type = asset->GetType();

			if (meta_data.Type == AssetType::None)
				return NULL_UUID;

			AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
				AssetUtils::AssetTypeToString(meta_data.Type) + "RunTimeAsset" + meta_data.AssetName
			));

			int index = 0;
			while (IsAssetHandleValid(handle))
			{
				meta_data.AssetName = asset_name + "_" + std::to_string(index);
				handle = static_cast<uint32_t>(std::hash<std::string>{}(
					AssetUtils::AssetTypeToString(meta_data.Type) + "RunTimeAsset" + meta_data.AssetName
				));

				index++;
			}

			meta_data.IsCustomAsset = true;
			meta_data.IsComposite = IsAssetTypeComposite(meta_data.Type);

			project_ref->GetAssetManager()->AddRuntimeAsset(asset, handle, meta_data);

			return handle;
		}

		static void RemoveRuntimeAsset(AssetHandle asset_handle)
		{
			auto project_ref = Project::GetActiveProject();
			if (!project_ref)
				return;

			project_ref->GetAssetManager()->RemoveRuntimeAsset(asset_handle);
		}

		static void ClearRuntimeAssets()
		{
			auto project_ref = Project::GetActiveProject();
			if (!project_ref)
				return;

			project_ref->GetAssetManager()->ClearRuntimeAssets();
		}
	};
}