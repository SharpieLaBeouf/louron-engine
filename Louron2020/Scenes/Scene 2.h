#pragma once

#include <iostream>
#include <stack>

#include "../Headers/imgui/imgui.h"
#include "../Headers/SceneState.h"
#include "../Headers/Input.h"
#include "../Headers/Shader.h"
#include "../Headers/Texture.h"
#include "../Headers/Entity.h"

namespace State {

	class Scene2 : public SceneState {

	private:

		float back_colour[4] = { 0.992f, 0.992f, 0.588f, 1.0f };
		float fore_colour[4] = { 1.0f  , 1.0f  , 1.0f  , 1.0f };

		InputManager m_Input;
		std::stack<std::unique_ptr<State::SceneState>>* m_States;

		Transform trans;
		unsigned int VAO = NULL;
		unsigned int VBO = NULL;
		unsigned int EBO = NULL;
		float vertices[20] = {
			 0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top right
			 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom right
			-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom left
			-0.5f,  0.5f, 0.0f,  0.0f, 1.0f  // top left 
		};
		unsigned int indices[6] = {
			0, 1, 3,
			1, 2, 3
		};

		Shader* textureShader = nullptr;
		Texture* texture = nullptr;


	public:

		Scene2(std::stack<std::unique_ptr<State::SceneState>>* SceneStates) : m_States(SceneStates) {
			std::cout << "[L20] Opening Scene 2..." << std::endl;

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			texture = new Texture("Resources/Images/carbon_fibre_texture.jpg");
			textureShader = new Shader("Resources/Shaders/basic_texture.glsl");
			textureShader->setInt("ourTexture", 0);
		}

		~Scene2() override
		{
			std::cout << "[L20] Closing Scene 2..." << std::endl;
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
		}

	private:
		float currentTime;
		float deltaTime;
		float lastTime = 0;

	public:


		void update(Window* wnd) override {
			currentTime = glfwGetTime();
			deltaTime = currentTime - lastTime;
			lastTime = currentTime;
		}

		void draw() override {

			processGUI();
			glClear(GL_COLOR_BUFFER_BIT);

			trans.Rotation.z += deltaTime;

			textureShader->setMat4("transform", trans.getTransform());
			textureShader->setVec4("ourColour", fore_colour[0], fore_colour[1], fore_colour[2], fore_colour[3]);
			textureShader->Bind();

			glBindVertexArray(VAO);
			glBindTexture(GL_TEXTURE_2D, texture->getID());
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindTexture(GL_TEXTURE_2D, 0);

			textureShader->UnBind();

		}

	private:
		void processGUI() {

			ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
			
			ImGui::Text("Z Value: %f", trans.Rotation.z);

			ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
			ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetWindowSize(ImVec2(300.0f, 400.0f));

			ImGui::Text("F9 = Toggle FPS\nF10 = Toggle Console\nF11 = Toggle Fullscreen");

			static bool wireFrame = false;
			ImGui::Checkbox("Wireframe Mode", &wireFrame);
			if (!wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			ImGui::Separator();

			ImGui::ColorPicker4("Background", back_colour);
			ImGui::ColorPicker4("Triangles", fore_colour);

			glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);

			ImGui::End();
		}

	};

}