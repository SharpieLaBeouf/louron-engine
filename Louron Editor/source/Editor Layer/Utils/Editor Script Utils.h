#pragma once

#include "Louron.h"

namespace Utils {

	static void GenerateScriptingProject(const std::string& project_name, const std::filesystem::path& path)
	{

		std::string project_name_no_spaces = project_name;
		project_name_no_spaces.erase(remove(project_name_no_spaces.begin(), project_name_no_spaces.end(), ' '), project_name_no_spaces.end()); // Remove Spaces

		std::filesystem::path solution_file_path = path / (project_name_no_spaces + ".sln");
		std::filesystem::path cs_proj_file_path = path / (project_name_no_spaces + ".csproj");

		// Solution File 
		std::ofstream solution_file(solution_file_path);

		if (!solution_file.is_open()) {
			L_CORE_ERROR("Failed to Generate Scripting Solution File: {}", solution_file_path.string());
			return;
		}

		// Content with project name inserted
		std::string content =
			"Microsoft Visual Studio Solution File, Format Version 12.00\n"
			"# Visual Studio Version 17\n"
			"VisualStudioVersion = 17.12.35527.113 d17.12\n"
			"MinimumVisualStudioVersion = 10.0.40219.1\n"
			"Project(\"{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}\") = \"" + project_name_no_spaces + "\", \"" + project_name_no_spaces + ".csproj\", \"{35131DDB-6653-4F8F-8A72-80F36259CB74}\"\n"
			"EndProject\n"
			"Global\n"
			"    GlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
			"        Debug|Any CPU = Debug|Any CPU\n"
			"        Release|Any CPU = Release|Any CPU\n"
			"    EndGlobalSection\n"
			"    GlobalSection(ProjectConfigurationPlatforms) = postSolution\n"
			"        {35131DDB-6653-4F8F-8A72-80F36259CB74}.Debug|Any CPU.ActiveCfg = Debug|Any CPU\n"
			"        {35131DDB-6653-4F8F-8A72-80F36259CB74}.Debug|Any CPU.Build.0 = Debug|Any CPU\n"
			"        {35131DDB-6653-4F8F-8A72-80F36259CB74}.Release|Any CPU.ActiveCfg = Release|Any CPU\n"
			"        {35131DDB-6653-4F8F-8A72-80F36259CB74}.Release|Any CPU.Build.0 = Release|Any CPU\n"
			"    EndGlobalSection\n"
			"    GlobalSection(SolutionProperties) = preSolution\n"
			"        HideSolutionNode = FALSE\n"
			"    EndGlobalSection\n"
			"EndGlobal\n";

		// Write the content to the file
		solution_file << content;

		solution_file.close();

		L_CORE_INFO("Scripting solution file created at: {}", solution_file_path.string());

		// CS PROJ
		std::ofstream cs_project_file(cs_proj_file_path);

		if (!cs_project_file.is_open()) {
			L_CORE_ERROR("Failed to Generate Scripting CS Proj File: {}", cs_proj_file_path.string());
			return;
		}

		// Content with project name and script core DLL placeholder
		content =
			"<Project Sdk=\"Microsoft.NET.Sdk\">\n\n"
			"  <PropertyGroup>\n"
			"    <OutputType>Library</OutputType>\n"
			"    <TargetFramework>netcoreapp3.1</TargetFramework>\n"
			"    <RootNamespace>" + project_name_no_spaces + "</RootNamespace>\n\n"
			"    <AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>\n"
			"    <AppendRuntimeIdentifierToOutputPath>false</AppendRuntimeIdentifierToOutputPath>\n\n"
			"    <BaseOutputPath>Binaries</BaseOutputPath>\n"
			"    <OutputPath>Binaries</OutputPath>\n\n"
			"  </PropertyGroup>\n\n"
			"  <ItemGroup>\n"
			"    <Reference Include=\"Louron Script Core\">\n"
			"      <HintPath>" + std::filesystem::current_path().string() + "\\Resources\\Scripting\\Louron Script Core.dll</HintPath>\n"
			"    </Reference>\n"
			"  </ItemGroup>\n\n"
			"  <!-- Automatically include all files in the Source folder -->\n"
			"  <ItemGroup>\n"
			"    <Folder Include=\"Source\\\" />\n"
			"  </ItemGroup>\n\n"
			"</Project>\n";

		// Write the content to the file
		cs_project_file << content;

		cs_project_file.close();

		L_CORE_INFO("Scripting csproj file created at: {}", cs_proj_file_path.string());


	}

	static std::string GenerateScriptFile(const std::string& project_name, const std::filesystem::path& file_path)
	{
		std::string projectName = project_name;
		std::string scriptName = file_path.stem().string();

		projectName.erase(std::remove(projectName.begin(), projectName.end(), ' '), projectName.end());
		scriptName.erase(std::remove(scriptName.begin(), scriptName.end(), ' '), scriptName.end());

		std::string content =
			"using System;\n"
			"using System.Collections.Generic;\n"
			"using System.Text;\n\n"
			"using Louron;\n\n"
			"namespace " + projectName + "\n"
			"{\n"
			"\n"
			"	public class " + scriptName + " : Entity\n"
			"	{\n"
			"		\n"
			"		public void OnStart()\n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"		public void OnUpdate()\n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"		public void OnFixedUpdate() \n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"		public void OnDestroy() \n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"		// Collider Functions\n"
			"		public void OnCollideEnter(Collider other) \n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"		public void OnCollideStay(Collider other) \n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"		public void OnCollideLeave(Collider other) \n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"		// Collider Trigger Functions\n"
			"		public void OnTriggerEnter(Collider other) \n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"		public void OnTriggerStay(Collider other) \n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"		public void OnTriggerLeave(Collider other) \n"
			"		{\n"
			"			\n"
			"		}\n"
			"		\n"
			"	}\n"
			"	\n"
			"}";

		// C# Script File 
		std::ofstream script_file_path(file_path);

		if (!script_file_path.is_open()) {
			L_CORE_ERROR("Failed to Generate C# Script File: {}", file_path.string());
			return "";
		}

		// Write the content to the file
		script_file_path << content;

		script_file_path.close();

		std::string command = "start \"\" \"" + file_path.string() + "\"";
		std::system(command.c_str());

		L_CORE_INFO("Scripting solution file created at: {}", file_path.string());
		L_CORE_INFO("Compile the App Assembly to Access Script Class.");
		return projectName + "." + scriptName;
	}

	static bool CompileAppAssembly(const std::filesystem::path& cs_proj_file_path)
	{
		L_CORE_INFO("[C# Compile] Attempting to Compile C# Scripting Project: {}", cs_proj_file_path.filename().string());

		// Construct the build command
		std::string buildCommand = std::format("dotnet build \"{}\" -c Release", cs_proj_file_path.string());

		// Execute the command and capture the return code
		int retCode = std::system(buildCommand.c_str());

		if (retCode == 0) {
			L_CORE_INFO("[C# Compile] Successfully Compiled C# Scripting Project: {}", cs_proj_file_path.filename().string());
			return true;
		}
		else {
			// Maybe add some error handling in here to get the compilation error reasons? IDK how to do that lol
			L_CORE_ERROR("[C# Compile] Could Not Compile C# Scripting Project: {}", cs_proj_file_path.filename().string());
			return false;
		}

	}

}