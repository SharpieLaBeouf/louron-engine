#pragma once

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

	GLFWwindow* getWindow();
	InputManager* getInput();

	float getWidth();
	float getHeight();

	void setWidth (float width);
	void setHeight(float height);

private:
	int m_ScreenMode;
	const char* m_Title;
	GLFWwindow* m_Window;
	InputManager* m_Input;
	static float m_Width, m_Height;

	friend static void windowSizeCallBack(GLFWwindow* window, int w, int h);
};