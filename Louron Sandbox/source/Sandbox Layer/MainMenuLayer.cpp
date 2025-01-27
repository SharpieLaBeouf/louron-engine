#include "MainMenuLayer.h"

MainMenuLayer::MainMenuLayer() {
	L_APP_INFO("Opening Main Menu");
	
}

void MainMenuLayer::OnAttach() {
	m_Scenes.push_back(std::make_unique<Scene1>());
	m_Scenes.push_back(std::make_unique<Scene2>());
	m_Scenes.push_back(std::make_unique<Scene3>());
	m_Scenes.push_back(std::make_unique<Scene4>());
	m_Scenes.push_back(std::make_unique<Scene5>());
	m_Scenes.push_back(std::make_unique<Scene6>());
	m_Scenes.push_back(std::make_unique<Scene7>());
	m_Scenes.push_back(std::make_unique<Scene8>());
	m_Scenes.push_back(std::make_unique<Scene9>());
	m_Scenes.push_back(std::make_unique<Scene10>());
	m_Scenes.push_back(std::make_unique<Scene11>());
	m_Scenes.push_back(std::make_unique<Scene12>());
	m_Scenes.push_back(std::make_unique<Scene13>());
	m_Scenes.push_back(std::make_unique<Scene14>());
	m_Scenes.push_back(std::make_unique<Scene15>());
	m_Scenes.push_back(std::make_unique<Scene16>());

	Louron::Engine::Get().GetWindow().SetVSync(false);
}

void MainMenuLayer::OnDetach() {
	m_Scenes.clear();
}

void MainMenuLayer::OnUpdate() {

	if (Louron::Engine::Get().GetInput().GetKeyUp(GLFW_KEY_ESCAPE)) {
		if (m_SceneSelector == 0) {
			L_APP_INFO("Closing Window");
			Louron::Engine::Get().Close();
		}
		else {
			m_Scenes[m_SceneSelector - 1]->OnDetach();
			m_SceneSelector = 0;
		}
	}

	switch (m_SceneSelector) {
	default:
		if (m_Scenes[m_SceneSelector - 1] != nullptr)
			m_Scenes[m_SceneSelector - 1]->Update();
		else
			L_APP_INFO("This Scene is Null - Cannot Update");
		break;

	case 0:
		glClearColor(211.0f / 255.0f, 238.0f / 255.0f, 255.0f / 255.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	}
}

void MainMenuLayer::OnFixedUpdate() {

	if (m_SceneSelector != 0) {
		if (m_Scenes[m_SceneSelector - 1] != nullptr)
			m_Scenes[m_SceneSelector - 1]->FixedUpdate();
		else
			L_APP_INFO("This Scene is Null - Cannot FixedUpdate");
	}
}

void MainMenuLayer::OnGuiRender() {
	ImGuiIO& io = ImGui::GetIO();

	switch (m_SceneSelector) {
	case 0: {
		// Main Menu
		ImGui::SetNextWindowBgAlpha(1.0f);
		if (ImGui::Begin("Main Menu", (bool*)0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove))
		{
			glm::vec2 menuSize = { 600.0f, 400.0f };
			ImGui::SetWindowSize(ImVec2(menuSize.x, menuSize.y), ImGuiCond_Always);
			ImGui::SetWindowPos(ImVec2(io.DisplaySize.x / 2 - menuSize.x / 2, io.DisplaySize.y / 2 - menuSize.y / 2), ImGuiCond_Always);
			ImGui::Text("Main Menu");
			ImGui::Separator();
			if (ImGui::Button(" 1. Basic Triangles        ")) {
				L_APP_INFO("Menu Item 1 Pressed");
				m_SceneSelector = 1;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}
			if (ImGui::Button(" 2. Basic Texture          ")) {
				L_APP_INFO("Menu Item 2 Pressed");
				m_SceneSelector = 2;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}
			if (ImGui::Button(" 3. Basic Cube             ")) {
				L_APP_INFO("Menu Item 3 Pressed");
				m_SceneSelector = 3;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}
			if (ImGui::Button(" 4. Basic Camera           ")) {
				L_APP_INFO("Menu Item 4 Pressed");
				m_SceneSelector = 4;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}
			if (ImGui::Button(" 5. Basic Lighting         ")) {
				L_APP_INFO("Menu Item 5 Pressed");
				m_SceneSelector = 5;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}
			if (ImGui::Button(" 6. Basic Material         ")) {
				L_APP_INFO("Menu Item 6 Pressed");
				m_SceneSelector = 6;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}
			if (ImGui::Button(" 7. Basic Model Loading    ")) {
				L_APP_INFO("Menu Item 7 Pressed");
				m_SceneSelector = 7;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}
			if (ImGui::Button(" 8. Entity System          ")) {
				L_APP_INFO("Menu Item 8 Pressed");
				m_SceneSelector = 8;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}
			if (ImGui::Button(" 9. First Rudimentary Game ")) {
				L_APP_INFO("Menu Item 9 Pressed");
				m_SceneSelector = 9;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}

			if (ImGui::Button(" 10. Pong Clone            ")) {
				L_APP_INFO("Menu Item 10 Pressed");
				m_SceneSelector = 10;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}

			if (ImGui::Button(" 11. Forward+ Rendering    ")) {
				L_APP_INFO("Menu Item 11 Pressed");
				m_SceneSelector = 11;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}

			if (ImGui::Button(" 12. Scene Serialization   ")) {
				L_APP_INFO("Menu Item 12 Pressed");
				m_SceneSelector = 12;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}

			if (ImGui::Button(" 13. PhysX                 ")) {
				L_APP_INFO("Menu Item 13 Pressed");
				m_SceneSelector = 13;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}

			if (ImGui::Button(" 14. Knerpix Remake        ")) {
				L_APP_INFO("Menu Item 14 Pressed");
				m_SceneSelector = 14;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}

			if (ImGui::Button(" 15. Hierarchy System      ")) {
				L_APP_INFO("Menu Item 15 Pressed");
				m_SceneSelector = 15;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}

			if (ImGui::Button(" 16. PBR Implementation    ")) {
				L_APP_INFO("Menu Item 16 Pressed");
				m_SceneSelector = 16;
				m_Scenes[m_SceneSelector - 1]->OnAttach();
			}
		}
		ImGui::End();
	}break;

	default:
		if (m_Scenes[m_SceneSelector - 1] != nullptr)
			m_Scenes[m_SceneSelector - 1]->UpdateGUI();
		else {
			ImGui::SetNextWindowBgAlpha(1.0f);
			if (ImGui::Begin("Main Menu", (bool*)0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove))
			{
				glm::vec2 menuSize = { 600.0f, 400.0f };
				ImGui::SetWindowSize(ImVec2(menuSize.x, menuSize.y), ImGuiCond_Always);
				ImGui::SetWindowPos(ImVec2(io.DisplaySize.x / 2 - menuSize.x / 2, io.DisplaySize.y / 2 - menuSize.y / 2), ImGuiCond_Always);

				ImGui::Text("This Scene is NULL!");
				if (ImGui::Button("Return")) m_SceneSelector = 0;
			}
			ImGui::End();
		}
		break;

	}

	if(m_SceneSelector != 14) {
		// Simple FPS Counter
		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10.0f, io.DisplaySize.y - 10.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(100.0f, 50.0f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.35f);
		if (ImGui::Begin("Simple FPS Overlay", (bool*)0, window_flags))
		{
			ImGui::Text("FPS Counter");
			ImGui::Separator();
			ImGui::Text("%.0f", io.Framerate);
		}
		ImGui::End();
	}
}
