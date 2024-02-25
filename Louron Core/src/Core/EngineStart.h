#pragma once

// Louron Core Headers
#include "Engine.h"
#include "Logging.h"

// C++ Standard Library Headers

// External Vendor Library Headers

extern Louron::Engine* Louron::CreateEngine(Louron::EngineCommandLineArgs args);

int main(int argc, char** argv)
{
	Louron::LoggingSystem::Init();

	auto app = Louron::CreateEngine({ argc, argv });

	app->Run();

	delete app;
}