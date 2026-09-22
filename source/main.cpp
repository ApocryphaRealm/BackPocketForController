// Back Pocket for Controller - GPL-3.0-or-later (2026-09-22). Back Pocket (Nexus 188847, by ThePenguinT, MIT,
// github.com/theosw/SkyrimBackPocket at 6d6112c) forked into one standalone mod, with a controller hold added: in the
// Inventory, hold Y on a favourited item to send it to the Back Pocket, or on an item in the Back Pocket to take it
// out; a tap of Y stays the Favourite button. The owner, 2026-09-22: "back pocket is mit liscence so lets make this a
// standalone mod" - "in gpl 3.0". No settings page; everything is in BackPocketForController.ini.
#include "PCH.h"

#include "DevBenchTool.h"
#include "HoldToPocket.h"
#include "SelfCheck.h"
#include "Settings.h"

#include "back_pocket/config.h"
#include "back_pocket/disenchant_filter.h"
#include "back_pocket/inventory_filter.h"
#include "back_pocket/persistence.h"
#include "back_pocket/pocket.h"

#include "utils/AddressLibraryGuard.h"
#include "utils/Logger.h"

namespace
{
	struct PluginState
	{
		back_pocket::config::settings configuration;
		back_pocket::pocket pocketState;
	};

	PluginState& State()
	{
		static PluginState instance;
		return instance;
	}

	// Back Pocket's hooks carry Address Library IDs and call-site offsets for these runtimes (upstream supports
	// SE 1.5.97, AE 1.6.1170 and GOG 1.6.1179); anywhere else the mod loads inert and says so.
	bool SupportedRuntime(const REL::Version& a_v)
	{
		return a_v == REL::Version{ 1, 5, 97, 0 } || a_v == REL::Version{ 1, 6, 1170, 0 } || a_v == REL::Version{ 1, 6, 1179, 0 };
	}

	void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
	{
		if (!a_msg) { return; }
		switch (a_msg->type)
		{
		case SKSE::MessagingInterface::kPostLoad:
			HoldToPocket::Install();
			DevBenchTool::Init(false);
			break;
		case SKSE::MessagingInterface::kNewGame:
			State().pocketState.clear();
			logger::info("new game: Back Pocket cleared");
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			{
				HoldToPocket::CheckForUpstream();
				if (HoldToPocket::GetSnapshot().upstreamLoaded)
				{
					RE::DebugNotification("Back Pocket for Controller replaces Back Pocket - disable one of them");
					SelfCheck::Set("Back Pocket filters", false, "not installed: the original Back Pocket is also loaded");
					DevBenchTool::Init(true);
					break;
				}
				auto& s = State();
				s.configuration = back_pocket::config::load();
				const bool inventoryReady = back_pocket::inventory_filter::install(s.pocketState, s.configuration);
				const bool disenchantReady = inventoryReady &&
					back_pocket::disenchant_filter::install(s.pocketState, s.configuration.hide_pocketed_from_disenchanting);
				logger::info("PLUGIN_READY inventory_filter={} disenchant_filter={}", inventoryReady, disenchantReady);
				SelfCheck::Set("Back Pocket filters", inventoryReady, inventoryReady ? "inventory filter and footer installed" : "the inventory filter could not install - see the log");
				SelfCheck::Set("Disenchant protection", disenchantReady, disenchantReady ? "installed" : "not installed (optional hook) - see the log");
				if (!inventoryReady)
				{
					RE::DebugNotification("Back Pocket for Controller failed to initialize; check BackPocketForController.log");
				}
				else if (!disenchantReady)
				{
					RE::DebugNotification("Back Pocket for Controller loaded without disenchant protection; check BackPocketForController.log");
				}
				DevBenchTool::Init(true);
			}
			break;
		default:
			break;
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::log::init("BackPocketForController");
	// Address Library pre-check (the guard every mod of ours carries), BEFORE SKSE::Init, which opens the
	// Address Library itself (logic library 6026): a missing file gets a message naming it and the plugin
	// loads inert instead of CommonLibSSE-NG's bare failure line.
	if (!AddressLibraryGuard::Guard("Back Pocket for Controller"))
	{
		SelfCheck::Set("Address Library", false, "the Address Library file for this runtime is missing; the mod is inert");
		return true;
	}
	const REL::Version runtime = a_skse->RuntimeVersion();
	if (!SupportedRuntime(runtime))
	{
		logger::critical("unsupported Skyrim runtime {}; this build supports 1.5.97, 1.6.1170 and 1.6.1179", runtime.string("."));
		SelfCheck::Set("Runtime", false, "unsupported runtime " + runtime.string(".") + " - the mod is inert");
		return true;
	}
	SKSE::Init(a_skse);
	SelfCheck::Set("Address Library", true, "present for this runtime");
	SelfCheck::Set("Runtime", true, runtime.string("."));

	settings::Init("BackPocketForController.ini");
	settings::ApplyLogLevel();
	SelfCheck::Set("INI", !settings::GetIniPath().empty(), settings::GetIniPath());

	logger::info("Back Pocket for Controller {} loading (log level {}, runtime {})",
				 SKSE::PluginDeclaration::GetSingleton()->GetVersion().string("."), settings::debug::logLevel, runtime.string("."));

	if (!back_pocket::persistence::install(State().pocketState))
	{
		logger::critical("failed to register SKSE serialization");
		SelfCheck::Set("Co-save", false, "SKSE serialization could not be registered - pockets would not save");
		return true;
	}
	SelfCheck::Set("Co-save", true, "registered (Back Pocket's record, so a Back Pocket save carries its pocket over)");

	SKSE::AllocTrampoline(28);   // 14 for the Y hold's input hook + 14 for Back Pocket's disenchant hook
	if (!SKSE::GetMessagingInterface()->RegisterListener(MessageHandler))
	{
		logger::error("could not register the SKSE message listener; nothing will install");
		SelfCheck::Set("SKSE messages", false, "RegisterListener refused");
		return true;
	}
	SelfCheck::Set("SKSE messages", true, "listener registered");
	return true;
}
