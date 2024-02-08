#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

#include "Scene/Scene Serializer.h"

class Scene12 : public TestScene {

private:

	std::shared_ptr<Louron::Scene> m_Scene;
	std::shared_ptr<Louron::ForwardPlusPipeline> m_Pipeline;

	Louron::Window& m_Window;
	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;

public:
	Scene12() :
		m_Window(Louron::Engine::Get().GetWindow()),
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary()),
		m_Pipeline(nullptr),
		m_Scene(nullptr) 
	{
		std::cout << "[L20] Loading Scene 12..." << std::endl;

		// Scene Configuration Setup
		m_Pipeline = std::make_shared<Louron::ForwardPlusPipeline>();
		m_Scene = std::make_shared<Louron::Scene>("Scene 12", m_Pipeline);

		Louron::SceneSerializer serializer{ m_Scene };

		serializer.Deserialize();
	}

	~Scene12() {
		std::cout << "[L20] Unloading Scene 12..." << std::endl;
	}

	void OnAttach() override {

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
		if (m_Scene->HasEntity("Main Camera")) {

			m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->Update(deltaTime);
			m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetPosition());
			m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>().direction = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetCameraDirection(), 1.0f);

			if (m_Input.GetKeyDown(GLFW_KEY_F)) {
				Louron::SpotLightComponent& spotLight = m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>();
				spotLight.lightProperties.active = (spotLight.lightProperties.active == GL_TRUE) ? GL_FALSE : GL_TRUE;
			}

		}

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