#pragma once

#include "Asset.h"

#include <map>
#include <memory>

// Credit to Cherno for this system design!
// www.github.com/TheCherno/Hazel/tree/asset-manager/Hazel/src/Hazel/Asset

namespace Louron {

	using AssetMap = std::map<AssetHandle, std::shared_ptr<Asset>>;
	using AssetRegistry = std::map<AssetHandle, AssetMetaData>;

	/// <summary>
	/// Static API Class.
	/// </summary>
	class AssetManager {

	public:

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


	class EditorAssetManager : public AssetManagerBase {

	public:

		template <typename AssetType>
		std::shared_ptr<AssetType> GetAsset(AssetHandle handle) {
			return std::static_pointer_cast<AssetType>(GetAsset(handle));
		}

		virtual std::shared_ptr<Asset> GetAsset(AssetHandle handle) override;

		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
		virtual AssetType GetAssetType(AssetHandle handle) const override;

		AssetHandle ImportAsset(const std::filesystem::path& filepath);

		const AssetMetaData& GetMetadata(AssetHandle handle) const;
		const std::filesystem::path& GetFilePath(AssetHandle handle) const;

		const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

		void SerializeAssetRegistry();
		bool DeserializeAssetRegistry();

	private:
		AssetRegistry m_AssetRegistry;
		AssetMap m_LoadedAssets;
	};

}