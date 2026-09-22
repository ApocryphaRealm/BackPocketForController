#pragma once

// Back Pocket for Controller. The owner, 2026-09-22: "I just want a very simple mod that allows you to
// hold Y on controller. On favorite items to send them to the back pocket." - and "I still want to be able
// to hold Y to take items out of the back pocket as well."
//
// HOW IT WORKS
//   This mod is Back Pocket (Nexus 188847, ThePenguinT, MIT - forked in source/back_pocket) with a controller hold
//   added. The game's input events pass through BSInputDeviceManager::PollInputDevices -> DispatchInputEvent
//   (RELOCATION_ID 67315 / 68617 + 0x7B, the call site Apocrypha Menu Framework and Perfected Wheeler also hook).
//   This file filters that list, in the Inventory menu only:
//     * Y going DOWN while the highlighted item is FAVOURITED, or is IN THE BACK POCKET (the pocket bit on the
//       row, or the Back Pocket category showing), is held back and timed;
//     * held past fHoldSeconds: back_pocket::inventory_filter::request_toggle_item() - the same toggle the
//       keyboard key runs;
//     * released sooner: the press is given back to the menu as an ordinary Y (the Favourite button);
//     * on any other item, anywhere else, or while AMF's own window is open, Y passes through untouched.

#include <cstdint>
#include <string>

namespace HoldToPocket
{
	// Install the dispatch filter. Call once (kPostLoad). False when the call site is not a call.
	bool Install();

	// True (and reported) when the original BackPocket.dll is also loaded, in which case the hold stands down.
	void CheckForUpstream();

	struct Snapshot
	{
		bool hookInstalled = false;
		bool upstreamLoaded = false;
		bool armed = false;
		std::uint32_t armedCount = 0;
		std::uint32_t sentIn = 0;
		std::uint32_t takenOut = 0;
		std::uint32_t tapsGivenBack = 0;
		std::uint32_t passedNotEligible = 0;
		std::uint32_t passedNotInventory = 0;
		std::uint32_t passedFrameworkOpen = 0;
		std::string lastDecision;
		std::string lastItem;
	};
	Snapshot GetSnapshot();

	// DevBench driving (rule 64): queue a Y press as if from the pad, held for a_frames dispatches
	// (60 a second), so it goes through this patch's own filter exactly like a hardware press.
	void QueueTestPress(int a_frames);
}
