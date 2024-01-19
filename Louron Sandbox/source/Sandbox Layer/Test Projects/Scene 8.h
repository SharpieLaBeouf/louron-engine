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
		std::cout << "[L20] Loading Scene 8..." << std::endl;
				
		// Create Entity for Monkey and Load Applicable Model
		Louron::Entity MonkeyEntity = m_Scene->CreateEntity("Monkey");
		MonkeyEntity.AddComponent<Louron::MaterialComponent>("FP_Material_BP_Shader");
		MonkeyEntity.AddComponent<Louron::MeshComponent>().LoadModel("assets/Models/Monkey/Pink_Monkey.fbx", MonkeyEntity.GetComponent<Louron::MaterialComponent>());
		
		// Create Entity for Camera and Set to Primary Camera
		m_Scene->CreateEntity("Main Camera").AddComponent<Louron::CameraComponent>().Camera = new Louron::Camera(glm::vec3(0.0f, 0.0f, 10.0f));
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Primary = true;

		m_Scene->CreateEntity("Flash Light").AddComponent<Louron::SpotLightComponent>();
	 }

	~Scene8() override
	{
		std::cout << "[L20] Unloading Scene 8..." << std::endl;
			
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->Update(deltaTime);
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::TransformComponent>().position = m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetPosition();

		m_Scene->FindEntityByName("Flash Light").GetComponent<Louron::TransformComponent>().position = m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::TransformComponent>().position;
		m_Scene->FindEntityByName("Flash Light").GetComponent<Louron::SpotLightComponent>().direction = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetCameraDirection(), 1.0f);

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

		glEnable(GL_DEPTH_TEST);
		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw All Entities in Scene
		m_Scene->OnUpdate();

		glDisable(GL_DEPTH_TEST);
	}

	glm::vec4 back_colour = { 0.3137f, 0.7843f, 1.0f, 1.0f };

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
};
