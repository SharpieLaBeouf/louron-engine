#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Management/InstanceManager.h"
#include "Scene/SceneManager.h"

class Scene2 : public State {

//Private Setup Variables
private:

	InstanceManager* m_InstanceManager = nullptr;

	Window* m_Window = nullptr;
	ShaderLibrary* m_ShaderLib = nullptr;
	TextureLibrary* m_TexLib = nullptr;

//Constructors
public:

	explicit Scene2(InstanceManager* instanceManager)
		: m_InstanceManager(instanceManager)
	{
		std::cout << "[L20] Opening Scene 2..." << std::endl;
		m_Window = m_InstanceManager->getWindowInstance();
		m_ShaderLib = m_InstanceManager->getShaderLibInstance();
		m_TexLib = m_InstanceManager->getTextureLibInstance();

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

		m_ShaderLib->getShader("basic_texture")->Bind();
		m_ShaderLib->getShader("basic_texture")->setInt("ourTexture", 0);
	}
	~Scene2() override
	{
		std::cout << "[L20] Closing Scene 2..." << std::endl;
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}


//Private Scene Variables
private:
	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
	int speed = 1;

	glm::vec4 back_colour = glm::vec4( 0.992f, 0.992f, 0.588f, 1.0f );
	glm::vec4 fore_colour = glm::vec4( 1.0f  , 1.0f  , 1.0f  , 1.0f );

	TransformComponent trans;
		
//Public Functions
public:

	void update() override {
		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
	}

	void draw() override {

		processGUI();
		glClear(GL_COLOR_BUFFER_BIT);

		trans.rotation.y += deltaTime * speed;

		Shader* shader = m_ShaderLib->getShader("basic_texture");
		if (shader)
		{
			glBindVertexArray(VAO);

			shader->Bind();
			shader->setMat4("proj", glm::perspective(glm::radians(60.0f), m_Window->getWidth() / m_Window->getHeight(), 0.1f, 100.0f));
			shader->setMat4("view", glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
			shader->setMat4("model", trans.getTransform());
			shader->setVec4("ourColour", fore_colour);

			m_TexLib->getTexture("cube_texture")->Bind();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			m_TexLib->UnBind();
		}
		shader->UnBind();
	}


//Private
private:
	void processGUI() {

		ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
			
		ImGui::InputInt("Speed", &speed);
		ImGui::DragFloat3("Translate", glm::value_ptr(trans.position), 0.01f, 0, 0, "%.2f");
		ImGui::DragFloat3("Rotate", glm::value_ptr(trans.rotation), 1.0f, 0, 0, "%.2f");
		ImGui::DragFloat3("Scale", glm::value_ptr(trans.scale), 0.01f, 0, 0, "%.2f");

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

};
