#include "Asset Manager.h"

#include "Asset Importer.h"

#include "../Project/Project.h"

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron {

	static GLFWwindow* s_AssetRegistryUpdateContext = nullptr;
	static std::atomic<bool> s_IsAssetRegistryUpdating = false;

	AssetType AssetManager::GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
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
		// We do this because if the AssetRegistry is updating, we need to have a
		// shared GLFW window context on the main thread. This is so that if the 
		// registry is being updated, once it's complete we can delete the shared
		// window context on the main thread!
		if (s_AssetRegistryUpdateContext != nullptr && !s_IsAssetRegistryUpdating.load()) {
			glfwDestroyWindow(s_AssetRegistryUpdateContext);
			s_AssetRegistryUpdateContext = nullptr;
		}

		// 1. VALID - Check if the Asset Handle is valid
		if (!IsAssetHandleValid(handle))
			return nullptr;

		// 2. LOAD - Check if the Asset needs to be loaded into memory
		std::shared_ptr<Asset> asset;
		if (IsAssetLoaded(handle)) {
			asset = m_LoadedAssets.at(handle);
		}
		else {
			AssetHandle parent_handle = m_AssetRegistry[handle].ParentAssetHandle;
			// 2.a. Determine if there is a pre-requisite asset that needs to be loaded first
			if (parent_handle != NULL_UUID) {

				// 2.a.i. If the parent asset is invalid in the registry, lets remove all 
				//    children that reference this asset that will also be invalid
				if (!IsAssetHandleValid(parent_handle)) {
					
					// First, collect all handles that need to be removed
					std::vector<AssetHandle> invalid_child_handles;
					for (const auto& [child_handle, metadata] : m_AssetRegistry) {
						if (metadata.ParentAssetHandle == parent_handle) {
							invalid_child_handles.push_back(child_handle);
						}
					}

					// Then, erase them from the map
					for (const auto& h : invalid_child_handles) {
						m_AssetRegistry.erase(h);
					}

					return nullptr;
				}
				
				// 2.a.ii. Load the parent asset if it is valid
				const AssetMetaData& parent_metadata = GetMetadata(parent_handle);
				asset = AssetImporter::ImportAsset(&m_LoadedAssets, &m_AssetRegistry, parent_handle, parent_metadata);
				if (!asset) {
					L_CORE_ERROR("EditorAssetManager::GetAsset - Parent Asset Import Failed!");
					return nullptr;
				}
				m_LoadedAssets[m_AssetRegistry[handle].ParentAssetHandle] = asset;

				// 2.a.iii. Get loaded child asset
				if (IsAssetLoaded(handle)) {
					asset = m_LoadedAssets.at(handle);
					asset->Handle = handle;
				}
				else {
					asset = nullptr; // If the child asset was not loaded when the parent was, then it is invalid!
				}
			}
			// 2.b. If there is no parent asset dependency, we will just load the asset and return it
			else {
				const AssetMetaData& metadata = GetMetadata(handle);
				asset = AssetImporter::ImportAsset(&m_LoadedAssets, &m_AssetRegistry, handle, metadata);
				if (!asset) {
					L_CORE_ERROR("EditorAssetManager::GetAsset - Asset Import Failed!");
					return nullptr;
				}
				asset->Handle = handle;
				m_LoadedAssets[handle] = asset;
			}
		}

		// 3. RETURN - Return the instance of our Asset
		return asset;
	}

	EditorAssetManager::EditorAssetManager()
	{
		AssetMetaData meta_data;

		meta_data.FilePath = "Resources/Models/Cube.fbx";
		meta_data.Type = AssetType::ModelImport;
		meta_data.AssetName = "Cube";

		AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
			std::string(AssetTypeToString(meta_data.Type)) + meta_data.FilePath.string()
		));

		auto asset = ModelImporter::LoadModel(&m_LoadedAssets, &m_AssetRegistry, handle, meta_data.FilePath);
		if (asset) {
			asset->Handle = handle;
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = meta_data;
		}

		meta_data.FilePath = "Resources/Models/Sphere.fbx";
		meta_data.Type = AssetType::ModelImport;
		meta_data.AssetName = "Sphere";

		handle = static_cast<uint32_t>(std::hash<std::string>{}(
			std::string(AssetTypeToString(meta_data.Type)) + meta_data.FilePath.string()
		));

		asset = ModelImporter::LoadModel(&m_LoadedAssets, &m_AssetRegistry, handle, meta_data.FilePath);
		if (asset) {
			asset->Handle = handle;
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = meta_data;
		}
	}

	bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const {
		return handle != NULL_UUID && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
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
		metadata.FilePath = AssetManager::NormalisePath(std::filesystem::relative(filepath, Project::GetActiveProject()->GetProjectDirectory() / "Assets"));
		metadata.Type = AssetManager::GetAssetTypeFromFileExtension(metadata.FilePath.extension());
		metadata.AssetName = metadata.FilePath.filename().replace_extension().string();

		AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
			std::string(AssetTypeToString(metadata.Type) + metadata.FilePath.string())
		));

		L_CORE_ASSERT(metadata.Type != AssetType::None, "Cannot Import Asset - MetaData Type Is Null.");

		if (m_LoadedAssets.count(handle) == 0) {
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
		}
		else {
			L_CORE_INFO("Asset Already Registered: {0}", metadata.FilePath.string());
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

	void EditorAssetManager::RefreshAssetRegistry() {

		if (s_IsAssetRegistryUpdating)
			return;

		// Create an offscreen OpenGL context shared with the main context
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		s_AssetRegistryUpdateContext = glfwCreateWindow(1, 1, "Asset Loading", nullptr, (GLFWwindow*)Engine::Get().GetWindow().GetNativeWindow());
		if (!s_AssetRegistryUpdateContext) {
			L_CORE_ERROR("Failed to create offscreen OpenGL context");
			return;
		}

		s_IsAssetRegistryUpdating = true;

		std::thread registry_load = std::thread([&]() {

			glfwMakeContextCurrent(s_AssetRegistryUpdateContext);

			std::filesystem::path filePath(Project::GetActiveProject()->GetConfig().AssetDirectory / "AssetRegistry.lassetreg");

			try {
				std::filesystem::remove(filePath);
			}
			catch (const std::filesystem::filesystem_error& e) {
				L_CORE_ERROR("Filesystem Error: {0}", e.what());
				s_IsAssetRegistryUpdating = false;
				return;
			}
			catch (const std::exception& e) {
				L_CORE_ERROR("General Exception: {0}", e.what());
				s_IsAssetRegistryUpdating = false;
				return;
			}

			auto newAssetManager = std::make_shared<EditorAssetManager>();

			// TODO: maybe look at implementing purely a metadata reload opposed to re-importing all assets
			auto importAssets = [&](const std::filesystem::path& directory) {

				std::vector<std::filesystem::path> deferred_asset_import;

				for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
					if (entry.is_regular_file()) {
						auto extension = entry.path().extension();

						if (AssetManager::GetAssetTypeFromFileExtension(extension) == AssetType::Material_Skybox) {
							deferred_asset_import.push_back(std::filesystem::absolute(entry.path()));
							continue;
						}

						if (s_AssetExtensionMap.find(extension) != s_AssetExtensionMap.end()) {
							newAssetManager->ImportAsset(std::filesystem::absolute(entry.path()));
						}
					}
				}

				// We do this because some assets may need to be deferred 
				// for loading as the metadata for dependencies may not 
				// be caught by the above loop yet
				for (const auto& path : deferred_asset_import) {
					newAssetManager->ImportAsset(std::filesystem::absolute(path));
				}
			};

			auto project = Project::GetActiveProject();
			std::filesystem::path my_path = std::filesystem::relative(project->GetProjectDirectory() / project->GetConfig().AssetDirectory);
			importAssets(my_path);

			// Remove references from old registry that no longer exist
			const AssetRegistry& newRegistry = newAssetManager->GetAssetRegistry();
			for (auto it = m_AssetRegistry.begin(); it != m_AssetRegistry.end(); ) {
				AssetHandle handle = it->first;
				if (newRegistry.count(handle) == 0) {
					it = m_AssetRegistry.erase(it);
					m_LoadedAssets.erase(handle);
				}
				else {
					++it;
				}
			}

			// Add in new references
			for (const auto& [handle, metaData] : newRegistry) {
				if (m_AssetRegistry.count(handle) == 0) {
					m_AssetRegistry[handle] = metaData;
				}
			}

			newAssetManager.reset();
			newAssetManager = nullptr;

			// After import, destroy the offscreen context
			s_IsAssetRegistryUpdating = false;

			SerializeAssetRegistry();
			
		});

		registry_load.detach();
	}

	void EditorAssetManager::SerializeAssetRegistry(const std::filesystem::path& asset_registry_path) {

		std::filesystem::path path = asset_registry_path; 

		if (asset_registry_path == "") 
			path = Project::GetActiveProject()->GetProjectDirectory() / Project::GetActiveProject()->GetConfig().AssetDirectory / Project::GetActiveProject()->GetConfig().AssetRegistry;

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "AssetRegistry" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : m_AssetRegistry)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				out << YAML::Key << "Name" << YAML::Value << metadata.AssetName;
				out << YAML::Key << "FilePath" << YAML::Value << metadata.FilePath.generic_string();
				out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
				out << YAML::Key << "ParentAssetHandle" << YAML::Value << metadata.ParentAssetHandle;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap; // Root
		}

		std::ofstream fout(path);
		fout << out.c_str();
	}

	bool EditorAssetManager::DeserializeAssetRegistry() {

		auto project = Project::GetActiveProject();
		auto path = project->GetProjectDirectory() / project->GetConfig().AssetDirectory / project->GetConfig().AssetRegistry;

		if (!std::filesystem::exists(path)) {
			L_CORE_ERROR("Cannot Deserialise Asset Registry - Path Does Not Exist: {0}\n     Creating New Asset Registry.", path.string().c_str());
			SerializeAssetRegistry();
			return false;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e)
		{
			L_CORE_ERROR("Failed to load project file '{0}'\n     {1}", path.string().c_str(), e.what());
			return false;
		}

		auto rootNode = data["AssetRegistry"];
		if (!rootNode)
			return false;

		for (const auto& node : rootNode)
		{
			AssetHandle handle = node["Handle"].as<uint32_t>();
			auto& metadata = m_AssetRegistry[handle];
			if (node["Name"])
				metadata.AssetName = node["Name"].as<std::string>();
			if(node["FilePath"])
			{
				metadata.FilePath = node["FilePath"].as<std::string>();
			}
			if(node["Type"])
				metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
			if (node["ParentAssetHandle"])
				metadata.ParentAssetHandle = node["ParentAssetHandle"].as<uint32_t>();
		}

		return true;
	}

	AssetHandle EditorAssetManager::GetHandleFromFilePath(const std::filesystem::path& path) {

		AssetHandle handle = static_cast<uint32_t>(std::hash<std::string>{}(
			std::string(AssetTypeToString(AssetManager::GetAssetTypeFromFileExtension(path.extension()))) + AssetManager::NormalisePath(std::filesystem::relative(path, Project::GetActiveProject()->GetProjectDirectory() / "Assets"))
		));

		return handle;
	}

	bool AssetManager::IsExtensionSupported(const std::filesystem::path& extension) {
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
			return false;
		return true;
	}

}