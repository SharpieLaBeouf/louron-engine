#include "Logging.h"

// Louron Core Headers

// C++ Standard Library Headers

// External Vendor Library Headers
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Louron {

	std::shared_ptr<spdlog::logger> LoggingSystem::s_CoreLogger;
	std::shared_ptr<spdlog::logger> LoggingSystem::s_ApplicationLogger;

	void LoggingSystem::Init() {

		spdlog::set_pattern("[%T][%^%l%$] %n: %v");

		s_CoreLogger = spdlog::stdout_color_mt("L_CORE");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ApplicationLogger = spdlog::stdout_color_mt("L_APP");
		s_ApplicationLogger->set_level(spdlog::level::trace);
	}

}