#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <chrono>

// External Vendor Library Headers

namespace Louron {

	class Time {

	public:

		static void Init() {
			Time::Get();
		}

		static Time& Get() {
			static Time s_Instance;

			return s_Instance;
		}

		void UpdateTime() {
			m_CurrentTimeClock = std::chrono::high_resolution_clock::now();

			m_CurrentTime = std::chrono::duration_cast<std::chrono::duration<double>>(m_CurrentTimeClock.time_since_epoch()).count();
			m_DeltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(m_CurrentTimeClock - m_LastTimeClock).count();

			m_LastTimeClock = m_CurrentTimeClock;

			m_FrameRateTimer -= (float)m_DeltaTime;
			if (m_FrameRateTimer <= 0.0f) {
				m_FrameRateEstimate = (int)(1.0f / (float)m_DeltaTime);
				m_FrameRateTimer = 1.0f;
			}

		}

		static float GetUnscaledDeltaTime()			{ return (float)Time::Get().m_DeltaTime; }
		static float GetDeltaTime()					{ return (float)Time::Get().m_DeltaTime * Time::Get().m_TimeScale; }

		static float GetUnscaledFixedDeltaTime()	{ return Time::Get().m_FixedDeltaTime; }
		static float GetFixedDeltaTime()			{ return Time::Get().m_FixedDeltaTime * Time::Get().m_TimeScale; }

		static void SetFixedDeltaTime(const float& fixedDeltaTime) {
			L_CORE_INFO("Fixed Update Frequency Interval Changed To: {0} seconds", fixedDeltaTime);
			Time::Get().m_FixedDeltaTime = fixedDeltaTime;
		}

		static int GetUnscaledFixedUpdatesHz() { return (int)(1.0f / Time::Get().m_FixedDeltaTime); }
		static int GetFixedUpdatesHz() { return (int)(1.0f / Time::Get().m_FixedDeltaTime * Time::Get().m_TimeScale); }

		static float GetTimeScale() { return Time::Get().m_TimeScale; }
		static void SetTimeScale(const float& timeScale) { Time::Get().m_TimeScale = timeScale; }
		
		/// <summary>Estimate of average framerate.</summary>
		static int GetFrameRate() { return Time::Get().m_FrameRateEstimate; } // TODO: Average this from last two seconds of runtime for smoother and more accurate FPS

		double GetCurrTime() const { return m_CurrentTime; }

	private:

		Time() {
			m_LastTimeClock = std::chrono::high_resolution_clock::now();
		}

		// Delete copy assignment and move assignment constructors
		Time(const Time&) = delete;
		Time(Time&&) = delete;

		// Delete copy assignment and move assignment operators
		Time& operator=(const Time&) = delete;
		Time& operator=(Time&&) = delete;

		double m_CurrentTime	{ 0.0 };
		double m_DeltaTime		{ 0.0 };
		float m_FixedDeltaTime	{ 1.0f / 60.0f };
		float m_TimeScale		{ 1.0f };

		int m_FrameRateEstimate	{ 0 };
		float m_FrameRateTimer	{ 1.0f };

		std::chrono::time_point<std::chrono::high_resolution_clock> m_LastTimeClock;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_CurrentTimeClock;

	};

}