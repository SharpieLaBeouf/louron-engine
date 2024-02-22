#include "Engine.h"

// Louron Core Headers

// C++ Standard Library Headers
#include <filesystem>

// External Vendor Library Headers

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
   
        // TODO: Move loading to Scene Management for Loading Scenes?

        // Load All Shaders 
        {
            for (const auto& path : FindFilePaths("assets", ".glsl")) {
                m_ShaderLibrary->LoadShader(path);
            }

            for (const auto& path : FindFilePaths("assets", ".comp")) {
                m_ShaderLibrary->LoadShader(path, true);
            }
        }

        m_TextureLibrary = std::make_unique<TextureLibrary>();

        // Load All Textures
        {
            for (const auto& path : FindFilePaths("assets", ".png")) {
                m_TextureLibrary->LoadTexture(path);
            }

            for (const auto& path : FindFilePaths("assets", ".jpg")) {
                m_TextureLibrary->LoadTexture(path);
            }
        }

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

    std::vector<std::string> Engine::FindFilePaths(const std::string& directory, const std::string& extension)
    {
        std::vector<std::string> foundPaths;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (std::filesystem::is_regular_file(entry) && entry.path().extension() == extension) {
                    foundPaths.push_back(entry.path().string());
                }
                else if (std::filesystem::is_directory(entry)) {
                   auto subDirectoryPaths = FindFilePaths(entry.path().string(), extension);
                   foundPaths.insert(foundPaths.end(), subDirectoryPaths.begin(), subDirectoryPaths.end());
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[L20] Error: " << e.what() << std::endl;
        }
       
        return foundPaths;
    }
}

