#pragma once

// STD LIBRARIES
#include <iostream>
#include <conio.h>
#include <stack>
#include <memory>

// GL LIBRARIES
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

// VENDER LIBRARIES
#include "../Vendor/imgui/imgui.h"
#include "../Vendor/imgui/imgui_impl_glfw.h"
#include "../Vendor/imgui/imgui_impl_opengl3.h"
 
// CUSTOM HEADERS
#include "Window.h"
#include "Input.h"

#include "SceneManager.h"
#include "../Scenes/MainMenu.h"

class Engine {


public:
	Engine();
	~Engine();

	int run();
	
	Window* getWindow();

private:

	State::SceneManager* m_SceneManager = nullptr;

	Window* m_Window = nullptr;
	InputManager* m_Input = nullptr;
	ShaderLibrary* m_ShaderLib = nullptr;
	std::vector<std::unique_ptr<State::SceneState>> m_States;

	bool m_fpsToggled = true;

};