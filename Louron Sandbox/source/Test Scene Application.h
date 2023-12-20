#pragma once

// STD LIBRARIES
#include <iostream>
#include <conio.h>
#include <stack>
#include <memory>

// GL LIBRARIES
#include <glad/glad.h>
#include <glfw/glfw3.h>

// VENDER LIBRARIES
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

// CUSTOM HEADERS
#include "Core/Window.h"
#include "Core/Input.h"
#include "../Tests/MainMenu.h"

class TestSceneApplication {


public:
	TestSceneApplication();
	~TestSceneApplication();

	int run();
	int init();

	Window* getWindow();

private:

	InstanceManager* m_InstanceManager = nullptr;

	Window* m_Window = nullptr;
	InputManager* m_Input = nullptr;
	ShaderLibrary* m_ShaderLib = nullptr;
	TextureLibrary* m_TextureLib = nullptr;
	std::vector<std::unique_ptr<State>> m_States;

};