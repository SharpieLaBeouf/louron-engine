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

#include "../include/imgui/imgui.h"

#include "../Headers/Input.h"
#include "../Headers/Scene/InstanceManager.h"

class MainMenu : public State {

private:

	InstanceManager* m_InstanceManager;
	std::vector<std::unique_ptr<State>>* m_States;

public:
		
	MainMenu(InstanceManager* instanceManager) : m_InstanceManager(instanceManager) {
		std::cout << std::endl << "[L20] Opening Main Menu..." << std::endl;
		m_States = m_InstanceManager->getStatesInstance();
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
			
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowSize(ImVec2(400.0f, 200.0f), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2 - 200.0f, io.DisplaySize.y / 2 - 100.0f), ImGuiCond_Always);
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
			if (ImGui::Button(" 1. Basic Triangles    "))
				m_States->push_back(std::make_unique<Scene1>(m_InstanceManager));
			if (ImGui::Button(" 2. Basic Texture      ")) 
				m_States->push_back(std::make_unique<Scene2>(m_InstanceManager));
			if (ImGui::Button(" 3. Basic Cube         ")) 
				m_States->push_back(std::make_unique<Scene3>(m_InstanceManager));
			if (ImGui::Button(" 4. Basic Camera       "))
				m_States->push_back(std::make_unique<Scene4>(m_InstanceManager));
			if (ImGui::Button(" 5. Basic Lighting     "))
				m_States->push_back(std::make_unique<Scene5>(m_InstanceManager));
			if (ImGui::Button(" 6. Basic Material     "))
				m_States->push_back(std::make_unique<Scene6>(m_InstanceManager));
			if (ImGui::Button(" 7. Basic Model Loading"))
				m_States->push_back(std::make_unique<Scene7>(m_InstanceManager));
		}
		ImGui::End();
    }

};
