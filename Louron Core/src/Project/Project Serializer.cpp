#include "Project Serializer.h"

// Louron Core Headers

// C++ Standard Library Headers
#include <fstream>

// External Vendor Library Headers
#pragma warning( push )
#pragma warning( disable : 4099)

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

namespace Louron {

    ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project) : m_Project(project) { }

    bool ProjectSerializer::Serialize(const std::filesystem::path& projectFilePath) {

        L_CORE_ASSERT((projectFilePath.extension() == ".lproj"), "Incompatible Project File Extension! Extension used: " + projectFilePath.extension().string() + ", Extension Required: .lproj");
        
        if (projectFilePath.extension() == ".lproj") {

            const auto& config = m_Project->GetConfig();

            YAML::Emitter out;
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Project Name" << YAML::Value << config.Name;

                out << YAML::Key << "Project Config" << YAML::Value;
                {
                    out << YAML::BeginMap;
                    out << YAML::Key << "StartScene" << YAML::Value << config.StartScene.string();
                    out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
                    out << YAML::EndMap;
                }
                out << YAML::EndMap;
            }
            
            std::filesystem::create_directories(projectFilePath.parent_path());

            std::ofstream fout(projectFilePath);
            fout << out.c_str();

            return true;
        }
        return false;
    }

    bool ProjectSerializer::Deserialize(const std::filesystem::path& projectFilePath) {

        L_CORE_ASSERT((projectFilePath.extension() == ".lproj"), "Incompatible Project File Extension! Extension used: " + projectFilePath.extension().string() + ", Extension Required: .lproj");

        if (projectFilePath.extension() == ".lproj") {
            ProjectConfig config = m_Project->GetConfig();

            YAML::Node data;
            try {
                data = YAML::LoadFile(projectFilePath.string());
            }
            catch (YAML::ParserException e) {
                L_CORE_ASSERT(false, "YAML-CPP Failed to Load Project File: " + projectFilePath.string() + ", " + e.what());
                return false;
            }

            if (!data["Project Name"] || !data["Project Config"]) {
                L_CORE_ASSERT(false, "Project Node's Not Correctly Declared in File : " + projectFilePath.string());
                return false;
            }

            config.Name = data["Project Name"].as<std::string>();

            auto projectConfig = data["Project Config"];
            config.StartScene = projectConfig["StartScene"].as<std::string>();
            config.AssetDirectory = projectConfig["AssetDirectory"].as<std::string>();

            m_Project->SetConfig(config);
            return true;
        }

        return false;
    }

}

#pragma warning( pop )