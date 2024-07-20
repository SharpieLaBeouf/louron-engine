#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Louron.h"
#include "Test Scene Base.h"

class Scene7 : public TestScene {

	//Private Setup Variables
private:

	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	Louron::Camera m_SceneCamera{ glm::vec3(0.0f, 0.0f, 10.0f) };

	Louron::Transform monkeyTransform{ {-5.0f, 0.0f, 0.0f} };
	Louron::MeshFilter monkeyFilter{};
	Louron::MeshRenderer monkeyRenderer{};

	Louron::Transform backPackTransform{ {5.0f, 0.0f, 0.0f} };
	Louron::MeshFilter backPackFilter{};
	Louron::MeshRenderer backPackRenderer{};

public:

	Scene7() :
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary())
	{
		L_APP_INFO("Loading Scene 7");

		m_SceneCamera.MouseToggledOff = false;
		m_SceneCamera.MovementSpeed = 10.0f;
		m_SceneCamera.MovementYDamp = 0.65f;

		monkeyRenderer.LoadModelFromFile("Sandbox Project/Assets/Models/Monkey/Monkey.fbx", monkeyFilter);
		backPackRenderer.LoadModelFromFile("Sandbox Project/Assets/Models/BackPack/BackPack.fbx", backPackFilter);
	}

	~Scene7() override
	{
		L_APP_INFO("Unloading Scene 7");
		glDisable(GL_DEPTH_TEST);
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
	}

	void OnAttach() override {
		glEnable(GL_DEPTH_TEST);
		lastTime = (float)glfwGetTime();

		m_SceneCamera.UpdateProjMatrix();
	}

	void OnDetach() override {

		glDisable(GL_DEPTH_TEST);
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		m_SceneCamera.Update(deltaTime);

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

		wireFrame ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

		monkeyTransform.SetTransform(monkeyTransform.GetLocalTransform());
		backPackTransform.SetTransform(backPackTransform.GetLocalTransform());

		monkeyRenderer.ManualDraw(monkeyFilter, m_SceneCamera, monkeyTransform);
		backPackRenderer.ManualDraw(backPackFilter, m_SceneCamera, backPackTransform);

	}

	glm::vec4 back_colour = { 0.3137f, 0.7843f, 1.0f, 1.0f };

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
};