#pragma once

// Internal
#include "Louron.h"
#include "Core/Layer.h"
#include "Panels/Properties Panel.h"
#include "Panels/Hierarchy Panel.h"

// External
#include "filewatch/FileWatch.hpp"

class LouronEditorLayer : public Louron::Layer {
public:

	LouronEditorLayer();
	virtual ~LouronEditorLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate() override;
	virtual void OnFixedUpdate() override;
	virtual void OnGuiRender() override;

	void OnScenePlay();
	void OnSceneStop();

private:

	std::unordered_map<const char*, bool> m_ActiveGUIWindows;

	bool m_SceneWindowFocused = false;
	glm::ivec2 m_ViewportWindowSize = { 800, 600 };

	Louron::Entity m_SelectedEntity;

	std::shared_ptr<Louron::Texture> m_IconPlay, m_IconPause, m_IconStep, m_IconSimulate, m_IconStop;

	// This is the scene that is copied before runtime starts
	// so that we can revert back to the active scene when 
	// runtime finishes
	std::shared_ptr<Louron::Scene> m_EditorScene;

	enum class SceneState
	{
		Edit = 0, Play = 1, Simulate = 2
	};
	SceneState m_SceneState = SceneState::Edit;

	PropertiesPanel m_PropertiesPanel{};
	HierarchyPanel m_HierarchyPanel{};

	bool m_WindowWasUnfocused = false;
	bool m_ScriptsCompiledSuccess = false;
	static std::atomic_bool m_ScriptsNeedCompiling;
	static std::unique_ptr<filewatch::FileWatch<std::string>> m_ScriptFileWatcher;

private:

	static void ScriptsModifiedEvent(const std::string& path, const filewatch::Event change_type)
	{
		// Set atomic bool
		if(std::filesystem::path(path).extension() == ".cs")
		{
			m_ScriptsNeedCompiling.store(true, std::memory_order_relaxed);
		}
	}

	void CheckInput();

	void NewScene();
	void OpenScene();
	void SaveScene(bool save_as = false);

	void DisplaySceneViewportWindow();

	void DisplayHierarchyWindow();
	void DisplayPropertiesWindow();
	void DisplayScriptFields(const std::string& script_name);

	void DisplayContentBrowserWindow();

	void DisplayRenderStatsWindow();
	void DisplayProfilerWindow();
	void DisplayAssetRegistryWindow();

	void DisplayProjectProperties();
	void DisplaySceneProperties();

};