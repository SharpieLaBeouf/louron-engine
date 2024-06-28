#include "Louron.h"
#include "Core/EngineStart.h"

#include "Sandbox Layer/MainMenuLayer.h"

class SandboxEngine : public Louron::Engine {

public:
	SandboxEngine(const Louron::EngineSpecification& specification) : Louron::Engine(specification) {
		PushLayer(new MainMenuLayer());
	}

	~SandboxEngine() = default;
};

Louron::Engine* Louron::CreateEngine(Louron::EngineCommandLineArgs args) {
	EngineSpecification spec;
	spec.Name = "Louron Sandbox Application";
	spec.WorkingDirectory = "";
	spec.CommandLineArgs = args;

	return new SandboxEngine(spec);
}