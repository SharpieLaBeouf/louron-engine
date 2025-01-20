#pragma once

#include "Asset.h"

#include "../Core/Logging.h"

#include <map>
#include <memory>

// Credit to Cherno for this system design!
// www.github.com/TheCherno/Hazel/tree/asset-manager/Hazel/src/Hazel/Asset

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
		{ ".lskybox",	AssetType::Material_Skybox }

	};

	using AssetMap = std::map<AssetHandle, std::shared_ptr<Asset>>;
	using AssetRegistry = std::map<AssetHandle, AssetMetaData>;

	/// <summary>
	/// Static API Class.
	/// </summary>
	class AssetManager {

	public:

		static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension);
		static bool IsExtensionSupported(const std::filesystem::path& extension);

		static std::string NormalisePath(const std::filesystem::path& p) {

			std::filesystem::path temp_path = std::filesystem::weakly_canonical(p);

			std::string path_str = temp_path.string();

			std::replace(path_str.begin(), path_str.end(), '\\', '/');

			return path_str;
		}

		template <typename AssetType>
		static AssetHandle CreateAsset(std::shared_ptr<AssetType> asset, const std::filesystem::path& file_path, const std::filesystem::path& project_file_path) {

			return 0;
		}

		template <typename AssetType>
		static std::shared_ptr<AssetType> GetAsset(AssetHandle handle) {

			//std::shared_ptr<Asset> asset = Project::GetActiveProject()->GetAssetManager()->GetAsset(handle);
			//return std::static_pointer_cast<T>(asset);

		}

		static bool IsAssetHandleValid(AssetHandle handle)
		{
			//return Project::GetActiveProject()->GetAssetManager()->IsAssetHandleValid(handle);
		}

		static bool IsAssetLoaded(AssetHandle handle)
		{
			//return Project::GetActiveProject()->GetAssetManager()->IsAssetLoaded(handle);
		}

		static AssetType GetAssetType(AssetHandle handle)
		{
			//return Project::GetActiveProject()->GetAssetManager()->GetAssetType(handle);
		}
	};

	class AssetManagerBase {

	public:
		virtual std::shared_ptr<Asset> GetAsset(AssetHandle handle) = 0;

		virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
		virtual AssetType GetAssetType(AssetHandle handle) const = 0;

	};

	class Texture;
	struct AssetMesh;
	class Prefab;
	class Material;
	class SkyboxMaterial;

	class EditorAssetManager : public AssetManagerBase {

	public:

		EditorAssetManager();

		template <typename TAssetType>
		AssetHandle CreateAsset(std::shared_ptr<TAssetType> asset, const std::filesystem::path& file_path, const std::filesystem::path& project_directory) {

			if (!asset)
				return NULL_UUID;

			// Determine the expected AssetType based on TAssetType
			AssetType asset_type = AssetType::None;
			if constexpr (std::is_same_v<TAssetType, Texture>) {
				asset_type = AssetType::Texture2D;
			}
			else if constexpr (std::is_same_v<TAssetType, AssetMesh>) {
				asset_type = AssetType::Mesh;
			}
			else if constexpr (std::is_same_v<TAssetType, Prefab>) {
				asset_type = AssetType::Prefab;
			}
			else if constexpr (std::is_same_v<TAssetType, SkyboxMaterial>) {
				asset_type = AssetType::Material_Skybox; // Check this first so we can return Material_Skybox opposed to Material_Standard
			}
			else if constexpr (std::is_base_of_v<Material, TAssetType>) {
				asset_type = AssetType::Material_Standard; // Material is a base class that may have derived classes
			}
			else {
				// If TAssetType does not match any known type, return
				return;
			}

			AssetMetaData meta_data;
			meta_data.FilePath = AssetManager::NormalisePath(std::filesystem::relative(file_path, project_directory / "Assets"));
			meta_data.Type = asset_type;
			meta_data.AssetName = file_path.stem().string();

			AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
				std::string(AssetTypeToString(meta_data.Type)) + meta_data.FilePath.string()
			));

			if (m_LoadedAssets.count(handle) == 0) {
				if (asset) {
					asset->Handle = handle;
					m_LoadedAssets[handle] = asset;
					m_AssetRegistry[handle] = meta_data;
					SerializeAssetRegistry();
				}
				else {
					handle = NULL_UUID;
				}
			}
			else {
				L_CORE_INFO("Asset Already Registered: {0}", meta_data.FilePath.string());
			}

			return handle;


		}

		template <typename TAssetType>
		std::shared_ptr<TAssetType> GetAsset(AssetHandle handle) {
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
				if (m_AssetRegistry[handle].Type == AssetType::ModelImport || m_AssetRegistry[handle].Type == AssetType::Prefab)
					return std::static_pointer_cast<TAssetType>(GetAsset(handle));
				
			}
			else {

				// Check if the type matches
				if (m_AssetRegistry[handle].Type == expectedType)
					return std::static_pointer_cast<TAssetType>(GetAsset(handle));
				
			}

			return nullptr;
		}

		virtual std::shared_ptr<Asset> GetAsset(AssetHandle handle) override;

		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
		virtual AssetType GetAssetType(AssetHandle handle) const override;

		/// <summary>
		/// This will Import an Asset give a file path.
		/// </summary>
		AssetHandle ImportAsset(const std::filesystem::path& filepath);

		const AssetMetaData& GetMetadata(AssetHandle handle) const;
		const std::filesystem::path& GetFilePath(AssetHandle handle) const;

		const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

		void RefreshAssetRegistry();

		void SerializeAssetRegistry(const std::filesystem::path& asset_registry_path = "");
		bool DeserializeAssetRegistry();

		AssetHandle GetHandleFromFilePath(const std::filesystem::path& path);

	private:
		AssetRegistry m_AssetRegistry;
		AssetMap m_LoadedAssets;
	};

}