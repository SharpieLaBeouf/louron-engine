#include "Input.h"

#include "imgui/imgui_impl_glfw.cpp"

namespace Louron {

	double InputManager::m_MouseX;
	double InputManager::m_MouseY;

	double InputManager::m_ScrollX;
	double InputManager::m_ScrollY;

	bool InputManager::mKeys[MAX_KEYS][2];
	bool InputManager::mButtons[MAX_BUTTONS][2];

	void mouseCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			InputManager::mButtons[button][0] = true;
			InputManager::mButtons[button][1] = true;
		}

		if (action == GLFW_RELEASE)
		{
			InputManager::mButtons[button][0] = false;
			InputManager::mButtons[button][1] = true;
		}

        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	}

	void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		InputManager::m_ScrollX += xoffset; // Accumulate scroll offsets
		InputManager::m_ScrollY += yoffset;

		ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	}

	void cursorCallback(GLFWwindow* window, double xpos, double ypos)
	{
		InputManager::m_MouseX = xpos;
		InputManager::m_MouseY = ypos;

        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
	}

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			InputManager::mKeys[key][0] = true;
			InputManager::mKeys[key][1] = true;
		}

		if (action == GLFW_RELEASE)
		{
			InputManager::mKeys[key][0] = false;
			InputManager::mKeys[key][1] = true;
		}

        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	}

	int InputManager::Init(GLFWwindow* window)
	{
		glfwSetKeyCallback(window, keyCallback);
		glfwSetMouseButtonCallback(window, mouseCallback);
		glfwSetScrollCallback(window, scrollCallback);
		glfwSetCursorPosCallback(window, cursorCallback);
		glfwSetWindowUserPointer(window, this);

		for (int i = 0; i < MAX_KEYS; i++)
			for (int x = 0; x < 2; x++)
				this->mKeys[i][x] = false;

		for (int i = 0; i < MAX_BUTTONS; i++)
			for (int x = 0; x < 2; x++)
				this->mButtons[i][x] = false;
	
		return 0;
	}

	bool InputManager::GetKey(int glfwKeyCode)
	{
		return mKeys[glfwKeyCode][0];
	}

	bool InputManager::GetKeyDown(int glfwKeyCode)
	{
		if (mKeys[glfwKeyCode][0] == true && mKeys[glfwKeyCode][1] == true)
		{
			mKeys[glfwKeyCode][1] = false;
			return true;
		}
		return false;
	}

	bool InputManager::GetKeyUp(int glfwKeyCode)
	{
		if (mKeys[glfwKeyCode][0] == false && mKeys[glfwKeyCode][1] == true)
		{
			mKeys[glfwKeyCode][1] = false;
			return true;
		}
		return false;
	}

	bool InputManager::GetMouseButton(int glfwButtonCode)
	{
		return mButtons[glfwButtonCode][0];
	}

	bool InputManager::GetMouseButtonDown(int glfwButtonCode)
	{
		if (mButtons[glfwButtonCode][0] == true && mButtons[glfwButtonCode][1] == true)
		{
			mButtons[glfwButtonCode][1] = false;
			return true;
		}
		return false;
	}

	bool InputManager::GetMouseButtonUp(int glfwButtonCode)
	{
		if (mButtons[glfwButtonCode][0] == false && mButtons[glfwButtonCode][1] == true)
		{
			mButtons[glfwButtonCode][1] = false;
			return true;
		}
		return false;
	}

	float InputManager::GetMouseX()
	{
		return (float)m_MouseX;
	}

	float InputManager::GetMouseY()
	{
		return (float)m_MouseY;
	}

	void InputManager::ResetScroll()
	{
		m_ScrollX = 0.0;
		m_ScrollY = 0.0;
	}

	float InputManager::GetScrollX()
	{
		return (float)m_ScrollX;
	}

	float InputManager::GetScrollY()
	{
		return (float)m_ScrollY;
	}

}

