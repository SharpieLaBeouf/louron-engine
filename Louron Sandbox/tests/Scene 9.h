#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Core/InstanceManager.h"
#include "Scene/SceneManager.h"

class Scene9 : public State {

	//Private Setup Variables
private:

	InstanceManager* m_InstanceManager;

	Window* m_Window;
	InputManager* m_Input;
	ShaderLibrary* m_ShaderLib;
	TextureLibrary* m_TextureLib;

	Material* phong_cube_mat = nullptr;
	Light light_properties;

	Camera* scnCamera;

public:

	explicit Scene9(InstanceManager* instanceManager) : m_InstanceManager(instanceManager) {
		std::cout << "[L20] Opening Scene 9..." << std::endl;
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
		
		scnCamera = new Camera(m_Window, glm::vec3(0.0f, 10.0f, 0.0f));
		scnCamera->setPitch(-90.0f);
		scnCamera->setYaw(-180.0f);
		scnCamera->toggleMovement();
		light_properties.position = scnCamera->getPosition();
			
		phong_cube_mat = new Material(m_ShaderLib->getShader("material_shader_phong"), m_TextureLib->getTexture("blank_texture"));
		phong_cube_mat->Bind();
		phong_cube_mat->setDiffuse(glm::vec4(1.0f));
		phong_cube_mat->AddTextureMap(TextureMapType::L20_TEXTURE_DIFFUSE_MAP, m_TextureLib->getTexture("cube_texture"));
		phong_cube_mat->AddTextureMap(TextureMapType::L20_TEXTURE_SPECULAR_MAP, m_TextureLib->getTexture("cube_texture_specular"));
		phong_cube_mat->UnBind();
	}

	~Scene9() override {
		std::cout << "[L20] Closing Scene 9..." << std::endl;
		glDisable(GL_DEPTH_TEST);
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		glDeleteVertexArrays(1, &cube_VAO);
		glDeleteBuffers(1, &cube_VBO);
		glDeleteBuffers(1, &cube_EBO);

		delete phong_cube_mat;
		delete scnCamera;
	}

	void update() override {
		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		scnCamera->Update(deltaTime);

		if (!gameover)
		{
			if (m_Input->GetKey(GLFW_KEY_W))
				cube1_position.x += deltaTime * 2.0f;
			if (m_Input->GetKey(GLFW_KEY_S))
				cube1_position.x -= deltaTime;
			if (m_Input->GetKey(GLFW_KEY_D))
				cube1_position.z += deltaTime;
			if (m_Input->GetKey(GLFW_KEY_A))
				cube1_position.z -= deltaTime;

			if (m_Input->GetKey(GLFW_KEY_UP))
				cube2_position.x += deltaTime * 2.0f;
			if (m_Input->GetKey(GLFW_KEY_DOWN))
				cube2_position.x -= deltaTime;
			if (m_Input->GetKey(GLFW_KEY_RIGHT))
				cube2_position.z += deltaTime;
			if (m_Input->GetKey(GLFW_KEY_LEFT))
				cube2_position.z -= deltaTime;
		}

		if (cube1_position.x > 5.5f)
			gameOverGUI(1);
		if (cube2_position.x > 5.5f)
			gameOverGUI(2);
	}

	void draw() override {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		if (phong_cube_mat->Bind()) {
			glBindVertexArray(cube_VAO);

			phong_cube_mat->setUniforms();
			phong_cube_mat->getShader()->setMat4("model", glm::translate(glm::mat4(1.0f), cube1_position));
			phong_cube_mat->getShader()->setMat4("proj", glm::perspective(glm::radians(60.0f), m_Window->getWidth() / m_Window->getHeight(), 0.1f, 100.0f));
			phong_cube_mat->getShader()->setMat4("view", scnCamera->getViewMatrix());
			phong_cube_mat->getShader()->setVec3("u_Light.position", light_properties.position);
			phong_cube_mat->getShader()->setVec4("u_Light.ambient", light_properties.ambient);
			phong_cube_mat->getShader()->setVec4("u_Light.diffuse", light_properties.diffuse);
			phong_cube_mat->getShader()->setVec4("u_Light.specular", light_properties.specular);
			phong_cube_mat->getShader()->setVec3("u_CameraPos", scnCamera->getPosition());

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

			phong_cube_mat->getShader()->setMat4("model", glm::translate(glm::mat4(1.0f), cube2_position));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

			phong_cube_mat->UnBind();
		}

		processGUI();
	}

private:
	void TextCentered(std::string text) {
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text(text.c_str());
	}

	void gameOverGUI(int winner) {
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		gameover = true;
		glm::vec2 menuSize = { 400.0f, 400.0f };
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowSize(ImVec2(menuSize.x, menuSize.y), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2 - menuSize.x / 2, io.DisplaySize.y / 2 - menuSize.y / 2), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(1.0f);

		ImGui::Begin("GAMEOVER!", (bool*)0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);
		
		TextCentered(std::string("GAME OVER!"));
		TextCentered(std::string("The Winner is Player " + std::to_string(winner) + "!!"));
		if (ImGui::Button("Try Again?")) {
			cube1_position = { -5.0f, 0.0f, -2.5f };
			cube2_position = { -5.0f, 0.0f, 2.5f };
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			gameover = false;
		}
		
		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]); 
		
		ImGui::End();

	}

	void processGUI() {

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

	glm::vec3 cube1_position = { -5.0f, 0.0f, -2.5f };
	glm::vec3 cube2_position = { -5.0f, 0.0f, 2.5f };

	bool gameover = false;

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