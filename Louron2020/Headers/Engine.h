#pragma once

// WINDOWS LIBRARIES
#if defined _WIN64 || defined _WIN32
#include <Windows.h>
#endif

// C++ STD LIBRARIES
#include <iostream>
#include <conio.h>
#include <stack>
#include <memory>

// OPENGL LIBRARIES
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
 
// CUSTOM HEADERS
#include "Window.h"
#include "Input.h"
#include "../Scenes/MainMenu.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class Engine {


public:
	Engine();
	~Engine();

	int run();
	
	Window* getWindow();

private:

	Window* m_Window;
	InputManager* m_Input;

	bool m_fpsToggled = true;

	std::stack<std::unique_ptr<State::GameState>> m_States;
};