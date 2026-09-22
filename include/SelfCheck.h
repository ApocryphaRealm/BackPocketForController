#pragma once

// Self-check report (the owner, 2026-09-19: "have it log its findings. In a text file"). One named
// check per install point - the Address Library guard, the INI, the input hook, Back Pocket being
// loaded and its toggle key, the settings page, the DevBench tool - each OK or FAIL with a detail,
// written to Documents\My Games\Skyrim Special Edition\SKSE\BackPocketForController-selfcheck.txt
// and REWRITTEN as the state changes, with the version and runtime at the top and the live counters
// at the bottom.

#include <string>

namespace SelfCheck
{
	void Set(const std::string& a_name, bool a_ok, const std::string& a_detail);
	void Write();
}
