#ifndef PLATFORM_DEFINITIONS_H
#define PLATFORM_DEFINITIONS_H

	// Detect the platform and define L_PLATFORM_* accordingly
	#if defined(_WIN32)
		#if defined(_WIN64)
			#define L_PLATFORM_WINDOWS
		#elif 
			#error "x86 Builds are not supported!"
			#define L_PLATFORM_WINDOWS
		#endif


	#elif defined(_WIN32) || defined(_WIN64)

		#define L_PLATFORM_WINDOWS

	#elif defined(__APPLE__) && defined(__MACH__)

		#error "MacOS is not supported!"
		#define L_PLATFORM_MAC

	#elif defined(__linux__)

		#error "Linux is not supported!"
		#define L_PLATFORM_LINUX

	#else

	#error "Unsupported platform!"

	#endif


#endif // PLATFORM_DEFINITIONS_H
