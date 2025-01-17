#pragma once

// Louron Core Headers
#include "../Core/KeyCodes.h"

// C++ Standard Library Headers
#include <string>

// External Vendor Library Headers
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <imgui/imgui.h>

const inline int MAX_KEYS = 1024;
const inline int MAX_BUTTONS = 32;

namespace Louron {

	class InputManager {

	public:

		InputManager() = default;

		int Init(GLFWwindow* window);

		bool GetKey(int glfwKeyCode);
		bool GetKeyDown(int glfwKeyCode);
		bool GetKeyUp(int glfwKeyCode);

		bool GetMouseButton(int glfwButtonCode);
		bool GetMouseButtonDown(int glfwButtonCode);
		bool GetMouseButtonUp(int glfwButtonCode);

		float GetMouseX();
		float GetMouseY();

		glm::vec2 GetMousePosition() { return glm::vec2((float)m_MouseX, (float)m_MouseY); }

	private:

		bool mMouseHidden = true;

		static bool mKeys[MAX_KEYS][2];
		static bool mButtons[MAX_BUTTONS][2];
		static double m_MouseX, m_MouseY;

		friend static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		friend static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
		friend static void cursorCallback(GLFWwindow* window, double xpos, double ypos);
	};
}