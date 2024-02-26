#include "Window.h"

// Louron Core Headers
#include "Engine.h"
#include "Logging.h"
#include "../Debug/Assert.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include <stb_image/stb_image.h>

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
		if (successGLFW)
			L_CORE_INFO("GLFW Initialised Successfully");
		else {
			L_CORE_FATAL("GLFW Not Initialised Successfully");
			Engine::Get().Close();
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data->Title.c_str(), nullptr, nullptr);

		glfwMakeContextCurrent(m_Window);

		// Initialise OpenGL (Graphics Context)
		int successOPENGL = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		if (successOPENGL) {
			L_CORE_INFO("OpenGL Initialised Successfully");
			L_CORE_INFO("OpenGL Version: {0}", (const char*)glGetString(GL_VERSION));
		}
		else {
			L_CORE_FATAL("OpenGL Not Initialised Successfully");
			Engine::Get().Close();
		}

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

		L_CORE_INFO("GLFW Window Created: {0} x {1}", m_Data->Width, m_Data->Height);
	}

	void Window::Shutdown()	{

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

}