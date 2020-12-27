#pragma once

#include <iostream>
#include <stack>

#include "../Headers/imgui/imgui.h"
#include "../Headers/SceneState.h"
#include "../Headers/Input.h"

namespace State {

	class Scene2 : public SceneState {

	public:

		Scene2(std::stack<std::unique_ptr<State::SceneState>>* SceneStates) : m_States(SceneStates) {
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

		std::stack<std::unique_ptr<State::SceneState>>* m_States;
		InputManager m_Input;
	};

}