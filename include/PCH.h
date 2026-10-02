#pragma once

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

#ifndef NDEBUG
#include <spdlog/sinks/msvc_sink.h>
#endif

#define DLLEXPORT __declspec(dllexport)

using namespace std::literals;
using namespace REL::literals;

namespace logger = SKSE::log;

#if RUNTIME_LINE == 17
// CommonLibSSE-NG 7.2 (the Skyrim 1.7.x line) renamed RE::DebugNotification to RE::SendHUDMessage::ShowHUDMessage,
// same arguments; the old name is kept so both lines build from one source.
namespace RE
{
	inline void DebugNotification(const char* a_notification, const char* a_soundToPlay = nullptr, bool a_cancelIfAlreadyQueued = true)
	{
		SendHUDMessage::ShowHUDMessage(a_notification, a_soundToPlay, a_cancelIfAlreadyQueued);
	}
}
#endif
