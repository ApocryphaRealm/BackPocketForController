#include "PCH.h"

#include "DevBenchTool.h"

#include "DevBench/DevBenchAPI.h"
#include "HoldToPocket.h"
#include "SelfCheck.h"
#include "Settings.h"
#include "utils/Logger.h"

#include <cmath>
#include <format>
#include <string>
#include <string_view>

namespace DevBenchTool
{
	namespace
	{
		std::string EscapeJson(std::string_view a_in)
		{
			std::string out;
			out.reserve(a_in.size() + 8);
			for (const char c : a_in)
			{
				switch (c)
				{
				case '\\': out += "\\\\"; break;
				case '"': out += "\\\""; break;
				case '\n': out += "\\n"; break;
				default: out += c; break;
				}
			}
			return out;
		}

		void ControlTool(void*, const char* a_argsJson, void* a_sink, DevBenchAPI::WriteFn a_write)
		{
			const std::string_view args = a_argsJson ? a_argsJson : "";
			if (args.find("\"tap\"") != std::string_view::npos)
			{
				HoldToPocket::QueueTestPress(2);
				a_write(a_sink, R"({"ok":true,"op":"tap","frames":2})");
				return;
			}
			if (args.find("\"hold\"") != std::string_view::npos)
			{
				// long enough to cross the configured hold time by a few frames
				const int frames = static_cast<int>(std::ceil(settings::general::holdSeconds * 60.0F)) + 6;
				HoldToPocket::QueueTestPress(frames);
				a_write(a_sink, std::format(R"({{"ok":true,"op":"hold","frames":{}}})", frames).c_str());
				return;
			}

			const auto s = HoldToPocket::GetSnapshot();
			const std::string json = std::format(
				"{{\"ok\":true,"
				"\"settings\":{{\"holdSeconds\":{:.2f},\"logLevel\":{},\"iniPath\":\"{}\"}},"
				"\"originalBackPocketAlsoLoaded\":{},"
				"\"runtime\":{{\"hookInstalled\":{},\"armed\":{},\"holdsTimed\":{},\"sentIn\":{},\"takenOut\":{},\"tapsGivenBack\":{},"
				"\"passedNotEligible\":{},\"passedNotInventory\":{},\"passedFrameworkOpen\":{},"
				"\"lastDecision\":\"{}\",\"lastItem\":\"{}\"}}}}",
				settings::general::holdSeconds, settings::debug::logLevel, EscapeJson(settings::GetIniPath()),
				s.upstreamLoaded,
				s.hookInstalled, s.armed, s.armedCount, s.sentIn, s.takenOut, s.tapsGivenBack,
				s.passedNotEligible, s.passedNotInventory, s.passedFrameworkOpen,
				EscapeJson(s.lastDecision), EscapeJson(s.lastItem));
			a_write(a_sink, json.c_str());
		}
	}

	void Init(bool a_lastAttempt)
	{
		static bool registered = false;
		if (registered) { return; }

		DevBenchAPI::IDevBenchInterface001* devBench = DevBenchAPI::GetDevBenchInterface001();
		if (!devBench)
		{
			if (a_lastAttempt)
			{
				logger::info("DevBench not detected; skipping the \"bpfc.control\" tool");
				SelfCheck::Set("DevBench tool", true, "DevBench is not installed; nothing to register (normal for players)");
			}
			else
			{
				logger::debug("DevBench not detected yet; will retry at the next message");
			}
			return;
		}

		constexpr const char* descriptor =
			"{"
			"\"description\":\"Back Pocket for Controller live state: whether the original Back Pocket DLL is also loaded, "
			"the hold time, and what the last Y press did. op=tap queues a short Y press, op=hold a Y press held past "
			"the hold time (both go through the mod's own filter like a pad press).\","
			"\"inputSchema\":{\"type\":\"object\",\"properties\":{\"op\":{\"type\":\"string\"}}},"
			"\"readOnly\":false"
			"}";

		if (devBench->RegisterTool("bpfc.control", descriptor, &ControlTool, nullptr))
		{
			logger::info("Registered \"bpfc.control\" with DevBench (build {})", devBench->GetBuildNumber());
			SelfCheck::Set("DevBench tool", true, "\"bpfc.control\" registered");
			registered = true;
		}
		else
		{
			SelfCheck::Set("DevBench tool", false, "DevBench refused to register \"bpfc.control\"");
		}
	}
}
