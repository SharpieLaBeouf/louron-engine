#pragma once

#include "Asset.h"

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
				expectedType = AssetType::ModelImport; // ModelImport is technically a Prefab as we create a Prefab when we import a model file
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

			if (!IsAssetHandleValid(handle))
				return nullptr;

			// Check if the type matches
			if (m_AssetRegistry[handle].Type != expectedType) {
				return nullptr;
			}
			return std::static_pointer_cast<TAssetType>(GetAsset(handle));
		}

		virtual std::shared_ptr<Asset> GetAsset(AssetHandle handle) override;

		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
		virtual AssetType GetAssetType(AssetHandle handle) const override;

		/// <summary>
		/// This will Import an Asset give a file path. This file path HAS to be relative 
		/// to the AssetDirectory path. If it is not relative, it will not load!
		/// </summary>
		/// <param name="filepath">Needs to be relative to Project Asset Directory</param>
		/// <returns>Handle that Asset has been created with.</returns>
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