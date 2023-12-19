#pragma once

#include <iostream>
#include <stack>

#include "Scene 1.h"
#include "Scene 2.h"
#include "Scene 3.h"
#include "Scene 4.h"
#include "Scene 5.h"
#include "Scene 6.h"
#include "Scene 7.h"
#include "Scene 8.h"
#include "Scene 9.h"

#include <imgui/imgui.h>

#include "Engine Manager.h"

class State {

public:
	virtual void update() = 0;
	virtual void draw() = 0;

	virtual ~State() = default;

private:

};

class MainMenu : public State {

private:

	LouronEngine* m_Engine;
	std::vector<std::unique_ptr<State>>* m_States;

public:
		
	MainMenu(LouronEngine* engine, std::vector<std::unique_ptr<State>>* states) : m_Engine(engine) {
		std::cout << std::endl << "[L20] Opening Main Menu..." << std::endl;
		m_States = states;
	}
	~MainMenu() override {
		std::cout << "[L20] Closing Main Menu!" << std::endl;
	}
		
	void update() override {

	}

	void draw() override {
		// Set Background Colour (211, 238, 255, 1)
		glClearColor(211.0f / 255.0f, 238.0f / 255.0f, 255.0f / 255.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT);
			

		glm::vec2 menuSize = { 400.0f, 400.0f };
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowSize(ImVec2(menuSize.x, menuSize.y), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2 - menuSize.x / 2, io.DisplaySize.y / 2 - menuSize.y / 2), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(1.0f);
		if (ImGui::Begin("Main Menu", (bool *)0, 
			ImGuiWindowFlags_NoDecoration | 
			ImGuiWindowFlags_AlwaysAutoResize | 
			ImGuiWindowFlags_NoSavedSettings | 
			ImGuiWindowFlags_NoNav | 
			ImGuiWindowFlags_NoMove))
		{
				
			ImGui::Text("Main Menu");
			ImGui::Separator();
			if (ImGui::Button(" 1. Basic Triangles        ")) {
				//m_States->push_back(std::make_unique<Scene1>(m_Engine));
			}
			if (ImGui::Button(" 2. Basic Texture          ")) {
				//m_States->push_back(std::make_unique<Scene2>(m_Engine));
			}
			if (ImGui::Button(" 3. Basic Cube             ")) {
				//m_States->push_back(std::make_unique<Scene3>(m_Engine));
			}
			if (ImGui::Button(" 4. Basic Camera           ")) {
				//m_States->push_back(std::make_unique<Scene4>(m_Engine));
			}
			if (ImGui::Button(" 5. Basic Lighting         ")) {
				//m_States->push_back(std::make_unique<Scene5>(m_Engine));
			}
			if (ImGui::Button(" 6. Basic Material         ")) {
				//m_States->push_back(std::make_unique<Scene6>(m_Engine));
			}
			if (ImGui::Button(" 7. Basic Model Loading    ")) {
				//m_States->push_back(std::make_unique<Scene7>(m_Engine));
			}
			if (ImGui::Button(" 8. Entity System          ")) {
				//m_States->push_back(std::make_unique<Scene8>(m_Engine));
			}
			if (ImGui::Button(" 9. First Rudimentary Game ")) {
				//m_States->push_back(std::make_unique<Scene9>(m_Engine));
			}
		}
		ImGui::End();
    }

};
