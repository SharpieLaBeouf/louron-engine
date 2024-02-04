#pragma once

#include "irrklang/irrKlang.h"

#include <string>

namespace Louron {

	class Audio {

	public:

		static void Init() {
			s_Instance = new Audio();
		}

		static Audio& Get() {
			if (!s_Instance)
				Init();

			return *s_Instance;
		}

		void PlaySound(const std::string& audioFile, bool looped = false) {
			m_SoundEngine->play2D(audioFile.c_str(), looped);
		}

	private:

		Audio() {
			m_SoundEngine = irrklang::createIrrKlangDevice();
		}

		// Delete copy assignment and move assignment constructors
		Audio(const Audio&) = delete;
		Audio(Audio&&) = delete;

		// Delete copy assignment and move assignment operators
		Audio& operator=(const Audio&) = delete;
		Audio& operator=(Audio&&) = delete;

		static Audio* s_Instance;
		irrklang::ISoundEngine* m_SoundEngine;


	};
}