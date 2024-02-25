#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

#include "Scene/Scene Serializer.h"

class Scene12 : public TestScene {

private:

	std::shared_ptr<Louron::Project> m_Project;
	std::shared_ptr<Louron::Scene> m_Scene;

	Louron::Window& m_Window;
	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	glm::vec3 m_LightPosition = glm::vec3(0.0);

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;

public:
	Scene12() :
		m_Window(Louron::Engine::Get().GetWindow()),
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary())
	{
		L_APP_INFO("Loading Scene 12");
		
		// Load Project and Get Active Scene Handle
		m_Project = Louron::Project::LoadProject("Sandbox Project/Sandbox Project.lproj");
		m_Scene = Louron::Project::GetActiveScene();

		m_LightPosition = m_Scene->FindEntityByName("Light Source 0").GetComponent<Louron::Transform>().GetPosition();
	}

	~Scene12() {
		L_APP_INFO("Unloading Scene 12");
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
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>().direction = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetCameraDirection(), 1.0f);

		if (m_Input.GetKeyDown(GLFW_KEY_F)) {
			Louron::SpotLightComponent& spotLight = m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>();
			spotLight.lightProperties.active = (spotLight.lightProperties.active == GL_TRUE) ? GL_FALSE : GL_TRUE;
		}

		// Update Cherry Picked Light Properties
		m_Scene->FindEntityByName("Light Source 0").GetComponent<Louron::Transform>().SetPosition(m_LightPosition);

		Draw();
	}

	void UpdateGUI() override {

		m_Scene->OnUpdateGUI();

		ImGui::Begin("Application Scene Control", (bool*)0,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoCollapse
		);

		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));

		ImGui::Separator();

		if (ImGui::TreeNode("Project and Scene Menu")) {

			std::string pipeline;
			switch (m_Scene->GetConfig().ScenePipelineType) {
			case Louron::L_RENDER_PIPELINE::FORWARD:
				pipeline = "Forward";
				break;
			case Louron::L_RENDER_PIPELINE::FORWARD_PLUS:
				pipeline = "Forward Plus";
				break;
			case Louron::L_RENDER_PIPELINE::DEFERRED:
				pipeline = "Deferred";
				break;
			}

			ImGui::Text(("Project Loaded: " + m_Project->GetConfig().Name).c_str());
			ImGui::Text(("Scene Loaded: " + m_Scene->GetConfig().Name).c_str());
			
			ImGui::Separator();

			ImGui::Text(("Asset Directory: " + m_Scene->GetConfig().AssetDirectory.string()).c_str());
			ImGui::Text(("Rendering Technique: " + pipeline).c_str());

			ImGui::Separator();

			if (ImGui::Button("Save Project")) m_Project->SaveProject();
			if (ImGui::Button("Save Scene")) m_Project->SaveScene();
			

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Light Properties")) {
			ImGui::DragFloat3("Position", &m_LightPosition[0], 0.1f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Profiling")) {

			for (auto& result : Louron::Profiler::Get().GetResults()) {

				char label[128];
				strcpy_s(label, result.second.Name);
				strcat_s(label, " %.3fms");

				ImGui::Text(label, result.second.Time);
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}

private:

	void Draw() override {

		Louron::Renderer::ClearColour(glm::vec4(0.0f));

		m_Scene->OnUpdate();

	}

};