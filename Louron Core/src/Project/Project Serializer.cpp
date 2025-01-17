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

        if (projectFilePath.extension() != ".lproj") {
            L_CORE_WARN("Incompatible Project File Extension");
            L_CORE_WARN("Extension Used: {0}", projectFilePath.extension().string());
            L_CORE_WARN("Extension Expected: .lproj");
            return false;
        }

        const auto& config = m_Project->GetConfig();

        YAML::Emitter out;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Project Name" << YAML::Value << config.Name;

            out << YAML::Key << "Project Config" << YAML::Value;
            {
                out << YAML::BeginMap;
                out << YAML::Key << "StartScene" << config.StartScene.string();
                out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
                out << YAML::Key << "AssetRegistry" << YAML::Value << config.AssetRegistry.string(); // Relative to AssetDirecotry
                out << YAML::Key << "CoreScriptModulePath" << YAML::Value << config.CoreScriptAssemblyPath.string(); // Relative to AssetDirecotry
                out << YAML::Key << "AppScriptModulePath" << YAML::Value << config.AppScriptAssemblyPath.string(); // Relative to AssetDirecotry
                out << YAML::EndMap;
            }
            out << YAML::EndMap;
        }
            
        std::filesystem::create_directories(projectFilePath.parent_path());

        std::ofstream fout(projectFilePath);
        fout << out.c_str();

        return true;
        
    }

    bool ProjectSerializer::Deserialize(const std::filesystem::path& projectFilePath) {

        if (projectFilePath.extension() != ".lproj") {
            L_CORE_WARN("Incompatible Project File Extension");
            L_CORE_WARN("Extension Used: {0}", projectFilePath.extension().string());
            L_CORE_WARN("Extension Expected: .lproj");

            return false;
        } 

        ProjectConfig config = m_Project->GetConfig();

        YAML::Node data;
        try {
            data = YAML::LoadFile(projectFilePath.string());
        }
        catch (YAML::ParserException e) {
            L_CORE_ERROR("YAML-CPP Failed to Load Project File: '{0}', {1}", projectFilePath.string(), e.what());
            return false;
        }

        if (!data["Project Name"] || !data["Project Config"]) {
            L_CORE_ERROR("Project Node's Not Correctly Declared in File: \'{0}\'", projectFilePath.string());
            return false;
        }

        config.Name = data["Project Name"].as<std::string>();

        auto projectConfig = data["Project Config"];
            
        if(projectConfig["StartScene"])
            config.StartScene = projectConfig["StartScene"].as<std::string>();
            
        if(projectConfig["AssetDirectory"])
            config.AssetDirectory = projectConfig["AssetDirectory"].as<std::string>();

        if (projectConfig["AssetRegistry"])
            config.AssetRegistry = projectConfig["AssetRegistry"].as<std::string>();

        if (projectConfig["CoreScriptModulePath"])
            config.CoreScriptAssemblyPath = projectConfig["CoreScriptModulePath"].as<std::string>();

        if (projectConfig["AppScriptModulePath"])
            config.AppScriptAssemblyPath = projectConfig["AppScriptModulePath"].as<std::string>();

        m_Project->SetConfig(config);

        return true;
    }
}

#pragma warning( pop )