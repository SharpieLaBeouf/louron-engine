#pragma once

#include <unordered_map>
#include <memory>
#include <chrono>

namespace Louron {

	struct ProfileResult {
		const char* Name;
		float Time;
	};

	class Profiler {

	public:

		static void Init() {
			s_Instance = new Profiler();
		}

		static Profiler& Get() {
			if (!s_Instance) 
				Init();
			
			return *s_Instance;
		}

		std::unordered_map<const char*, ProfileResult>& GetResults() { return m_Results; }
		void AddResult(const ProfileResult& result) { m_Results[result.Name] = result; }

	private:

		Profiler() { }

		// Delete copy assignment and move assignment constructors
		Profiler(const Profiler&) = delete;
		Profiler(Profiler&&) = delete;

		// Delete copy assignment and move assignment operators
		Profiler& operator=(const Profiler&) = delete;
		Profiler& operator=(Profiler&&) = delete;

		static Profiler* s_Instance;
		std::unordered_map<const char*, ProfileResult> m_Results;

	};

	class ProfileTimer {
	public:

		ProfileTimer(const char* name) : m_Name(name), m_Stopped(false) {
			m_TimerStart = std::chrono::high_resolution_clock::now();
		}
		~ProfileTimer() {
			if (!m_Stopped)
				StopTimer();
		}

		void StopTimer() {
			
			auto currentTime = std::chrono::high_resolution_clock::now();

			long long timerStart = std::chrono::time_point_cast<std::chrono::microseconds>(m_TimerStart).time_since_epoch().count();
			long long timerEnd = std::chrono::time_point_cast<std::chrono::microseconds>(currentTime).time_since_epoch().count();
			
			m_Stopped = true;

			float elapsedTime = (timerEnd - timerStart) * 0.001f;
			Profiler::Get().AddResult({ m_Name, elapsedTime });
		}

	private:

		const char* m_Name;
		std::chrono::time_point<std::chrono::steady_clock> m_TimerStart;
		bool m_Stopped;
	};

}

// Determine the Function Signature Macro depending on the compiler. Sourced and modified slightly from boost/current_function.hpp.
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	# define LOURON_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
	# define LOURON_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__) || (_MSC_VER)
	# define LOURON_CURRENT_FUNCTION __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	# define LOURON_CURRENT_FUNCTION __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	# define LOURON_CURRENT_FUNCTION __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	# define LOURON_CURRENT_FUNCTION __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
	# define LOURON_CURRENT_FUNCTION __func__
#else
	# define LOURON_CURRENT_FUNCTION "Unknown Compiler"
#endif

#define LOURON_PROFILE_SCOPE(name) ::Louron::ProfileTimer time##__LINE__(name)
#define LOURON_PROFILE_FUNCTION() LOURON_PROFILE_SCOPE(LOURON_CURRENT_FUNCTION)
