#include "Content Browser Panel.h"

#include "../Louron Editor Layer.h"
#include "../Utils/Editor Script Utils.h"


#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

using namespace Louron;

static const std::unordered_map<std::string, bool> s_SupportedOpenInEditorFiles = {
	{ ".lscene", true }
};

ContentBrowserPanel::ContentBrowserPanel() {

	m_DirectoryTexture = TextureImporter::LoadTexture2D("Resources/Icons/DirectoryIcon.png");
	m_FileTexture = TextureImporter::LoadTexture2D("Resources/Icons/FileIcon.png");
}

static efsw::WatchID m_AssetFileWatchID;

void ContentBrowserPanel::StartFileWatcher()
{
	m_AssetFileWatchID = m_AssetFileWatcher->addWatch(Project::GetActiveProject()->GetAssetDirectory().string(), m_AssetFileListener, true);
	m_AssetFileWatcher->watch();
}

void ContentBrowserPanel::SetDirectory(const std::filesystem::path& directory_path) {
	m_CurrentDirectory = directory_path;
}

static bool PathHasSubDirectory(const std::filesystem::path& directory_path) {
	for (const auto& entry : std::filesystem::directory_iterator(directory_path))
		if (entry.path().parent_path() == directory_path && entry.is_directory())
			return true;
	return false;
}

void ContentBrowserPanel::OnImGuiRender(LouronEditorLayer& editor_layer) {

	std::filesystem::path current_lexical_normal = m_CurrentDirectory.lexically_normal();
	std::filesystem::path parent_lexical_normal;

	// Check if the current directory is within "Assets" or "Scenes"
	if (current_lexical_normal.string().find((Project::GetActiveProject()->GetProjectDirectory() / "Assets").lexically_normal().string()) != std::string::npos)
		parent_lexical_normal = (Project::GetActiveProject()->GetProjectDirectory() / "Assets").lexically_normal();
	else if (current_lexical_normal.string().find((Project::GetActiveProject()->GetProjectDirectory() / "Scenes").lexically_normal().string()) != std::string::npos)
		parent_lexical_normal = (Project::GetActiveProject()->GetProjectDirectory() / "Scenes").lexically_normal();
	else
		parent_lexical_normal = Project::GetActiveProject()->GetProjectDirectory().lexically_normal();

		
	// Begin the table with 2 columns
	if (ImGui::BeginTable("MyTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings)) {

		{ // Setup

			// Setup the first column and set its initial width
			ImGui::TableSetupColumn("Directory", ImGuiTableColumnFlags_WidthFixed, 200.0f);
			ImGui::TableSetupColumn("Other", ImGuiTableColumnFlags_WidthStretch);

			// Set the initial width of the first column (this only sets the width on the first frame)

			// Row for first column content
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); // Set to first column
		}

		auto directory_folder_drop_source = [&](const std::filesystem::path& file_path) {

			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path path = file_path;
				std::string path_str = path.string(); // Convert to string
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_FOLDER", path_str.c_str(), path_str.size() + 1); // Null-terminate string

				ImGui::EndDragDropSource();
			}
		};

		// returns true if folder was updated, returns false if no folder updated or if file
		auto directory_folder_drop_target = [&](const std::filesystem::path& file_path) -> bool {

			if (ImGui::BeginDragDropTarget()) {

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {
					// Convert the payload data (string) back into a filesystem path
					std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize);
					std::filesystem::path dropped_path = dropped_path_str; // Convert to path

					try {
						// Combine dropped_path with entry.path() to get the target path
						std::filesystem::path target_path = file_path / dropped_path.filename();

						// Move the folder to the new target folder
						std::filesystem::rename(dropped_path, target_path); // Moves the folder

						return false; // Wasn't a folder

					}
					catch (const std::filesystem::filesystem_error& e) {
						L_APP_WARN("Could Not Drop {} Folder into {} Folder: {}", dropped_path.string().c_str(), file_path.string().c_str(), e.what());
					}
				}

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FOLDER")) {
					// Convert the payload data (string) back into a filesystem path
					std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize);
					std::filesystem::path dropped_path = dropped_path_str; // Convert to path

					try {
						// Combine dropped_path with entry.path() to get the target path
						std::filesystem::path target_path = file_path / dropped_path.filename();

						// Move the folder to the new target folder
						std::filesystem::rename(dropped_path, target_path); // Moves the folder
						
						return true;

					}
					catch (const std::filesystem::filesystem_error& e) {
						L_APP_WARN("Could Not Drop {} Folder into {} Folder: {}", dropped_path.string().c_str(), file_path.string().c_str(), e.what());
					}
				}

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_UUID")) {
					Louron::UUID droppedEntityUUID = *(const Louron::UUID*)payload->Data;
					Entity droppedEntity = Project::GetActiveScene()->FindEntityByUUID(droppedEntityUUID);

					if (droppedEntity) {

						std::shared_ptr<Prefab> prefab = std::make_shared<Prefab>(droppedEntity);
						if (prefab) 
						{
							std::filesystem::path file_path = m_CurrentDirectory / (droppedEntity.GetName() + ".lprefab");
							prefab->Serialize(file_path);

							Project::GetStaticEditorAssetManager()->ImportAsset(file_path, Project::GetActiveProject()->GetAssetDirectory());
						}
					}
				}


				ImGui::EndDragDropTarget();
			}

			return false;
		};

		// ------------- Directory Browser Side Bar -------------
		if (ImGui::BeginChild("Directory Browser", {}, ImGuiChildFlags_Border | ImGuiChildFlags_FrameStyle))
		{

			std::function<bool(const std::filesystem::path&)> recursive_directory_display;

			recursive_directory_display = [&](const std::filesystem::path& current_directory_path) -> bool {

				for (const auto& entry : std::filesystem::directory_iterator(current_directory_path)) {

					std::filesystem::path entry_path = entry.path().lexically_normal();

					if (entry.is_directory()) {

						bool is_leaf_node = !PathHasSubDirectory(entry_path);

						ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick;
						flags |= entry_path == m_CurrentDirectory ? ImGuiTreeNodeFlags_Selected : 0;
						flags |= is_leaf_node ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow;

						bool tree_node_opened = ImGui::TreeNodeEx((entry_path.filename().string() + "##" + entry_path.string()).c_str(), flags);

						directory_folder_drop_source(entry_path);
						if (directory_folder_drop_target(entry_path))
						{
							m_CurrentDirectory = entry_path;
							ImGui::TreePop();
							return false;
						}

						// This path tree node has been clicked
						if (ImGui::IsItemClicked()) {
							m_CurrentDirectory = entry_path;
						}

						if (tree_node_opened) {
							if (!recursive_directory_display(entry_path))
							{
								ImGui::TreePop();
								return false;
							}
							ImGui::TreePop();
						}

					}
				}
				return true;
			};

			// --------------- SIDE BAR ROOT FOLDERS ---------------
			for (int i = 0; i < 3; i++) {


				std::filesystem::path side_bar_root_path;
				
				switch (i) {
					case 0: ImGui::SeparatorText("Assets"); side_bar_root_path = Project::GetActiveProject()->GetProjectDirectory() / "Assets"; break;	// --------------- ASSETS ---------------
					case 1: ImGui::SeparatorText("Scenes"); side_bar_root_path = Project::GetActiveProject()->GetProjectDirectory() / "Scenes"; break;	// --------------- SCENES ---------------
					case 2: ImGui::SeparatorText("Scripts"); side_bar_root_path = Project::GetActiveProject()->GetProjectDirectory() / "Scripts"; break;	// --------------- SCRIPTS ---------------
				}

				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick;
				flags |= m_CurrentDirectory == side_bar_root_path ? ImGuiTreeNodeFlags_Selected : 0;
				flags |= PathHasSubDirectory(side_bar_root_path) ? ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf;
				if (ImGui::TreeNodeEx(side_bar_root_path.filename().string().c_str(), flags))
				{
					if(directory_folder_drop_target(side_bar_root_path))
						m_CurrentDirectory = side_bar_root_path;

					// This path has been clicked - WHEN OPENED
					if (ImGui::IsItemClicked()) {
						m_CurrentDirectory = side_bar_root_path;
					}

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2)); // Temporary spacing for tree nodes
					recursive_directory_display(side_bar_root_path);
					ImGui::PopStyleVar();

					ImGui::TreePop();
				}
				else 
				{
					if (directory_folder_drop_target(side_bar_root_path))
						m_CurrentDirectory = side_bar_root_path;

					// This path has been clicked - WHEN CLOSED
					if (ImGui::IsItemClicked()) {
						m_CurrentDirectory = side_bar_root_path;
					}
				}

			}

		}
		ImGui::EndChild();

		// ------------- Directory Browser Top Bar -------------
		{

			// Row for second column content
			ImGui::TableNextColumn();
			ImGui::TableSetColumnIndex(1); // Set to second column

			std::filesystem::path relative_path = std::filesystem::relative(current_lexical_normal, Project::GetActiveProject()->GetProjectDirectory());
			std::filesystem::path accumulated_path = Project::GetActiveProject()->GetProjectDirectory(); // Start with the parent directory's absolute path.
			for (auto it = relative_path.begin(); it != relative_path.end(); ++it) {

				accumulated_path /= *it; // Build the full path step by step.

				if (std::next(it) == relative_path.end()) {

					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

					ImGui::Button(it->filename().string().c_str());

					ImGui::PopStyleColor(3);
					ImGui::PopStyleVar();

				}
				else {

					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

					if (ImGui::Button((accumulated_path.filename().string() + "##" + accumulated_path.string()).c_str())) {
						m_CurrentDirectory = accumulated_path; // Set the full path here.
					}

					ImGui::PopStyleColor(2);
					ImGui::PopStyleVar();

					ImGui::SameLine();
					ImGui::Text("/");
					ImGui::SameLine();
				}
			}
		}

		const float slider_bar_height = 20.0f;

		ImGui::SetNextWindowSize({0.0f, ImGui::GetContentRegionAvail().y - slider_bar_height});

		// -------------       File Browser        -------------
		if (ImGui::BeginChild("File Browser", {}, ImGuiChildFlags_Border | ImGuiChildFlags_FrameStyle)) {

			float available_width = ImGui::GetContentRegionAvail().x;

			float icon_size = 64.0f + ImGui::GetStyle().ItemSpacing.x * 2.0f;
			int num_columns = std::truncf(available_width / icon_size) > 0.0f ? (int)std::truncf(available_width / icon_size) : 4;

			// Start the table with the specified number of columns
			if (ImGui::BeginTable("FileTable", num_columns, ImGuiTableFlags_NoSavedSettings)) {

				static bool image_button_clicked = false;
				static std::filesystem::path path_clicked = "";

				static bool is_renaming_path = false;
				static std::string new_path_file_name;
				static std::filesystem::path renaming_path = "";
				static bool first_focus = true; // Track if this is the first time the InputText box gets focus

				// Check for right-click on blank space
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsWindowHovered()) {
					ImGui::OpenPopup("BlankSpaceContextMenu");
					image_button_clicked = false;
				}

				// Sort Directories first, then files
				std::vector<std::filesystem::directory_entry> directories;
				std::vector<std::filesystem::directory_entry> files;

				for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
					if (std::filesystem::is_directory(entry)) {
						directories.push_back(entry);
					}
					else {

						if(entry.path().extension() != ".meta")
							files.push_back(entry);
					}
				}

				// Sort directories and files if necessary
				std::sort(directories.begin(), directories.end(), [](const auto& lhs, const auto& rhs) {
					return lhs.path() < rhs.path();
					});
				std::sort(files.begin(), files.end(), [](const auto& lhs, const auto& rhs) {
					return lhs.path() < rhs.path();
					});

				// Combine directories and files back
				directories.insert(directories.end(), files.begin(), files.end());

				for (const auto& entry : directories) {

					// Start a new row (ImGui will automatically handle row creation)
					ImGui::TableNextColumn();  // Move to the next column in the table

					if (entry.is_directory()) { // ----------- DIRECTORY -----------

						ImGui::SetNextItemWidth(icon_size);

						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
						ImGui::ImageButton(entry.path().string().c_str(), (ImTextureID)(uintptr_t)m_DirectoryTexture->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 });
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							m_CurrentDirectory = entry.path();
						}

						directory_folder_drop_source(entry.path());
						directory_folder_drop_target(entry.path());

						ImGui::PopStyleVar(2);

						if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered()) {
							image_button_clicked = true;
							path_clicked = entry.path();
							ImGui::OpenPopup("ItemContextMenu");
						}

					}
					else if (entry.is_regular_file()) { // ----------- REGULAR FILE -----------

						ImGui::SetNextItemWidth(icon_size);

						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
						GLuint texture_id = m_FileTexture->GetID();

						if (AssetManager::IsExtensionSupported(entry.path().extension()) && AssetManager::GetAssetTypeFromFileExtension(entry.path().extension()) == AssetType::Texture2D)
						{
							auto texture_asset = AssetManager::GetAsset<Texture>(Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(entry.path(), Project::GetActiveProject()->GetAssetDirectory()));
							if (texture_asset)
								texture_id = texture_asset->GetID();
						}

						ImGui::ImageButton(entry.path().string().c_str(), (ImTextureID)(uintptr_t)texture_id, {64.0f, 64.0f}, {0, 1}, {1, 0});

						if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && AssetManager::GetAssetTypeFromFileExtension(entry.path().extension()) == AssetType::Material_Standard)
							editor_layer.m_MaterialContext = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(entry.path(), Project::GetActiveProject()->GetAssetDirectory());

						if (ImGui::BeginDragDropSource())
						{
							std::string path_str = entry.path().string(); // Convert to string
							ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_FILE", path_str.c_str(), path_str.size() + 1); // Null-terminate string
							ImGui::EndDragDropSource();
						}
						
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {

							// If we support opening files in the editor, call the appropriate method to open that file
							if (s_SupportedOpenInEditorFiles.find(entry.path().extension().string()) != s_SupportedOpenInEditorFiles.end()) {
								editor_layer.OpenScene(entry.path());
							}
							else { // If not, we will system call the file to open in default system application
								std::string command = "start \"\" \"" + entry.path().string() + "\"";
								std::system(command.c_str());
							}

						}
						
						ImGui::PopStyleVar(2);

						if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered()) {
							image_button_clicked = true;
							path_clicked = entry.path();
							ImGui::OpenPopup("ItemContextMenu");
						}
					}

					// ----------- FILE NAME -----------

					// IF WE ARE RENAMING THE PATH
					if(renaming_path == entry.path() && is_renaming_path)
					{
						static char buf[256]{};

						if (first_focus) {
							strcpy_s(buf, sizeof(buf), new_path_file_name.c_str());
						}
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
						if (ImGui::InputText("##RenamingInputText", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
							// Rename file/folder

							try {
								// Construct the new path by replacing the filename or folder name
								std::filesystem::path old_path = entry.path();
								std::filesystem::path new_path = old_path.parent_path() / buf;

								// Perform the rename operation
								std::filesystem::rename(old_path, new_path);

							}
							catch (const std::filesystem::filesystem_error& e) {
								L_APP_WARN("Could Not Delete {0}: {1}.", std::filesystem::is_directory(entry.path()) ? "Folder" : "File", e.what());
							}


							is_renaming_path = false;
							new_path_file_name.clear();
							renaming_path.clear();
							first_focus = true; // Reset for the next renaming operation
						}
						else if (first_focus) {
							first_focus = false;
						}

						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered()) {

							is_renaming_path = false;
							new_path_file_name.clear();
							renaming_path.clear();
							first_focus = true; // Reset for the next renaming operation
						}
					}
					// JUST SHOW THE PATH
					else {
						ImGui::TextWrapped(entry.path().filename().string().c_str());
					}

				}

				static bool script_create = false;
				::Utils::OnCreateNewScriptGUI(&script_create);

				static bool delete_file_path = false;
				static std::filesystem::path path_to_delete{};

				if (image_button_clicked) {
					
					// Handle item-specific context menu
					if (ImGui::BeginPopup("ItemContextMenu")) {

						if (ImGui::MenuItem(std::filesystem::is_directory(path_clicked) ? "Open Directory In File Explorer" : "Open File")) {
							std::string command = "explorer \"" + path_clicked.string() + "\"";
							std::system(command.c_str());
						}

						if (ImGui::MenuItem("Rename")) {
							is_renaming_path = true;
							renaming_path = path_clicked;
							new_path_file_name = path_clicked.filename().string();
							first_focus = true;
						}

						if (ImGui::MenuItem(std::filesystem::is_directory(path_clicked) ? "Delete Folder" : "Delete File")) {
							// Delete logic here

							delete_file_path = true;
							path_to_delete = path_clicked;
						}
						ImGui::EndPopup();
					}
					else {
						image_button_clicked = false;
						path_clicked = "";
					}
				}
				else {

					// Handle blank space context menu
					if (ImGui::BeginPopup("BlankSpaceContextMenu")) {

						if (m_CurrentDirectory.lexically_normal().string().find((Project::GetActiveProject()->GetProjectDirectory() / "Scripts").lexically_normal().string()) != std::string::npos) {

							if (ImGui::MenuItem("Create Script File")) {

								std::filesystem::path file_path = m_CurrentDirectory / "New Script.cs";

								// Create file logic here
								if(!std::filesystem::exists(file_path))
								{
									script_create = true;
								}

							}
						}

						if (ImGui::MenuItem("Create Folder")) {
							std::filesystem::path file_path = m_CurrentDirectory / "New Folder";
							
							if (!std::filesystem::exists(file_path))
							{
								std::filesystem::create_directories(file_path);
								is_renaming_path = true;
								renaming_path = file_path;
								new_path_file_name = file_path.filename().string();
								first_focus = true;
							}
						}

						if (ImGui::MenuItem("Create New Material")) {
							std::filesystem::path file_path = m_CurrentDirectory / "New Material.lmat";

							// Ensure unique filename
							int counter = 1;
							while (std::filesystem::exists(file_path)) {
								file_path = m_CurrentDirectory / ("New Material (" + std::to_string(counter) + ").lmat");
								counter++;
							}

							PBRMaterial material = PBRMaterial{};
							material.SetName(file_path.stem().string());

							YAML::Emitter out;
							out << YAML::BeginMap;
							material.Serialize(out);
							out << YAML::EndMap;

							std::ofstream fout(file_path); // Create the file
							fout << out.c_str();

							is_renaming_path = true;
							renaming_path = file_path;
							new_path_file_name = file_path.filename().string();
							first_focus = true;

							Project::GetStaticEditorAssetManager()->ImportAsset(file_path, Project::GetActiveProject()->GetAssetDirectory());
						}

						if (ImGui::MenuItem("Create New Shader")) {
							std::filesystem::path file_path = m_CurrentDirectory / "New Shader.lshader";

							// Ensure unique filename
							int counter = 1;
							while (std::filesystem::exists(file_path)) {
								file_path = m_CurrentDirectory / ("New Shader (" + std::to_string(counter) + ").lshader");
								counter++;
							}

							std::filesystem::copy("Resources/Shaders/Forward+/FP_Material_PBR_Shader.glsl", file_path);
							is_renaming_path = true;
							renaming_path = file_path;
							new_path_file_name = file_path.filename().string();
							first_focus = true;

							Project::GetStaticEditorAssetManager()->ImportAsset(file_path, Project::GetActiveProject()->GetAssetDirectory());
						}

						if (ImGui::MenuItem("Open Directory In File Explorer")) {
							std::string command = "explorer \"" + m_CurrentDirectory.string() + "\"";
							std::system(command.c_str());
						}

						ImGui::EndPopup();
					}
				}

				if (delete_file_path)
				{
					ImGui::OpenPopup("Delete?##ContentBrowserDeletePath");

					// Calculate window size: 25% of the screen width, minimum 300.0f
					float windowWidth = glm::max(300.0f, ImGui::GetContentRegionAvail().x * 0.25f);
					float windowHeight = 100.0f; // Fixed height
					ImVec2 windowSize(windowWidth, windowHeight);
					ImGui::SetNextWindowSize(windowSize);

					ImGuiViewport* viewport = ImGui::GetMainViewport();
					ImVec2 viewportPos = viewport->Pos;
					ImVec2 viewportSize = viewport->Size;

					// Calculate centered position
					ImVec2 windowPos(
						viewportPos.x + (viewportSize.x - windowWidth) * 0.5f, // Center horizontally
						viewportPos.y + (viewportSize.y - windowHeight) * 0.5f // Center vertically
					);
					ImGui::SetNextWindowPos(windowPos);
				}

				if (ImGui::BeginPopupModal("Delete?##ContentBrowserDeletePath", &delete_file_path, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {

					ImGui::Dummy({ 0.0f, 5.0f });

					// Center the "Are you sure?" text
					ImVec2 windowSize = ImGui::GetWindowSize();
					float textWidth = ImGui::CalcTextSize("Are you sure?").x;
					ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);
					ImGui::Text("Are you sure?");

					ImGui::Dummy({ 0.0f, 5.0f });

					// Calculate button width and center them
					float buttonWidth = ImGui::GetContentRegionAvail().x * 0.4f;
					float centerOffset = (windowSize.x - buttonWidth * 2 - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

					// Center and display buttons
					ImGui::SetCursorPosX(centerOffset);
					if (ImGui::Button("Yes", ImVec2(buttonWidth, 0.0f))) {
						try {
							std::filesystem::remove_all(path_to_delete);
						}
						catch (const std::filesystem::filesystem_error& e) {
							L_APP_WARN("Could Not Delete {0}: {1}.", std::filesystem::is_directory(path_clicked) ? "Folder" : "File", e.what());
						}

						delete_file_path = false;
						path_to_delete = "";
					}

					ImGui::SameLine();
					if (ImGui::Button("No", ImVec2(buttonWidth, 0.0f))) {
						delete_file_path = false;
						path_to_delete = "";
					}

					ImGui::EndPopup();
				}

				// End the table
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();

		directory_folder_drop_target(m_CurrentDirectory);


		// -------------        File Slider        -------------
		//ImGui::TableNextRow();
		//static float scale = 64.0f;
		//if (ImGui::BeginChild("File Slider")) {
		//	ImGui::SliderFloat("##IconScaling", &scale, 32.0f, 512.0f);
		//}
		//ImGui::EndChild();

		// End the table
		ImGui::EndTable();
	}

}

void ContentBrowserPanel::AssetFileListener::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename)
{

	if (!Louron::AssetManager::IsExtensionSupported(std::filesystem::path(filename).extension()))
		return;

	if (action == efsw::Actions::Add) // ADDEDD OR MOVED INTO
	{
		std::filesystem::path new_path = dir + filename;
		if (m_AssetFileChanges.find(filename) != m_AssetFileChanges.end()) // Existing Asset Moved
		{
			std::filesystem::path& old_path = m_AssetFileChanges[filename];
			std::filesystem::path meta_old_path = m_AssetFileChanges[filename].string() + ".meta";

			if (std::filesystem::exists(meta_old_path)) // Rename old Meta Data File and Update Asset Manager Registry
			{
				AssetHandle handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(old_path, Project::GetActiveProject()->GetAssetDirectory());

				if (handle != NULL_UUID)
				{
					AssetMetaData meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(handle);
					std::filesystem::rename(meta_old_path, new_path.string() + ".meta");

					meta_data.FilePath = std::filesystem::relative(new_path, Project::GetActiveProject()->GetAssetDirectory());

					Engine::Get().SubmitToMainThread([this, handle, meta_data]() {
						Project::GetStaticEditorAssetManager()->UpdateAssetMetaData(handle, meta_data);
					});

				}
				else // Reimport
				{
					Engine::Get().SubmitToMainThread([this, new_path]() {
						Project::GetStaticEditorAssetManager()->ReImportAsset(new_path, Project::GetActiveProject()->GetAssetDirectory());
					});
				}

			}
			else // Reimport
			{
				Engine::Get().SubmitToMainThread([this, new_path]() {
					Project::GetStaticEditorAssetManager()->ReImportAsset(new_path, Project::GetActiveProject()->GetAssetDirectory());
				});
			}

			m_AssetFileChanges.erase(filename);
		}
		else // New Import
		{
			Engine::Get().SubmitToMainThread([this, new_path]() {
				Project::GetStaticEditorAssetManager()->ReImportAsset(new_path, Project::GetActiveProject()->GetAssetDirectory());
			});
		}
	}

	if (action == efsw::Actions::Delete) // MOVED OR DELETED - cannot determine if deleted at this stage as it could either be moved or deleted
	{
		m_AssetFileChanges[filename] = std::filesystem::path(dir + filename);
	}


	if (action == efsw::Actions::Modified) // MODIFIED
	{
		Engine::Get().SubmitToMainThread([this, dir, filename]() {
			Project::GetStaticEditorAssetManager()->ReImportAsset(dir + filename, Project::GetActiveProject()->GetAssetDirectory());
		});

	}


	if (action == efsw::Actions::Moved) // RENAMED
	{
		std::filesystem::path new_path = dir + filename;
		std::filesystem::path old_path = dir + oldFilename;
		std::filesystem::path meta_old_path = old_path.string() + ".meta";

		if (std::filesystem::exists(meta_old_path)) // Rename old Meta Data File and Update Asset Manager Registry
		{
			AssetHandle handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(old_path, Project::GetActiveProject()->GetAssetDirectory());

			if (handle != NULL_UUID)
			{
				AssetMetaData meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(handle);
				std::filesystem::rename(meta_old_path, new_path.string() + ".meta");

				meta_data.AssetName = new_path.stem().string();
				meta_data.FilePath = std::filesystem::relative(new_path, Project::GetActiveProject()->GetAssetDirectory());

				Engine::Get().SubmitToMainThread([this, handle, meta_data]() {
					Project::GetStaticEditorAssetManager()->UpdateAssetMetaData(handle, meta_data);
				});

			}
			else // Reimport
			{
				Engine::Get().SubmitToMainThread([this, new_path]() {
					Project::GetStaticEditorAssetManager()->ReImportAsset(new_path, Project::GetActiveProject()->GetAssetDirectory());
				});
			}

		}
		else // Reimport
		{
			Engine::Get().SubmitToMainThread([this, new_path]() {
				Project::GetStaticEditorAssetManager()->ReImportAsset(new_path, Project::GetActiveProject()->GetAssetDirectory());
			});
		}
	}
}
