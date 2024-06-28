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

        m_RenderFBO.Init();

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
            L_CORE_WARN("Error Finding File Paths: {0}", e.what());
        }
       
        return foundPaths;
    }

    void RenderFBO::Init() {

        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        RenderTexture = std::make_unique<Texture>("Engine_Render_Texture", Engine::Get().GetWindow().GetWidth(), Engine::Get().GetWindow().GetHeight());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTexture->GetID(), 0);
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Engine::Get().GetWindow().GetWidth(), Engine::Get().GetWindow().GetHeight()); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

        ScreenQuadVAO = std::make_unique<VertexArray>();

        float quadVertices[] = {
            // Positions       // Texture coordinates
            -1.0f,  1.0f,      0.0f, 1.0f, // LEFT - TOP
            -1.0f, -1.0f,      0.0f, 0.0f, // LEFT - BOTTOM
             1.0f, -1.0f,      1.0f, 0.0f, // RIGHT - BOTTOM
             1.0f,  1.0f,      1.0f, 1.0f  // RIGHT - TOP
        };
        unsigned int indices[] = {
             0, 1, 2,
             0, 2, 3
        };

        VertexBuffer* vbo = new VertexBuffer(quadVertices, 16);
        BufferLayout layout = {
            { ShaderDataType::Float2, "aPos" },
            { ShaderDataType::Float2, "aTexCoord" }
        };
        vbo->SetLayout(layout);

        IndexBuffer* ebo = new IndexBuffer(indices, 6);

        ScreenQuadVAO->AddVertexBuffer(vbo);
        ScreenQuadVAO->SetIndexBuffer(ebo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }
}

