#include "Engine.h"

extern Louron::Engine* Louron::CreateEngine(Louron::EngineCommandLineArgs args);

int main(int argc, char** argv)
{
	auto app = Louron::CreateEngine({ argc, argv });

	app->Run();

	delete app;
}