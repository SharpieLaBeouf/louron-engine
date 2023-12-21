#include "Louron.h"
#include "Core/EngineStart.h"

#include "Sandbox Layer/MainMenuLayer.h"

class SandboxEngine : public Louron::Engine {

public:
	SandboxEngine(const Louron::EngineSpecification& specification) : Louron::Engine(specification) {
		PushLayer(new MainMenuLayer());
	}

};

Louron::Engine* Louron::CreateEngine(Louron::EngineCommandLineArgs args) {
	EngineSpecification spec;
	spec.Name = "Sandbow Engine";
	spec.WorkingDirectory = "SanboxLayer";
	spec.CommandLineArgs = args;

	return new SandboxEngine(spec);
}