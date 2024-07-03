#include "Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#include <stdexcept>
#include <mutex>

#include "Logging.h"

namespace Louron {

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

	Audio& Audio::Get() {
		static std::once_flag onceFlag;
		static Audio* s_Instance = nullptr;

		std::call_once(onceFlag, []() {
			s_Instance = new Audio();
			});

		return *s_Instance;
	}

	void Audio::PlayAudioFile(const std::string& audioFile) {
		ma_engine_play_sound(m_SoundEngine, audioFile.c_str(), nullptr);
	}

	Audio::~Audio() {
		ma_engine_uninit(m_SoundEngine);
	}

}