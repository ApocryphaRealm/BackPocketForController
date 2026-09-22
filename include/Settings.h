#pragma once

// Back Pocket for Controller - settings. Plain-file INI (never the Win32 profile API, so
// PrivateProfileRedirector can neither serve stale values nor overwrite the file). No settings page:
// the owner, 2026-09-22, "There shouldn't be an AMF settings page".

#include <cstdint>
#include <string>

namespace settings
{
	namespace debug
	{
		inline std::uint32_t logLevel = 0;  // uLogLevel:Debug - 0 = trace (project default)
	}

	namespace general
	{
		// How long Y must be held on a favourited item before it goes to the Back Pocket. A shorter
		// press is handed back to the inventory as the ordinary Favourite press.
		inline float holdSeconds = 0.50F;   // fHoldSeconds:General
	}

	void Init(const std::string& a_iniFileName);
	void ApplyLogLevel();
	const std::string& GetIniPath();
}
