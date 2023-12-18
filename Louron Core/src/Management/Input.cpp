#include "Input.h"

double InputManager::m_MouseX;
double InputManager::m_MouseY;

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
}

void cursorCallback(GLFWwindow* window, double xpos, double ypos)
{
	InputManager::m_MouseX = xpos;
	InputManager::m_MouseY = ypos;
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

	ImGuiIO& io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[key] = true;
	if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;
}

int InputManager::init(GLFWwindow* window)
{
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
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

