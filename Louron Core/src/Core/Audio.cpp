#include "Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#include <stdexcept>
#include <mutex>

#include "Logging.h"
#include "../Debug/Assert.h"

namespace Louron {

	static Audio* s_Instance = nullptr;

	Audio::Audio() {

		ma_result result;
		ma_engine* engine = new ma_engine();

		result = ma_engine_init(nullptr, engine);
		if (result != MA_SUCCESS) {
			L_CORE_ERROR("Audio Engine Not Initialised.");
			return;
		}
		m_SoundEngine = engine;
	}

	void Audio::Init() {

		L_CORE_ASSERT(!s_Instance, "Audio Already Initialised!");

		s_Instance = new Audio();
	}

	void Audio::Shutdown() {
		delete s_Instance;
		s_Instance = nullptr;
	}

	Audio& Audio::Get() {
		
		L_CORE_ASSERT(s_Instance, "Audio Not Initialised!");

		return *s_Instance;
	}

	void Audio::PlayAudioFile(const std::string& audioFile) {
		ma_engine_play_sound(m_SoundEngine, audioFile.c_str(), nullptr);
	}

	Audio::~Audio() {
		ma_engine_uninit(m_SoundEngine);
	}

}