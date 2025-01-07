#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <string>

// External Vendor Library Headers

struct ma_engine;

namespace Louron {


	class Audio {

	public:

		static void Init();
		static void Shutdown();
		static Audio& Get();

		void PlayAudioFile(const std::string& audioFile);

	private:

		Audio();
		~Audio();

		// Delete copy assignment and move assignment constructors
		Audio(const Audio&) = delete;
		Audio(Audio&&) = delete;

		// Delete copy assignment and move assignment operators
		Audio& operator=(const Audio&) = delete;
		Audio& operator=(Audio&&) = delete;

		ma_engine* m_SoundEngine = nullptr;

	};
}