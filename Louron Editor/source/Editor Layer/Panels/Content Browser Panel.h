#pragma once

#include "Louron.h"

#include <filewatch/FileWatch.hpp>

class LouronEditorLayer;

class ContentBrowserPanel {

public:

	ContentBrowserPanel();

	/// <summary>
	/// Set the current directory that is being viewed in the content browser.
	/// </summary>
	void SetDirectory(const std::filesystem::path& directory_path);

	void OnImGuiRender(LouronEditorLayer& editor_layer);

private:

	std::filesystem::path m_CurrentDirectory;
	std::filesystem::path m_AssetDirectory;
	std::filesystem::path m_SceneDirectory;

	std::unique_ptr<filewatch::FileWatch<std::string>> m_FileWatcher = nullptr;

	std::shared_ptr<Louron::Texture> m_DirectoryTexture = nullptr;
	std::shared_ptr<Louron::Texture> m_FileTexture = nullptr;

};