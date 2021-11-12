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
#include "../include/imgui/imgui.h"
#include "../include/imgui/imgui_impl_glfw.h"
#include "../include/imgui/imgui_impl_opengl3.h"
 
// CUSTOM HEADERS
#include "Window.h"
#include "Input.h"

#include "Scene/InstanceManager.h"
#include "../Scenes/MainMenu.h"

class Engine {


public:
	Engine();
	~Engine();

	int run();
	
	Window* getWindow();

private:

	InstanceManager* m_InstanceManager = nullptr;

	Window* m_Window = nullptr;
	InputManager* m_Input = nullptr;
	ShaderLibrary* m_ShaderLib = nullptr;
	TextureLibrary* m_TextureLib = nullptr;
	std::vector<std::unique_ptr<State>> m_States;

};