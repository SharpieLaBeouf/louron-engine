#include "Louron.h"
#include "Core/EngineStart.h"

#include "Editor Layer/Louron Editor Layer.h"

class LouronEditorApplication : public Louron::Engine {

public:
	LouronEditorApplication(const Louron::EngineConfig& specification) : Louron::Engine(specification) {
		PushLayer(new LouronEditorLayer());
	}

	~LouronEditorApplication() = default;
};

Louron::Engine* Louron::CreateEngine(Louron::EngineCommandLineArgs args) {
	EngineConfig spec;
	spec.Name = "Louron Sandbox Application";
	spec.WorkingDirectory = "";
	spec.CommandLineArgs = args;

	return new LouronEditorApplication(spec);
}