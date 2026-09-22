#include "PCH.h"

#include "HoldToPocket.h"

#include "SelfCheck.h"
#include "Settings.h"
#include "back_pocket/inventory_filter.h"

#include "utils/Logger.h"

#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include <Windows.h>
#undef GetObject   // Windows.h maps it to GetObjectW, which hides InventoryEntryData::GetObject

namespace HoldToPocket
{
	namespace
	{
		// BSInputDeviceManager::PollInputDevices -> DispatchInputEvent (SE 67315 / AE 68617 + 0x7B).
		inline constexpr REL::RelocationID kPollInputDevicesID{ 67315, 68617 };
		inline constexpr std::ptrdiff_t kPollInputDevicesOffset = 0x7B;

		inline constexpr std::uint32_t kButtonY = 0x8000;   // XInput Y, as Skyrim's ButtonEvent carries it

		// Back Pocket's marks (its source, github.com/theosw/SkyrimBackPocket, MIT): the reserved filter bit it
		// sets on a pocketed row, and the boolean it puts on its own category entry.
		inline constexpr std::uint32_t kPocketFilterFlag = 0x00100000u;
		inline constexpr const char* kItemListPath = "_root.Menu_mc.inventoryLists.itemList";
		inline constexpr const char* kCategoryListPath = "_root.Menu_mc.inventoryLists.categoryList";
		inline constexpr const char* kCategoryMarker = "backPocketCategory";

		// Menus that sit ON TOP of the inventory and take the controller's buttons for themselves.
		inline constexpr const char* kCoveringMenus[] = { "MessageBoxMenu", "Book Menu", "Console" };

		struct Pending
		{
			RE::INPUT_DEVICE device;
			std::uint32_t code;
			int framesLeft;
			bool named;         // resolve the user-event name (the Y given back); false = anonymous (Back Pocket's key)
			bool mine;          // our own press, which this filter must not see again; false only for a DevBench test press
			bool downSent = false;
			int framesTotal = 0;
		};

		struct Highlight
		{
			bool valid = false;
			bool favourited = false;
			bool pocketed = false;   // the row carries Back Pocket's bit, or the Back Pocket category is showing
			std::string item;
		};

		std::mutex g_lock;                            // guards everything below
		std::vector<Pending> g_pending;
		std::vector<const RE::InputEvent*> g_ours;    // our own events spliced during the current dispatch
		Snapshot g_state;
		bool g_armed = false;                         // a Y hold is being timed
		bool g_sent = false;                          // this hold already pressed Back Pocket's key
		bool g_reportDirty = false;

		void Decide(const std::string& a_text)
		{
			// Called with g_lock held.
			g_state.lastDecision = a_text;
			g_reportDirty = true;
			logger::debug("decision: {}", a_text);
		}

		std::optional<std::uint32_t> NumberAsU32(const RE::GFxValue& a_value)
		{
			if (!a_value.IsNumber()) { return std::nullopt; }
			const double n = a_value.GetNumber();
			if (!std::isfinite(n) || n < 0.0 || n > static_cast<double>(std::numeric_limits<std::uint32_t>::max())) { return std::nullopt; }
			return static_cast<std::uint32_t>(n);
		}

		bool FrameworkWindowOpen()
		{
			// The owner, 2026-09-14: our mods' buttons do nothing while AMF's own window is open - AMF owns the input.
			using BlockingFn = bool (*)();
			static BlockingFn blocking = nullptr;
			static bool tried = false;
			if (!tried)
			{
				tried = true;
				for (const wchar_t* name : { L"!ApocryphaMenuFramework", L"ApocryphaMenuFramework", L"SKSEMenuFramework" })
				{
					if (HMODULE m = GetModuleHandleW(name))
					{
						blocking = reinterpret_cast<BlockingFn>(GetProcAddress(m, "IsAnyBlockingWindowOpened"));
						break;
					}
				}
				logger::debug("menu framework IsAnyBlockingWindowOpened {}", blocking ? "found" : "not found (no framework, or an older one)");
			}
			return blocking && blocking();
		}

		RE::GPtr<RE::InventoryMenu> InventoryInFront(std::string& a_why)
		{
			auto* ui = RE::UI::GetSingleton();
			if (!ui) { a_why = "the UI is not ready"; return nullptr; }
			if (!ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME)) { a_why = "the Inventory is not open"; return nullptr; }
			for (const char* name : kCoveringMenus)
			{
				if (ui->IsMenuOpen(name)) { a_why = std::format("{} is open over the Inventory", name); return nullptr; }
			}
			auto menu = ui->GetMenu<RE::InventoryMenu>();
			if (!menu) { a_why = "the Inventory menu object is not available"; return nullptr; }
			return menu;
		}

		Highlight ReadHighlight(RE::InventoryMenu& a_menu)
		{
			Highlight h;
			// Favourited, natively: the highlighted row's inventory entry and its hotkey extra data.
			if (auto* list = a_menu.GetRuntimeData().itemList)
			{
				if (auto* item = list->GetSelectedItem(); item && item->data.objDesc)
				{
					h.valid = true;
					h.favourited = item->data.objDesc->IsFavorited();
					const char* name = item->data.objDesc->GetDisplayName();
					const auto* obj = item->data.objDesc->GetObject();
					h.item = std::format("'{}' {:08X}", name ? name : "", obj ? obj->GetFormID() : 0);
				}
			}
			// In the Back Pocket: Back Pocket's bit on the row SkyUI shows, or its category is the one showing.
			if (auto* movie = a_menu.uiMovie.get())
			{
				RE::GFxValue itemList, entry, flag;
				if (movie->GetVariable(&itemList, kItemListPath) && itemList.IsObject() &&
					itemList.GetMember("selectedEntry", &entry) && entry.IsObject() && entry.GetMember("filterFlag", &flag))
				{
					if (const auto f = NumberAsU32(flag)) { h.pocketed = (*f & kPocketFilterFlag) != 0; }
				}
				RE::GFxValue categoryList, category, marker;
				if (!h.pocketed && movie->GetVariable(&categoryList, kCategoryListPath) && categoryList.IsObject() &&
					categoryList.GetMember("selectedEntry", &category) && category.IsObject() &&
					category.GetMember(kCategoryMarker, &marker) && marker.IsBool())
				{
					h.pocketed = marker.GetBool();
				}
			}
			return h;
		}

		std::string Describe(const Highlight& a_h)
		{
			return std::format("{} (favourited={} inBackPocket={})", a_h.item, a_h.favourited, a_h.pocketed);
		}

		// 1.0.1: bRequireFavourite=false lets a hold pocket whatever is highlighted (the owner, 2026-09-22);
		// true (the default) keeps 1.0.0's rule - favourites in, pocketed items out, everything else untouched.
		bool Eligible(const Highlight& a_h)
		{
			return a_h.valid && (a_h.pocketed || a_h.favourited || !settings::general::requireFavourite);
		}

		void Splice(RE::InputEvent** a_events, const Pending& a_p, float a_value, float a_held)
		{
			std::string_view name{};
			if (a_p.named)
			{
				// Resolve the user event in the context the game is IN, walking the context stack from
				// the top the way the engine does (Perfected Wheeler's replay, measured 2026-09-13).
				if (auto* controlMap = RE::ControlMap::GetSingleton())
				{
					const auto& stack = controlMap->contextPriorityStack;
					for (std::uint32_t i = stack.size(); i > 0 && name.empty(); --i)
					{
						name = controlMap->GetUserEventName(a_p.code, a_p.device, stack[i - 1]);
					}
					if (name.empty()) { name = controlMap->GetUserEventName(a_p.code, a_p.device); }
				}
			}
			RE::BSFixedString userEvent(name.empty() ? "" : std::string(name).c_str());
			auto* ev = RE::ButtonEvent::Create(a_p.device, userEvent, a_p.code, a_value, a_held);
			if (!ev)
			{
				logger::warn("ButtonEvent::Create failed; a press was not delivered");
				return;
			}
			ev->next = *a_events;
			*a_events = ev;
			if (a_p.mine) { g_ours.push_back(ev); }
			logger::debug("spliced {} {} code 0x{:X} userEvent '{}' held {:.2f}",
						  a_p.device == RE::INPUT_DEVICE::kGamepad ? "gamepad" : "keyboard", a_value > 0.0F ? "DOWN" : "UP", a_p.code, name, a_held);
		}

		void Service(RE::InputEvent** a_events)
		{
			// Called with g_lock held.
			g_ours.clear();   // last dispatch's pointers are dead
			for (auto it = g_pending.begin(); it != g_pending.end();)
			{
				if (!it->downSent) { Splice(a_events, *it, 1.0F, 0.0F); it->downSent = true; it->framesTotal = it->framesLeft; ++it; continue; }
				// held time rises by one 60 Hz frame per dispatch, as a hardware hold does
				if (it->framesLeft-- > 0) { Splice(a_events, *it, 1.0F, static_cast<float>(it->framesTotal - it->framesLeft) / 60.0F); ++it; continue; }
				Splice(a_events, *it, 0.0F, 0.1F);
				it = g_pending.erase(it);
			}
		}

		bool Ours(const RE::InputEvent* a_event)
		{
			for (const auto* e : g_ours) { if (e == a_event) { return true; } }
			return false;
		}

		// True when the event is swallowed. Called with g_lock held, on the game's input dispatch.
		bool Filter(RE::InputEvent* a_event)
		{
			auto* button = a_event->AsButtonEvent();
			if (!button || button->GetDevice() != RE::INPUT_DEVICE::kGamepad || button->GetIDCode() != kButtonY || Ours(a_event)) { return false; }

			if (button->IsDown())
			{
				g_armed = false;
				g_sent = false;
				if (g_state.upstreamLoaded)
				{
					Decide("passed: the original Back Pocket (BackPocket.dll) is also installed, so this mod stands down");
					return false;
				}
				std::string why;
				auto menu = InventoryInFront(why);
				if (!menu) { ++g_state.passedNotInventory; Decide("passed: " + why); return false; }
				if (FrameworkWindowOpen()) { ++g_state.passedFrameworkOpen; Decide("passed: the menu framework's window is open"); return false; }
				const Highlight h = ReadHighlight(*menu);
				if (!h.valid) { Decide("passed: no highlighted item"); return false; }
				g_state.lastItem = Describe(h);
				if (!Eligible(h)) { ++g_state.passedNotEligible; Decide("passed: " + g_state.lastItem + " - not a favourite and not in the Back Pocket"); return false; }
				g_armed = true;
				++g_state.armedCount;
				Decide("timing a hold on " + g_state.lastItem);
				return true;
			}

			if (!g_armed) { return false; }

			if (button->IsPressed())
			{
				if (!g_sent && button->HeldDuration() >= settings::general::holdSeconds)
				{
					g_sent = true;
					std::string why;
					auto menu = InventoryInFront(why);
					const Highlight h = menu ? ReadHighlight(*menu) : Highlight{};
					if (!menu || !Eligible(h))
					{
						// The highlight moved (or the menu went) during the hold - do nothing rather than move the wrong item.
						Decide(menu ? "held, but the highlighted item is now neither a favourite nor in the Back Pocket - nothing sent" : "held, but " + why + " - nothing sent");
						return true;
					}
					back_pocket::inventory_filter::request_toggle_item();
					g_state.lastItem = Describe(h);
					if (h.pocketed) { ++g_state.takenOut; } else { ++g_state.sentIn; }
					Decide(std::format("held {:.2f}s: toggle requested to {} {}", button->HeldDuration(),
									   h.pocketed ? "take out" : "send in", g_state.lastItem));
					logger::info("{} {}", h.pocketed ? "taking out of Back Pocket:" : "sending to Back Pocket:", g_state.lastItem);
				}
				return true;
			}

			// released
			g_armed = false;
			if (!g_sent)
			{
				std::string why;
				if (InventoryInFront(why))
				{
					g_pending.push_back({ RE::INPUT_DEVICE::kGamepad, kButtonY, 1, true, true });
					++g_state.tapsGivenBack;
					Decide(std::format("tap ({:.2f}s): Y given back to the Inventory", button->HeldDuration()));
				}
				else
				{
					Decide("tap, but " + why + " - not given back");
				}
			}
			g_sent = false;
			return true;
		}

		struct PollInputDevicesHook
		{
			static inline REL::Relocation<void(RE::BSTEventSource<RE::InputEvent*>*, RE::InputEvent**)> func;

			static void thunk(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_events)
			{
				bool writeReport = false;
				if (a_events)
				{
					std::scoped_lock l(g_lock);
					Service(a_events);
					for (RE::InputEvent** link = a_events; *link;)
					{
						RE::InputEvent* ev = *link;
						if (Filter(ev)) { *link = ev->next; continue; }
						link = &ev->next;
					}
					g_state.armed = g_armed;
					writeReport = std::exchange(g_reportDirty, false);
				}
				if (writeReport) { SelfCheck::Write(); }   // outside the lock: the report reads the snapshot
				func(a_dispatcher, a_events);
			}
		};

	}

	bool Install()
	{
		const std::uintptr_t site = kPollInputDevicesID.address() + kPollInputDevicesOffset;
		if (!REL::make_pattern<"E8">().match(site))
		{
			logger::error("PollInputDevices site 0x{:X} (ID {}+0x{:X}) is not a call instruction; the hold is NOT installed",
						  site, kPollInputDevicesID.id(), kPollInputDevicesOffset);
			SelfCheck::Set("Input hook", false, std::format("site 0x{:X} (ID {}+0x{:X}) is not a call", site, kPollInputDevicesID.id(), kPollInputDevicesOffset));
			return false;
		}
		auto& trampoline = SKSE::GetTrampoline();
		PollInputDevicesHook::func = trampoline.write_call<5>(site, PollInputDevicesHook::thunk);
		{
			std::scoped_lock l(g_lock);
			g_state.hookInstalled = true;
		}
		logger::info("PollInputDevices hook installed at 0x{:X} (ID {}+0x{:X})", site, kPollInputDevicesID.id(), kPollInputDevicesOffset);
		SelfCheck::Set("Input hook", true, std::format("installed at 0x{:X} (ID {}+0x{:X})", site, kPollInputDevicesID.id(), kPollInputDevicesOffset));
		return true;
	}

	void CheckForUpstream()
	{
		// This mod IS Back Pocket (forked, with the Y hold built in). The original's DLL beside it would filter the same
		// lists and write the same co-save record, so when it is present this one stands down and says so.
		const bool upstream = GetModuleHandleW(L"BackPocket.dll") != nullptr;
		{
			std::scoped_lock l(g_lock);
			g_state.upstreamLoaded = upstream;
		}
		if (upstream)
		{
			logger::error("the original Back Pocket (BackPocket.dll) is also loaded; Back Pocket for Controller replaces it - disable one of them");
		}
		SelfCheck::Set("No second Back Pocket", !upstream,
					   upstream ? "BackPocket.dll (the original) is also installed - this mod replaces it; disable the original" : "the original BackPocket.dll is not loaded");
	}

	Snapshot GetSnapshot()
	{
		std::scoped_lock l(g_lock);
		return g_state;
	}

	void QueueTestPress(int a_frames)
	{
		std::scoped_lock l(g_lock);
		// Named and NOT ours: it reaches this patch's own filter like a hardware press.
		g_pending.push_back({ RE::INPUT_DEVICE::kGamepad, kButtonY, a_frames < 1 ? 1 : (a_frames > 600 ? 600 : a_frames), true, false });
		logger::info("DevBench: queued a {}-frame Y press", a_frames);
	}
}
