#pragma once

#include <stack>
#include <iostream>

#include <imgui/imgui.h>

#include "Louron.h"
#include "Test Scene Base.h"

class Scene1 : public Scene {

private:

	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;

	glm::vec4 back_colour = glm::vec4(0.75f, 0.90f, 1.0f, 1.0f);
	glm::vec4 fore_colour = glm::vec4(1.00f, 0.65f, 1.0f, 1.0f);

	unsigned int triangleVAO = -1;
	unsigned int triangleVBO = -1;
	unsigned int triangleEBO = -1;
	float triangle_vertices[12] = {
			0.5f,  0.5f, 0.0f,  // top right
			0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};
	unsigned int indices[6] = {
		0, 1, 3,
		1, 2, 3
	};

public:

	Scene1() : m_Input(Louron::Engine::Get().GetInput()), m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()) {
		std::cout << "[L20] Opening Scene 1..." << std::endl;

		glGenVertexArrays(1, &triangleVAO);
		glGenBuffers(1, &triangleVBO);
		glGenBuffers(1, &triangleEBO);
		glBindVertexArray(triangleVAO);

		glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_ShaderLib.LoadShader("assets/Shaders/Basic/basic.glsl");

	}
	~Scene1() override {
		std::cout << "[L20] Closing Scene 1..." << std::endl;
		glDeleteVertexArrays(1, &triangleVAO);
		glDeleteBuffers(1, &triangleVBO);
		glDeleteBuffers(1, &triangleEBO);
	}

	void Update() override {
		if (m_Input.GetKeyDown(GLFW_KEY_F))
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (m_Input.GetKeyDown(GLFW_KEY_W))
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		Draw();
	}

	void UpdateGUI() override {

		ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
		ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(ImVec2(300.0f, 400.0f));

		ImGui::Text("F11 = Toggle Fullscreen");

		static bool wireFrame = false;
		ImGui::Checkbox("Wireframe Mode", &wireFrame);

		wireFrame ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ImGui::Separator();

		ImGui::ColorPicker4("Background", glm::value_ptr(back_colour));
		ImGui::ColorPicker4("Triangles", glm::value_ptr(fore_colour));


		ImGui::End();
	}

private:
	void Draw() override {
		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);
		glClear(GL_COLOR_BUFFER_BIT);

		Louron::Shader* shader = m_ShaderLib.GetShader("basic");
		if (shader)
		{
			glBindVertexArray(triangleVAO);

			shader->Bind();
			shader->SetMat4("model", glm::mat4(1.0f));
			shader->SetMat4("proj", glm::mat4(1.0f));
			shader->SetMat4("view", glm::mat4(1.0f));
			shader->SetVec4("ourColour", fore_colour);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		shader->UnBind();
	}

};