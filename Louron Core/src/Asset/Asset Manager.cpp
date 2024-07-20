#include "Asset Manager.h"

#include "Asset Importer.h"

#include "../Project/Project.h"

namespace Louron {


	static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {

		{ ".lscene",	AssetType::Scene },
		{ ".lprefab",	AssetType::Prefab },

		{ ".png",		AssetType::Texture2D },
		{ ".jpg",		AssetType::Texture2D },
		{ ".jpeg",		AssetType::Texture2D },
		{ ".psd",		AssetType::Texture2D },

		{ ".obj",		AssetType::ModelImport },
		{ ".fbx",		AssetType::ModelImport },

		{ ".lmaterial",			AssetType::Material_Standard },
		{ ".lskyboxmaterial",	AssetType::Material_Skybox }

	};

	static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
	{
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
		{
			L_CORE_WARN("Could not find AssetType for {0}", extension.string().c_str());
			return AssetType::None;
		}

		return s_AssetExtensionMap.at(extension);
	}

	std::shared_ptr<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
	{
		// 1. VALID - Check if the Asset Handle is valid
		if (!IsAssetHandleValid(handle))
			return nullptr;

		// 2. LOAD - Check if the Asset needs to be loaded into memory
		std::shared_ptr<Asset> asset;
		if (IsAssetLoaded(handle))
		{
			asset = m_LoadedAssets.at(handle);
		}
		else
		{
			const AssetMetaData& metadata = GetMetadata(handle);
			asset = AssetImporter::ImportAsset(&m_LoadedAssets, &m_AssetRegistry, handle, metadata);
			if (!asset)
			{
				L_CORE_ERROR("EditorAssetManager::GetAsset - asset import failed!");
			}
			m_LoadedAssets[handle] = asset;
		}

		// 3. RETURN - Return the instance of our Asset
		return asset;
	}

	bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const {
		return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const {
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const {
		if (!IsAssetHandleValid(handle))
			return AssetType::None;

		return m_AssetRegistry.at(handle).Type;
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filepath) {

		AssetMetaData metadata;
		metadata.FilePath = filepath;
		metadata.Type = GetAssetTypeFromFileExtension(filepath.extension());

		AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
			std::string(AssetTypeToString(metadata.Type)) + filepath.string()
		));

		L_CORE_ASSERT(metadata.Type != AssetType::None, "Cannot Import Asset - MetaData Type Is Null.");

		std::shared_ptr<Asset> asset = AssetImporter::ImportAsset(&m_LoadedAssets, &m_AssetRegistry, handle, metadata);
		if (asset) {
			asset->Handle = handle;
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = metadata;
			SerializeAssetRegistry();
		}
		else {
			handle = NULL_UUID;
		}

		return handle;
	}

	const AssetMetaData& EditorAssetManager::GetMetadata(AssetHandle handle) const {
		
		auto it = m_AssetRegistry.find(handle);
		if (it == m_AssetRegistry.end())
		{
			static AssetMetaData s_NullMetadata;
			return s_NullMetadata;
		}

		return it->second;
	}

	const std::filesystem::path& EditorAssetManager::GetFilePath(AssetHandle handle) const {

		return GetMetadata(handle).FilePath;
	}

	void EditorAssetManager::SerializeAssetRegistry() {

	}

	bool EditorAssetManager::DeserializeAssetRegistry() {

		return false;
	}

}