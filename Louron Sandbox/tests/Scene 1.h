#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Management/Input.h"
#include "Management/InstanceManager.h"
#include "OpenGL/Shader.h"

class Scene1 : public State {

private:

	InstanceManager* m_InstanceManager;

	InputManager* m_Input;
	ShaderLibrary* m_ShaderLib;

	glm::vec4 back_colour = glm::vec4( 0.75f, 0.90f, 1.0f, 1.0f );
	glm::vec4 fore_colour = glm::vec4( 1.00f, 0.65f, 1.0f, 1.0f );

	unsigned int triangleVAO = NULL;
	unsigned int triangleVBO = NULL;
	unsigned int triangleEBO = NULL;
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

	Scene1(InstanceManager* instanceManager)
		: m_InstanceManager(instanceManager)
	{
		std::cout << "[L20] Opening Scene 1..." << std::endl;
		m_Input = m_InstanceManager->getInputInstance();
		m_ShaderLib = m_InstanceManager->getShaderLibInstance();
			
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
			
	}
	~Scene1() override
	{
		std::cout << "[L20] Closing Scene 1..." << std::endl;
		glDeleteVertexArrays(1, &triangleVAO);
		glDeleteBuffers(1, &triangleVBO);
		glDeleteBuffers(1, &triangleEBO);
	}

	void update() override {
		if (m_Input->GetKeyDown(GLFW_KEY_F))
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (m_Input->GetKeyDown(GLFW_KEY_W))
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}


	void draw() override {
			
		processGUI();
		glClear(GL_COLOR_BUFFER_BIT);

		Shader* shader = m_ShaderLib->getShader("basic");
		if (shader)
		{
			glBindVertexArray(triangleVAO);

			shader->Bind();
			shader->setMat4("model", glm::mat4(1.0f));
			shader->setMat4("proj", glm::mat4(1.0f));
			shader->setMat4("view", glm::mat4(1.0f));
			shader->setVec4("ourColour", fore_colour);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		shader->UnBind();
	}

private:


	void processGUI() {

		ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
		ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(ImVec2(300.0f, 400.0f));

		ImGui::Text("F11 = Toggle Fullscreen");

		static bool wireFrame = false;
		ImGui::Checkbox("Wireframe Mode", &wireFrame);
		if (!wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		ImGui::Separator();

		ImGui::ColorPicker4("Background", glm::value_ptr(back_colour));
		ImGui::ColorPicker4("Triangles", glm::value_ptr(fore_colour));

		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);

		ImGui::End();
	}
	
	
};