#pragma once

// Louron Core Headers

// C++ Standard Library Headers

#include <string>

// External Vendor Library Headers

#include <irrklang/irrKlang.h>

namespace Louron {

	class Audio {

	public:

		static Audio& Get() {
			static Audio s_Instance;

			return s_Instance;
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

		irrklang::ISoundEngine* m_SoundEngine;

	};
}