#include "Time.h"

#include "../Debug/Assert.h"

namespace Louron {

	static Time* s_Instance = nullptr;

	Time::Time() {
		m_LastTimeClock = std::chrono::high_resolution_clock::now();
	}

	void Time::Init()
	{
		L_CORE_ASSERT(!s_Instance, "Time Already Initialised!");
		s_Instance = new Time();
	}

	void Time::Shutdown()
	{
		delete s_Instance;
		s_Instance = nullptr;
	}

	Time& Time::Get() {
		L_CORE_ASSERT(s_Instance, "Time Not Initialised!");
		return *s_Instance;
	}

	void Time::UpdateTime() {
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

	float Time::GetUnscaledDeltaTime() { return (float)Time::Get().m_DeltaTime; }

	float Time::GetDeltaTime() { return (float)Time::Get().m_DeltaTime * Time::Get().m_TimeScale; }

	float Time::GetUnscaledFixedDeltaTime() { return Time::Get().m_FixedDeltaTime; }

	float Time::GetFixedDeltaTime() { return Time::Get().m_FixedDeltaTime * Time::Get().m_TimeScale; }

	void Time::SetFixedDeltaTime(const float& fixedDeltaTime) {
		L_CORE_INFO("Fixed Update Frequency Interval Changed To: {0} seconds", fixedDeltaTime);
		Time::Get().m_FixedDeltaTime = fixedDeltaTime;
	}

	int Time::GetUnscaledFixedUpdatesHz() { return (int)(1.0f / Time::Get().m_FixedDeltaTime); }

	int Time::GetFixedUpdatesHz() { return (int)(1.0f / Time::Get().m_FixedDeltaTime * Time::Get().m_TimeScale); }

	float Time::GetTimeScale() { return Time::Get().m_TimeScale; }

	void Time::SetTimeScale(const float& timeScale) { Time::Get().m_TimeScale = timeScale; }

	int Time::GetFrameRate() { return Time::Get().m_FrameRateEstimate; } // TODO: Average this from last two seconds of runtime for smoother and more accurate FPS

	double Time::GetCurrTime() const { return m_CurrentTime; }



}