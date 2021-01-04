#pragma once

#include <iostream>
#include <stack>

#include "Scene 1.h"
#include "Scene 2.h"
#include "Scene 3.h"
#include "Scene 4.h"

#include "../Headers/imgui/imgui.h"
#include "../Headers/SceneState.h"
#include "../Headers/Input.h"

namespace State {

	class MainMenu : public SceneState {

	public:
		
		MainMenu(std::stack<std::unique_ptr<State::SceneState>>* SceneStates, Window* wnd) : m_States(SceneStates) {
			std::cout << std::endl << "[L20] Linked Game States -> Opening Main Menu..." << std::endl;
			m_Window = wnd;
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
				if (ImGui::Button(" 1. Basic Triangles "))
					m_States->push(std::make_unique<State::Scene1>(m_States, m_Window));
				if (ImGui::Button(" 2. Basic Texture   ")) 
					m_States->push(std::make_unique<State::Scene2>(m_States, m_Window));
				if (ImGui::Button(" 3. Basic Cube      ")) 
					m_States->push(std::make_unique<State::Scene3>(m_States, m_Window));
				if (ImGui::Button(" 3. Basic Camera    "))
					m_States->push(std::make_unique<State::Scene4>(m_States, m_Window));
			}
			ImGui::End();
        }

	private:

		std::stack<std::unique_ptr<State::SceneState>>* m_States;
		Window* m_Window;
	};

}