#include "Engine.h"

// Louron Core Headers
#include "Logging.h"
#include "Time.h"
#include "Physics.h"
#include "../Debug/Profiler.h"

#include "../OpenGL/Vertex Array.h"

// C++ Standard Library Headers
#include <filesystem>

// External Vendor Library Headers

namespace Louron {

    Engine* Engine::s_Instance = nullptr;

    Engine::Engine(const EngineSpecification& specification) : m_Specification(specification) {
        s_Instance = this;

        L_CORE_INFO("Initialising Louron Engine");

        if (!m_Specification.WorkingDirectory.empty())
            std::filesystem::current_path(m_Specification.WorkingDirectory);

        m_Window = Window::Create(WindowProps(m_Specification.Name));

        m_GuiLayer = new GuiLayer();
        PushOverlay(m_GuiLayer);

        // Init Time Manager
        Time::Init();

        // Init Physics Manager
        Physics::Init();

        // Init Audio System
        Audio::Get();

        m_ShaderLibrary = std::make_unique<ShaderLibrary>();

        // TODO: Move loading to Scene Management for Loading Scenes?

        // Load All Shaders 
        {
            for (const auto& path : FindFilePaths(".glsl")) {
                m_ShaderLibrary->LoadShader(path);
            }

            for (const auto& path : FindFilePaths(".comp")) {
                m_ShaderLibrary->LoadShader(path, true);
            }
        }

        m_TextureLibrary = std::make_unique<TextureLibrary>();

        // Load All Textures
        {
            for (const auto& path : FindFilePaths(".png")) {
                m_TextureLibrary->LoadTexture(path);
            }

            for (const auto& path : FindFilePaths(".jpg")) {
                m_TextureLibrary->LoadTexture(path);
            }

            for (const auto& path : FindFilePaths(".psd")) {
                m_TextureLibrary->LoadTexture(path);
            }
        }

        // Init Input Manager
        m_Input = std::make_unique<InputManager>();
        m_Input->Init((GLFWwindow*)m_Window->GetNativeWindow());
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
            L_PROFILE_SCOPE("Engine: Overall Loop");
            Time::Get().UpdateTime();

            {
                L_PROFILE_SCOPE("Engine: Standard Update Loop");

                for (Layer* layer : m_LayerStack) {
                    layer->OnUpdate();
                }
            }

            {
                L_PROFILE_SCOPE("Engine: Fixed Update Loop");

                m_FixedUpdateTimer += Time::GetDeltaTime();
                          
                while (m_FixedUpdateTimer >= Time::GetUnscaledFixedDeltaTime()) {

                    for (Layer* layer : m_LayerStack) {
                        layer->OnFixedUpdate();
                    }

                    m_FixedUpdateTimer -= Time::GetUnscaledFixedDeltaTime();
                }
            }

            m_GuiLayer->Begin();
            {
                L_PROFILE_SCOPE("Engine: GUI Update Loop");

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

    std::vector<std::string> Engine::FindFilePaths(const std::string& extension) {
        std::vector<std::string> paths;

        namespace fs = std::filesystem;

        try {
            fs::path currentPath = fs::current_path();

            for (const auto& entry : fs::recursive_directory_iterator(currentPath)) {
                if (entry.is_regular_file()) {
                    if (entry.path().extension() == extension) {
                        paths.push_back(entry.path().string());
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error searching for files: " << e.what() << std::endl;
        }

        return paths;
    }
}

