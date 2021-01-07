#pragma once

#include <iostream>
#include <stack>

#include "../Vendor/imgui/imgui.h"

#include "../Headers/Input.h"
#include "../Headers/Entity.h"
#include "../Headers/Camera.h"
#include "../Headers/SceneManager.h"

#include "../Headers/Abstracted GL/Shader.h"
#include "../Headers/Abstracted GL/Texture.h"

namespace State {

	class Scene4 : public SceneState {

		//Private Setup Variables
	private:

		State::SceneManager* m_SceneManager;

		Window* m_Window;
		InputManager* m_Input;
		ShaderLibrary* m_ShaderLib;
		TextureLibrary* m_TextureLib;

		//Constructors
	public:

		Scene4(SceneManager* scnMgr)
			: m_SceneManager(scnMgr)
		{
			std::cout << "[L20] Opening Scene 4..." << std::endl;
			m_Window = m_SceneManager->getWindowInstance();
			m_Input = m_SceneManager->getInputInstance();
			m_ShaderLib = m_SceneManager->getShaderLibInstance();
			m_TextureLib = m_SceneManager->getTextureLibInstance();

			glEnable(GL_DEPTH_TEST);
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			// Init Plane VAO
			glGenVertexArrays(1, &plane_VAO);
			glGenBuffers(1, &plane_VBO);
			glGenBuffers(1, &plane_EBO);

			glBindVertexArray(plane_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, plane_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane_EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_indices), plane_indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
			glEnableVertexAttribArray(1);

			// Init Cube VAO
			glGenVertexArrays(1, &cube_VAO);
			glGenBuffers(1, &cube_VBO);
			glGenBuffers(1, &cube_EBO);

			glBindVertexArray(cube_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
			glEnableVertexAttribArray(1);


			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			plane_trans.position.y = -0.6f;
			plane_trans.scale = glm::vec3(40.0f, 0.0f, 40.0f);

			sceneCamera = new Camera(m_Window, glm::vec3(0.0f, 10.0f, 25.0f));
			
			m_ShaderLib->getShader("basic_texture")->Bind();
			m_ShaderLib->getShader("basic_texture")->setInt("ourTexture", 0);
		}
		~Scene4() override
		{
			std::cout << "[L20] Closing Scene 4..." << std::endl;
			glDisable(GL_DEPTH_TEST);
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			glDeleteVertexArrays(1, &plane_VAO);
			glDeleteBuffers(1, &plane_VBO);
			glDeleteBuffers(1, &plane_EBO);

			glDeleteVertexArrays(1, &cube_VAO);
			glDeleteBuffers(1, &cube_VBO);
			glDeleteBuffers(1, &cube_EBO);

			delete sceneCamera;
		}


		//Private Scene Variables
	private:

		float currentTime = 0;
		float deltaTime = 0;
		float lastTime = 0;

		int waveSize = 20;
		int waveHeight = 10;
		float waveSpeed = 5;

		Transform cube_trans;
		Transform plane_trans;
		Camera* sceneCamera = nullptr;

		glm::vec4 back_colour = glm::vec4(0.992f, 0.325f, 0.325f, 1.0f);
		glm::vec4 box_colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 plane_colour = glm::vec4(1.0f, 0.784f, 0.313f, 1.0f);

		//Public Functions
	public:

		void update() override {

			currentTime = (float)glfwGetTime();
			deltaTime = currentTime - lastTime;
			lastTime = currentTime;
			
			sceneCamera->Update(deltaTime);

			if (m_Input->GetKeyUp(GLFW_KEY_LEFT_ALT)) {
				sceneCamera->MouseToggledOff = !sceneCamera->MouseToggledOff;
				if(sceneCamera->MouseToggledOff)
					glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				else if(!sceneCamera->MouseToggledOff)
					glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			
		}

		void draw() override {

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Shader* shader = m_ShaderLib->getShader("basic");
			if (shader)
			{
				// DRAW PLANE BENEATH
				glBindVertexArray(plane_VAO);

				shader->Bind();
				shader->setMat4("model", plane_trans.getTransform());
				shader->setMat4("proj", glm::perspective(glm::radians(60.0f), m_Window->getWidth() / m_Window->getHeight(), 0.1f, 100.0f));
				shader->setMat4("view", sceneCamera->getViewMatrix());
				shader->setVec4("ourColour", plane_colour);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}

			shader = m_ShaderLib->getShader("basic_texture");
			if (shader)
			{
				// DRAW CUBES ABOVE

				glBindVertexArray(cube_VAO);

				shader->Bind();
				shader->setMat4("proj", glm::perspective(glm::radians(60.0f), m_Window->getWidth() / m_Window->getHeight(), 0.1f, 100.0f));
				shader->setMat4("view", sceneCamera->getViewMatrix());
				shader->setVec4("ourColour", box_colour);

				m_TextureLib->getTexture("cube_texture")->Bind();
				glm::vec3 pos = glm::vec3(0.0f);
				for (int x = 1; x <= waveSize; x++)
				{
					pos.x = -waveSize / 2 + x - cube_trans.scale.x / 2;
					for (int z = 1; z <= waveSize; z++)
					{
						pos.z = -waveSize / 2 + z - cube_trans.scale.z / 2;

						double time = glfwGetTime();
						pos.y = (float)sin((time * waveSpeed + floor(x - waveSize) + floor(z - waveSize))) / waveSize * waveHeight;

						shader->setMat4("model", glm::translate(cube_trans.getTransform(), pos));
						glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
					}
				}
			}

			m_ShaderLib->UnBind();
			m_TextureLib->UnBind();

			processGUI();

		}

	private:

		void processGUI() {

			static bool wireFrame = false;

			ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

			ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
			ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetWindowSize(ImVec2(300.0f, 400.0f));

			ImGui::DragInt("Cube Amount", &waveSize, 1.0f, 0, 100);
			ImGui::DragInt("Wave Height", &waveHeight, 1.0f, 0, 100);
			ImGui::DragFloat("Wave Speed", &waveSpeed, 0.1f);
			ImGui::DragFloat3("Translate", glm::value_ptr(cube_trans.position), 0.01f, 0, 0, "%.2f");
			ImGui::DragFloat3("Rotate", glm::value_ptr(cube_trans.rotation), 1.0f, 0, 0, "%.2f");
			ImGui::DragFloat3("Scale", glm::value_ptr(cube_trans.scale), 0.01f, 0, 0, "%.2f");
			ImGui::DragFloat3("Plane Scale", glm::value_ptr(plane_trans.scale), 0.01f, 0, 0, "%.2f");

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
				ImGui::ColorPicker4("Box", glm::value_ptr(box_colour));
				ImGui::ColorPicker4("Plane", glm::value_ptr(plane_colour));
				ImGui::TreePop();
			}

			glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);

			ImGui::End();
		}

		unsigned int plane_VAO = NULL;
		unsigned int plane_VBO = NULL;
		unsigned int plane_EBO = NULL;
		float plane_vertices[20] = {
			 0.5f, 0.0f,  0.5f, 1.0f, 1.0f,
			 0.5f, 0.0f, -0.5f, 1.0f, 0.0f,
			-0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
			-0.5f, 0.0f,  0.5f, 0.0f, 1.0f
		};
		unsigned int plane_indices[6] = {
			0, 1, 3,
			1, 2, 3
		};

		unsigned int cube_VAO = NULL;
		unsigned int cube_VBO = NULL;
		unsigned int cube_EBO = NULL;
		float cube_vertices[120] = {
			 0.5f,  0.5f, 0.5f,  1.0f, 1.0f,   // front top right
			 0.5f, -0.5f, 0.5f,  1.0f, 0.0f,   // front bottom right
			-0.5f, -0.5f, 0.5f,  0.0f, 0.0f,   // front bottom left
			-0.5f,  0.5f, 0.5f,  0.0f, 1.0f,   // front top left

			 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,  // back top right
			 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // back bottom right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  // back bottom left
			-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,  // back top left

			 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,   // right top right
			 0.5f,  0.5f, -0.5f, 1.0f, 0.0f,   // right bottom right
			 0.5f, -0.5f, -0.5f, 0.0f, 0.0f,   // right bottom left
			 0.5f, -0.5f,  0.5f, 0.0f, 1.0f,   // right top left 

			-0.5f,  0.5f,  0.5f, 1.0f, 1.0f,   // left top right
			-0.5f,  0.5f, -0.5f, 1.0f, 0.0f,   // left bottom right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,   // left bottom left
			-0.5f, -0.5f,  0.5f, 0.0f, 1.0f,   // left top left 

			 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,   // top top right
			 0.5f,  0.5f, -0.5f, 1.0f, 0.0f,   // top bottom right
			-0.5f,  0.5f, -0.5f, 0.0f, 0.0f,   // top bottom left
			-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,   // top top left 

			 0.5f, -0.5f,  0.5f, 1.0f, 1.0f,   // bottom top right
			 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,   // bottom bottom right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,   // bottom bottom left
			-0.5f, -0.5f,  0.5f, 0.0f, 1.0f    // bottom top left 
		};
		unsigned int cube_indices[36] = {
			0, 1, 3,
			1, 2, 3,

			4, 5, 7,
			5, 6, 7,

			8, 9, 11,
			9, 10, 11,

			12, 13, 15,
			13, 14, 15,

			16, 17, 19,
			17, 18, 19,

			20, 21, 23,
			21, 22, 23
		};
	};
}