#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Louron.h"
#include "Test Scene Base.h"


class Scene8 : public TestScene {

	//Private Setup Variables
private:

	std::shared_ptr<Louron::Scene> m_Scene;

	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	std::shared_ptr<Louron::ForwardPlusPipeline> m_Pipeline;

public:

	 Scene8() :
		 m_Input(Louron::Engine::Get().GetInput()),
		 m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		 m_TextureLib(Louron::Engine::Get().GetTextureLibrary()),
		 m_Pipeline(nullptr),
		 m_Scene(nullptr)
	{
		L_APP_INFO("Loading Scene 8");

		m_Pipeline = std::make_shared<Louron::ForwardPlusPipeline>();
		m_Scene = std::make_shared<Louron::Scene>("Scene 8", Louron::L_RENDER_PIPELINE::FORWARD_PLUS);

		const auto& resources = m_Scene->GetResources();
		resources->LinkShader(Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"));
		resources->LoadMesh("assets/Models/Monkey/Pink_Monkey.fbx", resources->Shaders["FP_Material_BP_Shader"]);
		resources->LoadMesh("assets/Models/Torch/model/obj/Torch.obj", resources->Shaders["FP_Material_BP_Shader"]);
		
		// Create Entity for Monkey and Load Applicable Model
		Louron::Entity entity = m_Scene->CreateEntity("Monkey");

		entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("Pink_Monkey"));
		entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("Pink_Monkey"));

		// Create Entity for Camera and Set to Primary Camera
		entity = m_Scene->CreateEntity("Main Camera");
		entity.AddComponent<Louron::CameraComponent>().Camera = std::make_shared<Louron::Camera>(glm::vec3(0.0f, 0.0f, 10.0f));
		entity.GetComponent<Louron::CameraComponent>().Primary = true;

		entity = m_Scene->CreateEntity("Flash Light");
		entity.AddComponent<Louron::SpotLightComponent>();

		entity = m_Scene->CreateEntity("Point Light");
		entity.GetComponent<Louron::Transform>().SetPositionY(7.0f);
		entity.AddComponent<Louron::PointLightComponent>();
		entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("Torch"));
		entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("Torch"));
	 }

	~Scene8() override
	{
		L_APP_INFO("Unloading Scene 8");
			
	}

	void OnAttach() override { 

		lastTime = (float)glfwGetTime();
		m_Scene->OnStart();
	}
	void OnDetach() override { 

		m_Scene->OnStop();
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->Update(deltaTime);
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetPosition());
	
		m_Scene->FindEntityByName("Flash Light").GetComponent<Louron::Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::Transform>().GetPosition());
		m_Scene->FindEntityByName("Flash Light").GetComponent<Louron::SpotLightComponent>().direction = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetCameraDirection(), 1.0f);
		m_Scene->FindEntityByName("Point Light").GetComponent<Louron::Transform>().TranslateX(sin(currentTime) * deltaTime);

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

		glClearColor(back_colour.r, back_colour.g, back_colour.b, back_colour.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Scene->OnUpdate();
	}

	glm::vec4 back_colour = { 0.3137f, 0.7843f, 1.0f, 1.0f };

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
};
