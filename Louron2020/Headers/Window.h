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

	float getWidth();
	float getHeight();

	void setWidth (float width);
	void setHeight(float height);

private:
	int m_ScreenMode;
	bool m_ConsoleToggled;
	float m_Width, m_Height;
	const char* m_Title;
	GLFWwindow* m_Window;
	InputManager* m_Input;

	static void windowSizeCallBack(GLFWwindow* window, int w, int h)
	{
		glViewport(0, 0, w, h);
	}
};