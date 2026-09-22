#include "PCH.h"

#include "Settings.h"

#include "utils/Logger.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace settings
{
	namespace
	{
		std::string iniPath;

		constexpr float kMinHold = 0.20F;
		constexpr float kMaxHold = 2.00F;

		std::string Lower(std::string a_s)
		{
			for (char& c : a_s) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
			return a_s;
		}

		std::string Trim(const std::string& a_s)
		{
			const auto b = a_s.find_first_not_of(" \t\r\n");
			if (b == std::string::npos) { return {}; }
			const auto e = a_s.find_last_not_of(" \t\r\n");
			return a_s.substr(b, e - b + 1);
		}

		bool ParseFloat(const std::string& a_text, float& a_out)
		{
			try { a_out = std::stof(Trim(a_text)); return true; } catch (...) { return false; }
		}

		bool ParseBool(const std::string& a_text, bool& a_out)
		{
			const std::string v = Lower(Trim(a_text));
			if (v == "true" || v == "1") { a_out = true; return true; }
			if (v == "false" || v == "0") { a_out = false; return true; }
			return false;
		}

		bool ParseUInt(const std::string& a_text, std::uint32_t& a_out)
		{
			try { a_out = static_cast<std::uint32_t>(std::stoull(Trim(a_text), nullptr, 0)); return true; } catch (...) { return false; }
		}
	}

	void Init(const std::string& a_iniFileName)
	{
		iniPath = (std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / a_iniFileName).string();
		std::ifstream in(iniPath);
		if (!in)
		{
			logger::warn("INI not found at {}; using the compiled defaults (hold {:.2f}s, log level {})", iniPath, general::holdSeconds, debug::logLevel);
			return;
		}
		std::map<std::string, std::string> keys;
		std::string line, section;
		while (std::getline(in, line))
		{
			const std::string t = Trim(line);
			if (t.empty() || t[0] == ';' || t[0] == '#') { continue; }
			if (t.front() == '[' && t.back() == ']') { section = Lower(t.substr(1, t.size() - 2)); continue; }
			const auto eq = t.find('=');
			if (eq == std::string::npos) { continue; }
			keys[Lower(Trim(t.substr(0, eq))) + ":" + section] = Trim(t.substr(eq + 1));
		}
		if (const auto it = keys.find("uloglevel:debug"); it != keys.end() && !ParseUInt(it->second, debug::logLevel))
		{
			logger::warn("uLogLevel \"{}\" is not a number; keeping {}", it->second, debug::logLevel);
		}
		if (const auto it = keys.find("fholdseconds:general"); it != keys.end() && !ParseFloat(it->second, general::holdSeconds))
		{
			logger::warn("fHoldSeconds \"{}\" is not a number; keeping {:.2f}", it->second, general::holdSeconds);
		}
		if (const auto it = keys.find("brequirefavourite:general"); it != keys.end() && !ParseBool(it->second, general::requireFavourite))
		{
			logger::warn("bRequireFavourite \"{}\" is not true/false or 1/0; keeping {}", it->second, general::requireFavourite);
		}
		if (general::holdSeconds < kMinHold || general::holdSeconds > kMaxHold)
		{
			const float was = general::holdSeconds;
			general::holdSeconds = std::clamp(general::holdSeconds, kMinHold, kMaxHold);
			logger::warn("fHoldSeconds {:.2f} is outside {:.2f}-{:.2f}; using {:.2f}", was, kMinHold, kMaxHold, general::holdSeconds);
		}
		logger::info("settings loaded from {}: holdSeconds={:.2f} requireFavourite={} logLevel={}", iniPath,
					 general::holdSeconds, general::requireFavourite, debug::logLevel);
	}

	void ApplyLogLevel()
	{
		const auto lvl = static_cast<spdlog::level::level_enum>(std::clamp<std::uint32_t>(debug::logLevel, 0u, 6u));
		SKSE::log::set_level(lvl, lvl);
	}

	const std::string& GetIniPath() { return iniPath; }
}
