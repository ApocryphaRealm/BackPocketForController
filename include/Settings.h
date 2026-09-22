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

		// Whether an item has to be a favourite before a hold can send it to the Back Pocket (the owner,
		// 2026-09-22). ON keeps 1.0.0's behaviour: favourites go in, anything already in the Back Pocket comes
		// out, every other item is left to the menu. OFF lets a hold pocket any highlighted item.
		inline bool requireFavourite = true;  // bRequireFavourite:General
	}

	void Init(const std::string& a_iniFileName);
	void ApplyLogLevel();
	const std::string& GetIniPath();
}
