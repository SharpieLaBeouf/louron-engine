#pragma once

#include "Asset.h"

#include "../Core/Logging.h"

#include <map>
#include <memory>

// Credit to Cherno for this system design!
// www.github.com/TheCherno/Hazel/tree/asset-manager/Hazel/src/Hazel/Asset
//  
// P.s., changed dramatically to my own style, but still relevant

namespace Louron {

	class Shader;

	using AssetMap = std::map<AssetHandle, std::shared_ptr<Asset>>;
	using AssetRegistry = std::map<AssetHandle, AssetMetaData>;

	class AssetManagerBase {

	public:
		virtual std::shared_ptr<Asset> GetAsset(const AssetHandle& handle) = 0;
		virtual std::shared_ptr<Shader> GetInbuiltShader(const std::string& default_shader_name, bool is_compute = false) = 0;

		virtual bool IsAssetHandleValid(const AssetHandle& handle) const = 0;
		virtual bool IsAssetLoaded(const AssetHandle& handle) const = 0;
		virtual AssetType GetAssetType(const AssetHandle& handle) const = 0;

		virtual void AddRuntimeAsset(std::shared_ptr<Asset> asset, AssetHandle asset_handle, AssetMetaData asset_meta_data) = 0;
		virtual void RemoveRuntimeAsset(AssetHandle asset_handle) = 0;
	};

	struct AssetMesh;

	class Prefab;
	class Texture;
	class Material;
	class SkyboxMaterial;
	class ComputeShaderAsset;

	class EditorAssetManager : public AssetManagerBase {

	public: // Primary Methods

		/// <summary>
		/// This will refresh the entire asset registry. This will unload all assets, clear 
		/// the current registry, then recurse through the provided asset directory for all 
		/// .meta files, and compatible files that do not have .meta file created.
		/// </summary>
		/// <param name="project_asset_directory">File path must be absolute file location on disk.</param>
		void RefreshAssetRegistry(const std::filesystem::path& project_asset_directory);

		/// <summary>
		/// This updates the metadata of a particular Asset Handle, for instance, if the file path changes.
		/// </summary>
		/// <param name="asset_handle">The Asset Handle we want to update the meta data for.</param>
		/// <param name="asset_meta_data">The new meta data to replace the old meta data.</param>
		void UpdateAssetMetaData(const AssetHandle& asset_handle, const AssetMetaData& asset_meta_data);

		/// <summary>
		/// This will add an asset to the registry that is not in the Project Asset Directory. 
		/// 
		/// Please Note: This will require a valid ABSOLUTE file path in the meta data provided.
		/// 
		/// For Example: Default editor resources such as a default particle sprite, or a checkered 
		/// invalid texture, etc.
		/// </summary>
		/// <param name="asset">The Asset to store.</param>
		/// <param name="asset_handle">The handle of the Asset.</param>
		/// <param name="asset_meta_data">The meta data of the Asset.</param>
		void AddCustomAsset(std::shared_ptr<Asset> asset, const AssetHandle& asset_handle, const AssetMetaData& asset_meta_data);

		/// <summary>
		/// This will import the custom asset to the registry that is not in the Project Asset Directory.
		/// 
		/// Please Note: This will require a valid ABSOLUTE file path in the meta data provided.
		/// 
		/// This is particularly useful for if we want to load a custom asset that is composite and has
		/// child assets that should be loaded into the asset manager alongside the primary composite asset.
		/// 
		/// For Example: A default cube that can be instantiated through the editor has an internal material
		/// that should be loaded into the asset registry aswell for runtime rendering, or a default skybox
		/// material that references textures that need to be loaded too.
		/// </summary>
		/// <param name="asset_handle">The handle of the Asset.</param>
		/// <param name="asset_meta_data">The meta data of the Asset.</param>
		void ImportCustomAsset(const AssetHandle& asset_handle, const AssetMetaData& asset_meta_data);

		/// <summary>
		/// This will re-import the appropriate custom asset where there have been changes to a custom asset.
		/// </summary>
		/// <param name="asset_file_path">File path must be absolute file location on disk.</param>
		void ReImportCustomAsset(const std::filesystem::path& asset_file_path);

		/// <summary>
		/// Imports the appropriate asset located at the file path.
		/// </summary>
		/// <param name="asset_file_path">File path must be absolute file location on disk.</param>
		/// <param name="project_asset_directory">The absolute file path of the Project Asset Directory.</param>
		/// <returns>The Asset Handle of the loaded asset, or NULL_UUID.</returns>
		AssetHandle ImportAsset(const std::filesystem::path& asset_file_path, const std::filesystem::path& project_asset_directory, const AssetHandle custom_handle = NULL_UUID);

		/// <summary>
		/// This will force reimport a particular Asset. For instance, an FBX file is modified and has
		/// more composite assets that will need to be reflected in the new Meta Data.
		/// </summary>
		/// <param name="asset_file_path">File path must be absolute file location on disk.</param>
		/// <param name="project_asset_directory">The absolute file path of the Project Asset Directory.</param>
		/// <returns>The Asset Handle of the loaded asset, or NULL_UUID.</returns>
		AssetHandle ReImportAsset(const std::filesystem::path& asset_file_path, const std::filesystem::path& project_asset_directory);

		/// <summary>
		/// This will load the asset by a given handle.
		/// </summary>
		/// <param name="asset_handle">The handle of the Asset you wish to load.</param>
		/// <returns>A pointer to the loaded Asset or nullptr.</returns>
		std::shared_ptr<Asset> LoadAsset(const AssetHandle& asset_handle);

		/// <summary>
		/// This will unload the memory associated with an asset.
		/// </summary>
		/// <param name="asset_handle">The Asset Handle of the Asset you are trying to unload.</param>
		void UnLoadAsset(const AssetHandle& asset_handle);

		/// <summary>
		/// This will remove the asset and all associated Meta Data from the Asset Manager.
		/// </summary>
		/// <param name="asset_handle">The handle of the Asset you wish to remove.</param>
		void RemoveAsset(const AssetHandle& asset_handle);

		/// <summary>
		/// Get an asset of a particular type.
		/// </summary>
		/// <typeparam name="TAssetType">The Type of the Asset you are trying to get.</typeparam>
		/// <param name="handle">The Asset Handle of the Asset you are trying to get.</param>
		/// <returns>A pointer to the asset, or nullptr if does not exist, or type does not match.</returns>
		template <typename TAssetType>
		std::shared_ptr<TAssetType> GetAsset(const AssetHandle& asset_handle) {
			// Determine the expected AssetType based on TAssetType
			AssetType expectedType;
			if constexpr (std::is_same_v<TAssetType, Texture>) {
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

			if constexpr (std::is_same_v<TAssetType, Prefab>) {
				// Model Imports are prefabs but there is no dedicated type 
				// for a model import, so we check both 
				if (m_AssetRegistry[asset_handle].Type == AssetType::ModelImport || m_AssetRegistry[asset_handle].Type == AssetType::Prefab)
					return std::static_pointer_cast<TAssetType>(GetAsset(asset_handle));

			}
			else {

				// Check if the type matches
				if (m_AssetRegistry[asset_handle].Type == expectedType)
					return std::static_pointer_cast<TAssetType>(GetAsset(asset_handle));

			}

			return nullptr;
		}

		virtual void AddRuntimeAsset(std::shared_ptr<Asset> asset, AssetHandle asset_handle, AssetMetaData asset_meta_data) override;
		virtual void RemoveRuntimeAsset(AssetHandle asset_handle) override;

		/// <summary>
		/// Get an asset of no particular type. This will need to be static_pointer_cast'd to 
		/// the applicable Asset Type for use.
		/// </summary>
		/// <param name="handle">The Asset Handle of the Asset you are trying to get.</param>
		/// <returns>A pointer to the asset base class, or nullptr if does not exist, or type does not match.</returns>
		virtual std::shared_ptr<Asset> GetAsset(const AssetHandle& asset_handle) override;

		/// <summary>
		/// When you call this function, it will look for internal shaders that are built in 
		/// to the pipeline and are not custom shaders made through the editor.
		/// </summary>
		/// <param name="default_shader_name">Name of InBuilt Shader.</param>
		virtual std::shared_ptr<Shader> GetInbuiltShader(const std::string& default_shader_name, bool is_compute = false) override;

		/// <summary>
		/// This will initialise all default inbuilt resources the editor provides.
		/// </summary>
		void InitDefaultResources();

	public: // Helper Methods

		EditorAssetManager() = default;

		// Virtual Override
		virtual bool IsAssetHandleValid(const AssetHandle& asset_handle) const override;
		virtual bool IsAssetLoaded(const AssetHandle& asset_handle) const override;
		virtual AssetType GetAssetType(const AssetHandle& asset_handle) const override;

		// General
		const AssetMap& GetAssetMap() const { return m_LoadedAssets; }
		const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

		const AssetMetaData& GetMetadata(const AssetHandle& asset_handle) const;
		AssetHandle GetHandleFromFilePath(const std::filesystem::path& asset_file_path, const std::filesystem::path& project_asset_directory);
		const std::filesystem::path& GetFilePathFromHandle(const AssetHandle& asset_handle) const;

	private:

		/// <summary>
		/// Generate a New Unique Asset Handle.
		/// </summary>
		/// <param name="asset_type">The Type of Asset.</param>
		/// <param name="asset_file_path">The Relative Path to the Project Asset Directory.</param>
		/// <returns>New Asset Handle.</returns>
		AssetHandle GenerateNewAssetHandle(const AssetType& asset_type, const std::filesystem::path& asset_file_path);

		/// <summary>
		/// This will serialise the Asset's Meta Data path to the path provided.
		/// </summary>
		/// <param name="asset_handle">The Handle of the Asset.</param>
		/// <param name="asset_meta_data">The Meta Data of the Asset.</param>
		/// <param name="meta_data_file_path">This must be passed as the (filepath + ".meta") at the end.</param>
		void SerialiseMetaDataFile(const AssetHandle& asset_handle, const AssetMetaData& asset_meta_data, const std::filesystem::path& meta_data_file_path);

	private:

		AssetRegistry m_AssetRegistry{};
		AssetMap m_LoadedAssets{};

		std::vector<AssetHandle> m_RuntimeCreatedAssetRegistry;
	};

}