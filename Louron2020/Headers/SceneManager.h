#pragma once

#include "../Headers/Window.h"
#include "../Headers/Input.h"

#include "../Headers/Abstracted GL/Shader.h"

namespace State {

	class SceneState {

	public:
		virtual void update() = 0;
		virtual void draw() = 0;

		virtual ~SceneState() = default;

	private:

	};

	class SceneManager {
	
	public:

		SceneManager(Window* wnd, InputManager* inp, ShaderLibrary* shaderLib, std::vector<std::unique_ptr<State::SceneState>>* states)
			: m_Window(wnd), m_Input(inp), m_ShaderLib(shaderLib), m_States(states) { }
		~SceneManager() = default;

		Window* getWindowInstance() { return m_Window; }
		InputManager* getInputInstance() { return m_Input; }
		ShaderLibrary* getShaderLibInstance() { return m_ShaderLib; }

		std::vector<std::unique_ptr<State::SceneState>>* getStatesInstance() { return m_States; }

	private:

		Window* m_Window;
		InputManager* m_Input;
		ShaderLibrary* m_ShaderLib;
		std::vector<std::unique_ptr<State::SceneState>>* m_States;
	};

}