#pragma once

// WINDOWS LIBRARIES
#if defined _WIN64 || defined _WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <GLFW/glfw3.h>

#include "../Headers/Input.h"

class Window {

public:
	Window();
	Window(int width, int height);
	Window(const char* title, int width, int height);
	Window(const char* title, int width, int height, int screenMode);
	~Window();

	int init();

	void toggleFullscreen();
	void toggleConsole();

	GLFWwindow* getWindow();
	InputManager* getInput();

	int getWidth();
	int getHeight();

private:
	int m_ScreenMode;
	bool m_ConsoleToggled;
	int m_Width, m_Height;
	const char* m_Title;
	GLFWwindow* m_Window;
	InputManager* m_Input;
};