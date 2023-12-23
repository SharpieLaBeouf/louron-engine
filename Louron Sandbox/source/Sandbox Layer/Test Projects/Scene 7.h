#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Louron.h"
#include "Test Scene Base.h"

class Scene7 : public Scene {

	//Private Setup Variables
private:

	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	Louron::Camera* m_SceneCamera;
	Louron::Light light_properties;

	std::unique_ptr<Louron::MeshRendererComponent> monkey;
	std::unique_ptr < Louron::MeshRendererComponent> back_pack;

public:

	Scene7() :
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary())
	{
		std::cout << "[L20] Opening Scene 7..." << std::endl;

		m_SceneCamera = new Louron::Camera(glm::vec3(0.0f, 0.0f, 10.0f));
		m_SceneCamera->MouseToggledOff = false;
		m_SceneCamera->MovementSpeed = 10.0f;
		m_SceneCamera->MovementYDamp = 0.65f;

		m_ShaderLib.LoadShader("assets/Shaders/Materials/material_shader_phong.glsl");

		back_pack = std::make_unique<Louron::MeshRendererComponent>();
		back_pack->loadModel("assets/Models/BackPack/BackPack.fbx", "material_shader_phong");

		monkey = std::make_unique<Louron::MeshRendererComponent>();
		monkey->loadModel("assets/Models/Monkey/Monkey.fbx", "material_shader_phong");
	}

	~Scene7() override
	{
		std::cout << "[L20] Closing Scene 7..." << std::endl;
		glDisable(GL_DEPTH_TEST);
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		delete m_SceneCamera;
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		m_SceneCamera->Update(deltaTime);

		light_properties.position.z = sin(currentTime * 5);

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

		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);

		monkey->renderEntireMesh(m_SceneCamera, &light_properties);
		back_pack->renderEntireMesh(m_SceneCamera, &light_properties);

		glDisable(GL_DEPTH_TEST);
	}

	glm::vec4 back_colour = { 0.3137f, 0.7843f, 1.0f, 1.0f };

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
};