#include "Engine.h"

#include <filesystem>

namespace Louron {

    Engine* Engine::s_Instance = nullptr;

    Engine::Engine(const EngineSpecification& specification) {
        s_Instance = this;

        // Display Louron Banner
        std::cout << "<---------------------------------------------->" << std::endl;
        std::cout << "<                                              >" << std::endl;
        std::cout << "<                 Louron Engine                >" << std::endl;
        std::cout << "<                  Aaron Sharp                 >" << std::endl;
        std::cout << "<                                              >" << std::endl;
        std::cout << "<---------------------------------------------->" << std::endl << std::endl;

        if (!m_Specification.WorkingDirectory.empty())
            std::filesystem::current_path(m_Specification.WorkingDirectory);

        m_Window = Window::Create(WindowProps(m_Specification.Name));
        m_GuiLayer = new GuiLayer();
        PushOverlay(m_GuiLayer);

        m_ShaderLibrary = std::make_unique<ShaderLibrary>();
        m_TextureLibrary = std::make_unique<TextureLibrary>();

        m_Input = std::make_unique<InputManager>();
        m_Input->Init((GLFWwindow*)m_Window->GetNativeWindow());
    }

    Engine::~Engine() {

    }

    void Engine::PushLayer(Layer* layer) {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Engine::PushOverlay(Layer* layer) {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    void Engine::Close() {
        m_Running = false;
    }

    void Engine::Run() {

        while (m_Running) {

            {
                for (Layer* layer : m_LayerStack) {
                    layer->OnUpdate();
                }
            }

            m_GuiLayer->Begin();
            {
                for (Layer* layer : m_LayerStack) {
                    layer->OnGuiRender();
                }
            }
            m_GuiLayer->End();

            m_Window->OnUpdate();
        }
    }

    bool Engine::OnWindowClose() {
        m_Running = false;
        return true;
    }

    bool Engine::OnWindowResize() {
        return false;
    }
}

