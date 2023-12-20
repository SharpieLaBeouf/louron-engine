#include "Engine.h"

#include <filesystem>

namespace Louron {

    Engine* Engine::s_Instance = nullptr;

    Engine::Engine(const EngineSpecification& specification) {
        s_Instance = this;

        if (!m_Specification.WorkingDirectory.empty())
            std::filesystem::current_path(m_Specification.WorkingDirectory);

        m_GuiLayer = new GuiLayer();
        PushOverlay(m_GuiLayer);
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
    }

    bool Engine::OnWindowClose() {
        return false;
    }

    bool Engine::OnWindowResize() {
        return false;
    }
}

