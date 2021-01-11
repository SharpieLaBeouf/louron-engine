#pragma once

#include <iostream>
#include <stack>

#include "../Vendor/imgui/imgui.h"

#include "../Headers/Input.h"
#include "../Headers/Entity.h"
#include "../Headers/Camera.h"
#include "../Headers/InstanceManager.h"

#include "../Headers/Abstracted GL/Light.h"
#include "../Headers/Abstracted GL/Shader.h"
#include "../Headers/Abstracted GL/Texture.h"
#include "../Headers/Abstracted GL/Material.h"
#include "../Headers/Abstracted GL/MeshRenderer.h"

namespace State {
	class Scene7 : public SceneState {

		//Private Setup Variables
	private:

		State::InstanceManager* m_InstanceManager;

		Window* m_Window;
		InputManager* m_Input;
		ShaderLibrary* m_ShaderLib;
		TextureLibrary* m_TextureLib;

		Camera* scnCamera;
		Light light_properties;

		MeshRenderer* monkey;
		MeshRenderer* back_pack;

	public:

		explicit Scene7(InstanceManager* instanceManager)
			: m_InstanceManager(instanceManager)
		{
			std::cout << "[L20] Opening Scene 7..." << std::endl;
			m_Window = m_InstanceManager->getWindowInstance();
			m_Input = m_InstanceManager->getInputInstance();
			m_ShaderLib = m_InstanceManager->getShaderLibInstance();
			m_TextureLib = m_InstanceManager->getTextureLibInstance();

			glEnable(GL_DEPTH_TEST);
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			scnCamera = new Camera(m_Window, glm::vec3(0.0f, 0.0f, 10.0f));

			back_pack = new MeshRenderer(m_InstanceManager);
			back_pack->loadModel("Resources/Models/BackPack/backpack.obj", "material_shader_phong");

			monkey = new MeshRenderer(m_InstanceManager);
			monkey->loadModel("Resources/Models/Monkey/Monkey.fbx", "material_shader_phong");
		}

		~Scene7() override
		{
			std::cout << "[L20] Closing Scene 7..." << std::endl;
			glDisable(GL_DEPTH_TEST);
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			delete monkey;
			delete scnCamera;
		}

		void update() override {

			currentTime = (float)glfwGetTime();
			deltaTime = currentTime - lastTime;
			lastTime = currentTime;

			scnCamera->Update(deltaTime);

			if (m_Input->GetKeyUp(GLFW_KEY_LEFT_ALT)) {
				scnCamera->MouseToggledOff = !scnCamera->MouseToggledOff;
				if (scnCamera->MouseToggledOff)
					glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				else if (!scnCamera->MouseToggledOff)
					glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			light_properties.position.z = sin(currentTime * 5);
		}

		void draw() override {

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			monkey->renderEntireMesh(scnCamera, &light_properties);

			back_pack->renderEntireMesh(scnCamera, &light_properties);

			processGUI();
		}

	private:

		void processGUI() {

			static bool wireFrame = false;

			ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

			ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
			ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetWindowSize(ImVec2(300.0f, 400.0f));

			ImGui::Text("F9 = Toggle FPS\nF10 = Toggle Console\nF11 = Toggle Fullscreen");
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

			glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);

			ImGui::End();

		}

		glm::vec4 back_colour = { 0.3137f, 0.7843f, 1.0f, 1.0f };

		float currentTime = 0;
		float deltaTime = 0;
		float lastTime = 0;
	};
}