#include "../Headers/Window.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include "../Vendor/stb_image.h"

float Window::m_Width;
float Window::m_Height;

void windowSizeCallBack(GLFWwindow* window, int w, int h) {
	Window::m_Width = (w == 0) ? 1 : (float)w;
	Window::m_Height = (h == 0) ? 1 : (float)h;
	glViewport(0, 0, w, h);
}

Window::Window() : 
	m_Title("Game Window"),
	m_ScreenMode(0),
	m_Window(NULL),
	m_Input(nullptr)
{
	m_Width = 800.0f;
	m_Height = 600.0f;
}

Window::Window(int width, int height) : 
	
	m_Title("Game Window"),
	m_ScreenMode(0),
	m_Window(NULL),
	m_Input(nullptr)
{
	m_Width = (float)width;
	m_Height = (float)height;
}

Window::Window(const char* title, int width, int height) :
	m_Title(title),
	m_ScreenMode(0),
	m_Window(NULL),
	m_Input(nullptr)
{
	m_Width = (float)width;
	m_Height = (float)height;
}

/// <summary>
/// </summary>
/// <param name="screenMode"> Windowed = 0, Windowed Borderless Fullscreen = 1, Fullscreen = 2, Windowed Borderless = 3</param>
/// <returns></returns>
Window::Window(const char* title, int width, int height, int screenMode) :
	m_Title(title),
	m_ScreenMode(screenMode),
	m_Window(NULL),
	m_Input(nullptr)
{
	m_Width = (float)width;
	m_Height = (float)height;
}

Window::~Window()
{
	glfwDestroyWindow(m_Window);
}

int Window::init()
{
	
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	switch (m_ScreenMode)
	{
		case 0:
			m_Window = glfwCreateWindow((int)m_Width, (int)m_Height, m_Title, NULL, NULL);
		break;

		case 1:
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
			m_Window = glfwCreateWindow(mode->width, mode->height, m_Title, monitor, NULL);
		break;

		case 2:
			m_Window = glfwCreateWindow((int)m_Width, (int)m_Height, m_Title, monitor, NULL);
		break;

		case 3:
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
			m_Window = glfwCreateWindow((int)m_Width, (int)m_Height, m_Title, NULL, NULL);
		break;
	}

	glfwMakeContextCurrent(m_Window);
	glfwSetFramebufferSizeCallback(m_Window, windowSizeCallBack);

	if (m_Window == NULL)
		return -1;

	std::cout << "[L20] GLFW Created Window: " 
		<< m_Width << "x" << m_Height << " in " 
		<< ((m_ScreenMode == 0) ? "Windowed Mode!" :
		   ((m_ScreenMode == 1) ? "Windowed Borderless Mode!" : "Fullscreen Mode!")) <<  std::endl;
	
	m_Input = new InputManager();
	m_Input->init(m_Window);

	GLFWimage icon; int nrChannels;
	icon.pixels = stbi_load("Resources/Images/L20 Icon.png", &icon.width, &icon.height, &nrChannels, 0);
	glfwSetWindowIcon(m_Window, 1, &icon);
	stbi_image_free(icon.pixels);

	return 0;
}

void Window::toggleFullscreen() {
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	if (m_ScreenMode == 0){
		std::cout << "[L20] GLFW Changing to Fullscreen" << std::endl;
		glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
		m_ScreenMode = 2;
	}
	else if (m_ScreenMode == 2){
		std::cout << "[L20] GLFW Changing to Windowed" << std::endl;
		m_Width = 800.0f;
		m_Height = 800.0f;
		glfwSetWindowMonitor(m_Window, NULL, mode->width / 2 - (int)m_Width / 2, mode->height / 2 - (int)m_Height / 2, (int)m_Width, (int)m_Height, mode->refreshRate);
		m_ScreenMode = 0;
	}
}

GLFWwindow* Window::getWindow()
{
	return this->m_Window;
}

InputManager* Window::getInput()
{
	return m_Input;
}

float Window::getWidth()
{
	return m_Width;
}

float Window::getHeight()
{
	return m_Height;
}

void Window::setWidth(float width)
{
	m_Width = width;
}

void Window::setHeight(float height)
{
	m_Height = height;
}
