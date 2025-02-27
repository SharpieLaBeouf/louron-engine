#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <memory>

// External Vendor Library Headers
#pragma warning( push )
#pragma warning( disable : 4996 )
#include <spdlog/spdlog.h>

namespace Louron {

	class LoggingSystem {

	public:

		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetApplicationLogger() { return s_ApplicationLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ApplicationLogger;

	};

}

#define L_CORE_INFO(...)   ::Louron::LoggingSystem::GetCoreLogger()->info(__VA_ARGS__)
#define L_CORE_TRACE(...)  ::Louron::LoggingSystem::GetCoreLogger()->trace(__VA_ARGS__)
#define L_CORE_WARN(...)   ::Louron::LoggingSystem::GetCoreLogger()->warn(__VA_ARGS__)
#define L_CORE_ERROR(...)  ::Louron::LoggingSystem::GetCoreLogger()->error(__VA_ARGS__)
#define L_CORE_FATAL(...)  ::Louron::LoggingSystem::GetCoreLogger()->critical(__VA_ARGS__)

#define L_APP_INFO(...)   ::Louron::LoggingSystem::GetApplicationLogger()->info(__VA_ARGS__)
#define L_APP_TRACE(...)  ::Louron::LoggingSystem::GetApplicationLogger()->trace(__VA_ARGS__)
#define L_APP_WARN(...)   ::Louron::LoggingSystem::GetApplicationLogger()->warn(__VA_ARGS__)
#define L_APP_ERROR(...)  ::Louron::LoggingSystem::GetApplicationLogger()->error(__VA_ARGS__)
#define L_APP_FATAL(...)  ::Louron::LoggingSystem::GetApplicationLogger()->critical(__VA_ARGS__)

#pragma warning( pop )