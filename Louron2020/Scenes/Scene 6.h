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

namespace State {
	class Scene6 : public SceneState {

		//Private Setup Variables
	private:

		State::InstanceManager* m_InstanceManager;

		Window* m_Window;
		InputManager* m_Input;
		ShaderLibrary* m_ShaderLib;
		TextureLibrary* m_TextureLib;

		Material* flat_cube_mat = nullptr;
		Material* phong_cube_mat = nullptr;
		Light light_properties;

		Camera* scnCamera;

	public:

		explicit Scene6(InstanceManager* instanceManager)
			: m_InstanceManager(instanceManager)
		{
			std::cout << "[L20] Opening Scene 6..." << std::endl;
			m_Window = m_InstanceManager->getWindowInstance();
			m_Input = m_InstanceManager->getInputInstance();
			m_ShaderLib = m_InstanceManager->getShaderLibInstance();
			m_TextureLib = m_InstanceManager->getTextureLibInstance();

			{
				glEnable(GL_DEPTH_TEST);
				glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

				// Init Cube VAO
				glGenVertexArrays(1, &cube_VAO);
				glGenBuffers(1, &cube_VBO);
				glGenBuffers(1, &cube_EBO);

				glBindVertexArray(cube_VAO);

				glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)0);
				glEnableVertexAttribArray(0);

				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
				glEnableVertexAttribArray(1);

				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
				glEnableVertexAttribArray(2);


				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}

			scnCamera = new Camera(m_Window, glm::vec3(0.0f, 0.0f, 10.0f));

			flat_cube_mat = new Material(m_ShaderLib->getShader("material_shader_flat"), m_TextureLib->getTexture("blank_texture"));
			flat_cube_mat->Bind();
			flat_cube_mat->setDiffuse(glm::vec4(1.0f));
			flat_cube_mat->AddTextureMap(TextureMapType::L20_TEXTURE_DIFFUSE_MAP, m_TextureLib->getTexture("cube_texture"));
		
			phong_cube_mat = new Material(m_ShaderLib->getShader("material_shader_phong"), m_TextureLib->getTexture("blank_texture"));
			phong_cube_mat->Bind();
			phong_cube_mat->setDiffuse(glm::vec4(1.0f));
			phong_cube_mat->AddTextureMap(TextureMapType::L20_TEXTURE_DIFFUSE_MAP, m_TextureLib->getTexture("cube_texture"));
			phong_cube_mat->AddTextureMap(TextureMapType::L20_TEXTURE_SPECULAR_MAP, m_TextureLib->getTexture("cube_texture_specular"));
		}

		~Scene6() override
		{
			std::cout << "[L20] Closing Scene 6..." << std::endl;
			glDisable(GL_DEPTH_TEST);
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			glDeleteVertexArrays(1, &cube_VAO);
			glDeleteBuffers(1, &cube_VBO);
			glDeleteBuffers(1, &cube_EBO);

			delete flat_cube_mat;
			delete phong_cube_mat;
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

		}

		void draw() override {

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

			if (flat_cube_mat->Bind())
			{
				glBindVertexArray(cube_VAO);

				flat_cube_mat->setUniforms();
				flat_cube_mat->getShader()->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)));
				flat_cube_mat->getShader()->setMat4("proj", glm::perspective(glm::radians(60.0f), m_Window->getWidth() / m_Window->getHeight(), 0.1f, 100.0f));
				flat_cube_mat->getShader()->setMat4("view", scnCamera->getViewMatrix());
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

				flat_cube_mat->getShader()->setMat4("model", glm::translate(glm::mat4(1.0f), light_properties.position));
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

				flat_cube_mat->UnBind();
			}

			if (phong_cube_mat->Bind()) {
				phong_cube_mat->setUniforms();
				phong_cube_mat->getShader()->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)));
				phong_cube_mat->getShader()->setMat4("proj", glm::perspective(glm::radians(60.0f), m_Window->getWidth() / m_Window->getHeight(), 0.1f, 100.0f));
				phong_cube_mat->getShader()->setMat4("view", scnCamera->getViewMatrix());
				phong_cube_mat->getShader()->setVec3("u_Light.position", light_properties.position);
				phong_cube_mat->getShader()->setVec4("u_Light.ambient", light_properties.ambient);
				phong_cube_mat->getShader()->setVec4("u_Light.diffuse", light_properties.diffuse);
				phong_cube_mat->getShader()->setVec4("u_Light.specular", light_properties.specular);
				phong_cube_mat->getShader()->setVec3("u_CameraPos", scnCamera->getPosition());

				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
				phong_cube_mat->UnBind();
			}

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

			if (ImGui::TreeNode("Flat Cube Material"))
			{
				ImGui::ColorEdit4("Ambient", glm::value_ptr(*flat_cube_mat->getAmbient()));
				ImGui::ColorEdit4("Diffuse", glm::value_ptr(*flat_cube_mat->getDiffuse()));
				ImGui::ColorEdit4("Specular", glm::value_ptr(*flat_cube_mat->getSpecular()));
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Phong Cube Material"))
			{
				ImGui::ColorEdit4("Ambient", glm::value_ptr(*phong_cube_mat->getAmbient()));
				ImGui::ColorEdit4("Diffuse", glm::value_ptr(*phong_cube_mat->getDiffuse()));
				ImGui::ColorEdit4("Specular", glm::value_ptr(*phong_cube_mat->getSpecular()));
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Light Properties"))
			{
				ImGui::DragFloat3("Position", glm::value_ptr(light_properties.position), 0.1f);
				ImGui::ColorEdit4("Ambient", glm::value_ptr(light_properties.ambient));
				ImGui::ColorEdit4("Diffuse", glm::value_ptr(light_properties.diffuse));
				ImGui::ColorEdit4("Specular", glm::value_ptr(light_properties.specular));
				ImGui::TreePop();
			}
			
			if (ImGui::TreeNode("Colours"))
			{
				ImGui::ColorPicker4("Background", glm::value_ptr(back_colour));
				ImGui::TreePop();
			}

			glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);

			ImGui::End();

		}

		glm::vec4 back_colour = { 0.3137f, 0.7843f, 1.0f, 1.0f };
		glm::vec4 cube_colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		float currentTime = 0;
		float deltaTime = 0;
		float lastTime = 0;

		int selection = 0;

		GLuint cube_VAO;
		GLuint cube_VBO;
		GLuint cube_EBO;
		float cube_vertices[192] = {
			// POSITION          // NORMAL            // TEXCORD
			0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,  // front top right
			0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,  // front bottom right
		   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  // front bottom left
		   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,  // front top left

			0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,  // back top right
			0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,  // back bottom right
		   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,  // back bottom left
		   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,  // back top left

			0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // right top right
			0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // right bottom right
			0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // right bottom left
			0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // right top left 

		   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // left top right
		   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // left bottom right
		   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // left bottom left
		   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // left top left 

			0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,  // top top right
			0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  // top bottom right
		   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,  // top bottom left
		   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  // top top left 

			0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  // bottom top right
			0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  // bottom bottom right
		   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  // bottom bottom left
		   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f   // bottom top left 
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