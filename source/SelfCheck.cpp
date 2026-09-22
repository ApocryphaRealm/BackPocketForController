#include "PCH.h"

#include "SelfCheck.h"

#include "HoldToPocket.h"
#include "Settings.h"

#include "utils/Logger.h"

#include <chrono>
#include <format>
#include <fstream>
#include <mutex>
#include <vector>

namespace SelfCheck
{
	namespace
	{
		struct Check
		{
			std::string name;
			bool ok;
			std::string detail;
		};
		std::mutex g_lock;
		std::vector<Check> g_checks;
	}

	void Set(const std::string& a_name, bool a_ok, const std::string& a_detail)
	{
		{
			std::scoped_lock l(g_lock);
			bool found = false;
			for (auto& c : g_checks)
			{
				if (c.name == a_name) { c.ok = a_ok; c.detail = a_detail; found = true; break; }
			}
			if (!found) { g_checks.push_back({ a_name, a_ok, a_detail }); }
		}
		if (a_ok) { logger::debug("self-check {}: OK - {}", a_name, a_detail); }
		else { logger::warn("self-check {}: FAIL - {}", a_name, a_detail); }
		Write();
	}

	void Write()
	{
		const auto dir = SKSE::log::log_directory();
		if (!dir)
		{
			static bool warned = false;
			if (!warned) { warned = true; logger::warn("self-check: no log directory; report not written"); }
			return;
		}
		const auto path = *dir / "BackPocketForController-selfcheck.txt";
		const auto snap = HoldToPocket::GetSnapshot();

		std::vector<Check> checks;
		{
			std::scoped_lock l(g_lock);
			checks = g_checks;
		}
		std::ofstream out(path, std::ios::trunc);
		if (!out)
		{
			static bool warned = false;
			if (!warned) { warned = true; logger::warn("self-check: could not write {}", path.string()); }
			return;
		}
		const auto ver = SKSE::PluginDeclaration::GetSingleton()->GetVersion().string(".");
		const auto rt = REL::Module::get().version().string("-");
		out << "Back Pocket for Controller " << ver << " - self-check\n";
		out << "Runtime " << rt << "   written " << std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() }) << "\n\n";
		std::size_t failed = 0;
		for (const auto& c : checks)
		{
			out << (c.ok ? "  OK    " : "  FAIL  ") << c.name << " - " << c.detail << "\n";
			if (!c.ok) { ++failed; }
		}
		out << "\n" << (failed == 0 ? "Everything came up." : std::format("{} check(s) failed - the hold may not work; see above.", failed)) << "\n\n";
		out << "Settings: hold Y for " << std::format("{:.2f}", settings::general::holdSeconds) << " s (fHoldSeconds in " << settings::GetIniPath() << ")\n";
		out << "Counters: holds timed " << snap.armedCount << ", sent to Back Pocket " << snap.sentIn << ", taken out " << snap.takenOut
			<< ", taps given back " << snap.tapsGivenBack << ", passed (not a favourite, not in the Back Pocket) " << snap.passedNotEligible
			<< ", passed (not in the Inventory) " << snap.passedNotInventory << ", passed (framework window open) " << snap.passedFrameworkOpen << "\n";
		out << "Last decision: " << (snap.lastDecision.empty() ? "(none yet)" : snap.lastDecision) << "\n";
		out << "Last item: " << (snap.lastItem.empty() ? "(none yet)" : snap.lastItem) << "\n";
	}
}
