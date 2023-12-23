#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Louron.h"
#include "Test Scene Base.h"

class Scene8 : public Scene {

	//Private Setup Variables
private:

	std::unique_ptr<Louron::Scene> m_Scene;

	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

public:

	 Scene8() :
		 m_Input(Louron::Engine::Get().GetInput()),
		 m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		 m_TextureLib(Louron::Engine::Get().GetTextureLibrary()),
		 m_Scene(std::make_unique<Louron::Scene>())
	{
		std::cout << "[L20] Opening Scene 8..." << std::endl;

		//m_SceneCamera = new Camera(m_Window, glm::vec3(0.0f, 0.0f, 10.0f));

		glEnable(GL_DEPTH_TEST);
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		if (m_Scene->LoadScene("hello world")) {
			std::cout << "[L20] Failed to Load Scene." << std::endl;
		}
	}

	~Scene8() override
	{
		std::cout << "[L20] Closing Scene 8..." << std::endl;
		glDisable(GL_DEPTH_TEST);
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	
}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		Draw();
	}

	void UpdateGUI() override {

		static bool wireFrame = false;

		ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

		ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(ImVec2(300.0f, 400.0f));

		ImGui::Text("F11 = Toggle Fullscreen");
		ImGui::Checkbox("Wireframe Mode", &wireFrame);

		if (!wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		ImGui::Separator();

		if (ImGui::TreeNode("Colours"))
		{
			ImGui::ColorPicker4("Background", glm::value_ptr(back_colour));
			ImGui::TreePop();
		}
		ImGui::End();
	}

private:

	void Draw() override {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);

	}

	glm::vec4 back_colour = { 0.3137f, 0.7843f, 1.0f, 1.0f };

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
};
