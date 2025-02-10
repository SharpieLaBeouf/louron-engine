#include "Engine.h"

// Louron Core Headers
#include "Logging.h"
#include "Time.h"
#include "Physics.h"
#include "../Debug/Profiler.h"

#include "../OpenGL/Vertex Array.h"

#include "../Renderer/Renderer.h"

#include "../Scripting/Script Manager.h"

#include "../Project/Project.h"

// C++ Standard Library Headers
#include <filesystem>

// External Vendor Library Headers

namespace Louron {

    Engine* Engine::s_Instance = nullptr;

    Engine::Engine(const EngineConfig& specification) : m_Specification(specification) {
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
        Audio::Init();

        // Renderer Init Debug VAOs
        Renderer::Init();

        // TODO: Do we even use this anymore?
        m_TextureLibrary = std::make_unique<TextureLibrary>();

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

    void Engine::SubmitToMainThread(const std::function<void()>& function)
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        m_MainThreadQueue.emplace_back(function);
    }

    void Engine::Run() {

        while (m_Running) {
            L_PROFILE_SCOPE("Engine: Overall Loop");

            Profiler::Get().NewFrame();

            Time::Get().UpdateTime();

            {
                L_PROFILE_SCOPE("Engine: 1. Execute Main Thread");
                ExecuteMainThreadQueue();
            }

            {
                L_PROFILE_SCOPE("Engine: 2. Standard Update Loop");

                for (Layer* layer : m_LayerStack) {
                    layer->OnUpdate();
                }
            }

            {
                L_PROFILE_SCOPE("Engine: 3. Fixed Update Loop");

                m_FixedUpdateTimer += Time::GetDeltaTime();
                          
                while (m_FixedUpdateTimer >= Time::GetUnscaledFixedDeltaTime()) {

                    for (Layer* layer : m_LayerStack) {
                        layer->OnFixedUpdate();
                    }

                    m_FixedUpdateTimer -= Time::GetUnscaledFixedDeltaTime();
                }
            }

            {
                L_PROFILE_SCOPE("Engine: 4. GUI Update Loop");
                m_GuiLayer->Begin();
                {

                    for (Layer* layer : m_LayerStack) {
                        layer->OnGuiRender();
                    }
                }
                m_GuiLayer->End();
            }

            {
                L_PROFILE_SCOPE("Engine: 5. Update Window (Finish GL Commands)");
                glFinish();
                m_Input->ResetScroll();
                m_Window->OnUpdate();
            }
        }

        Audio::Shutdown();
        Time::Shutdown();

        // Shutdown scene properly if still running
        auto scene = Project::GetActiveScene();
        if (scene->IsRunning())
            scene->OnRuntimeStop();

        if (scene->IsSimulating())
            scene->OnSimulationStop();

        scene->OnStop();
    }

    bool Engine::OnWindowClose() {
        m_Running = false;
        return true;
    }

    bool Engine::OnWindowResize() {
        return false;
    }

    void Engine::ExecuteMainThreadQueue()
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        for (auto& func : m_MainThreadQueue)
            func();

        m_MainThreadQueue.clear();
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

