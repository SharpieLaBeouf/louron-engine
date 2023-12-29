#include "Window.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include <stb_image/stb_image.h>

#include "Engine.h"

namespace Louron {
	
	Window::Window(const WindowProps& props)
	{
		m_Data = std::make_unique<WindowData>();

		Init(props);
	}

	Window::~Window() {
		Shutdown();
	}

	void Window::OnUpdate()	{
		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

	void Window::SetVSync(bool enabled)	{
		enabled ? glfwSwapInterval(1) : glfwSwapInterval(0);
		m_Data->VSync = enabled;
	}

	std::unique_ptr<Window> Window::Create(const WindowProps& props)
	{
		return std::make_unique<Window>(props);
	}

	void Window::Init(const WindowProps& props)
	{

		m_Data->Title = props.Title;
		m_Data->Width = props.Width;
		m_Data->Height = props.Height;

		// Initialise GLFW
		int successGLFW = glfwInit();
		std::cout << (successGLFW ? "[L20] GLFW Initialised Successfully" : "[L20] GLFW Could Not Initialise!") << std::endl << std::endl;
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data->Title.c_str(), nullptr, nullptr);

		glfwMakeContextCurrent(m_Window);

		// Initialise OpenGL (Graphics Context)
		int successOPENGL = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		std::cout << (successOPENGL ? "[L20] OpenGL Initialised Successfully" : "[L20] OpenGL Could Not Initialise!") << std::endl;
		std::cout << "[L20] OpenGL Version: " << (const char*)glGetString(GL_VERSION) << std::endl << std::endl;
		glViewport(0, 0, (GLsizei)m_Data->Width, (GLsizei)m_Data->Height);
		
		glfwSetWindowUserPointer(m_Window, &m_Data);

		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(m_Window);

		glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int w, int h) 
			{
				WindowData& data = Engine::Get().GetWindow().GetData();
				data.Width = w;
				data.Height = h;
				glViewport(0, 0, data.Width, data.Height);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				Engine::Get().Close();
			});

		std::cout << "[L20] GLFW Created Window: " << m_Data->Width << "x" << m_Data->Height << std::endl;
	}

	void Window::Shutdown()	{

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

}