#pragma once

#include <GLFW/glfw3.h>
#include "../imgui/imgui.h"
#include <string>

const int MAX_KEYS = 1024;
const int MAX_BUTTONS = 32;

class InputManager {

public:
	
	InputManager() { }

	int init(GLFWwindow* window);
	
	bool GetKey(int glfwKeyCode);
	bool GetKeyDown(int glfwKeyCode);
	bool GetKeyUp(int glfwKeyCode);

	bool GetMouseButton(int glfwButtonCode);
	bool GetMouseButtonDown(int glfwButtonCode);
	bool GetMouseButtonUp(int glfwButtonCode);

private:

	bool mMouseHidden = true;

	static bool mKeys[MAX_KEYS][2];
	static bool mButtons[MAX_BUTTONS][2];

	friend static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	friend static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
};