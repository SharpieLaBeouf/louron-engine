#pragma once

#include <iostream>
#include <stack>

#include "../imgui/imgui.h"
#include "../Headers/SceneState.h"
#include "../Headers/Input.h"

namespace State {

	class Scene2 : public GameState {

	public:

		Scene2(std::stack<std::unique_ptr<State::GameState>>* gameStates) : m_States(gameStates) {
			std::cout << "[L20] Opening Scene 2..." << std::endl;
		}

		~Scene2() override
		{
			std::cout << "[L20] Closing Scene 2..." << std::endl;
		}

		void update(Window* wnd) override {

		}

		void draw() override {
			glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		}

	private:

		std::stack<std::unique_ptr<State::GameState>>* m_States;
		InputManager m_Input;
	};

}