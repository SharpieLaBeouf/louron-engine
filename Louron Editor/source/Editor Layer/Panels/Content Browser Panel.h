#pragma once

#include "Louron.h"

#include <filesystem>

#include <efsw/efsw.hpp>

class LouronEditorLayer;

class ContentBrowserPanel {

public:

	ContentBrowserPanel();

	/// <summary>
	/// Set the current directory that is being viewed in the content browser.
	/// </summary>
	void SetDirectory(const std::filesystem::path& directory_path);

	void OnImGuiRender(LouronEditorLayer& editor_layer);

	void StartFileWatcher();

private:

	std::filesystem::path m_CurrentDirectory;
	std::filesystem::path m_AssetDirectory;
	std::filesystem::path m_SceneDirectory;

	class AssetFileListener : public efsw::FileWatchListener {

        std::unordered_map<std::string, std::filesystem::path> m_AssetFileChanges;

	public:
		void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) override;
	};

    efsw::FileWatcher* m_AssetFileWatcher = new efsw::FileWatcher();
    AssetFileListener* m_AssetFileListener = new AssetFileListener();

	std::shared_ptr<Louron::Texture> m_DirectoryTexture = nullptr;
	std::shared_ptr<Louron::Texture> m_FileTexture = nullptr;

	Louron::Entity m_NewFocalEntity = {};

	friend class LouronEditorLayer;

};