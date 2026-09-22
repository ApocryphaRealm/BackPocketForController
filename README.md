# Back Pocket For Controller

Keeps the items you never sell or drop out of the way: a **Back Pocket** category in SkyUI's item menus holds them,
and every regular category stops listing them. They stay in your inventory; they are just not in your way.

**On a controller:** in the Inventory, **hold Y** on a favourited item to send it to the Back Pocket, or on an item in
the Back Pocket to take it out. A quick press of Y is still the Favourite button, and on any other item Y is untouched.
**On a keyboard:** B moves the highlighted item in or out (`toggle_item_scan_code`).

* Pocket membership is saved per character (SKSE co-save), by base form.
* Container, barter and gift menus filter the player's side only. Pocketed enchanted items are kept out of the
  Disenchant list (`hide_pocketed_from_disenchanting`).
* No plugin, no scripts, no settings page - everything is in `SKSE\Plugins\BackPocketForController.ini`.
* SE 1.5.97, AE 1.6.1170 and GOG 1.6.1179. Needs SKSE, Address Library and SkyUI.
* Replaces Back Pocket: install one or the other. A save made with Back Pocket keeps its pocketed items.

## How the Y hold works

`source/HoldToPocket.cpp` filters the game's input at `BSInputDeviceManager::PollInputDevices` -> `DispatchInputEvent`
(Address Library 67315 / 68617 + 0x7B, checked for a `call` before it is patched). In the Inventory, Y pressed on a
favourited or pocketed row is held back and timed: past `fHoldSeconds` (0.5 s) it runs the same toggle as the
keyboard key; released sooner, the press is given back to the menu as an ordinary Y.

## Diagnostics

* Log: `Documents\My Games\Skyrim Special Edition\SKSE\BackPocketForController.log` (trace by default).
* Self-check: `BackPocketForController-selfcheck.txt` beside it - whether each part came up, and the last Y decision.
* DevBench tool `bpfc.control`: live state; `op=tap` and `op=hold` drive a Y press through the filter.

## Building

`configure.bat` + `build.bat` (CommonLibSSE-NG 3.7, vcpkg). Needs Visual Studio (found with `vswhere`) and
`VCPKG_ROOT`. `python tools/build-package.py` assembles the package.

## Credit and licence

A fork of [Back Pocket](https://www.nexusmods.com/skyrimspecialedition/mods/188847) by ThePenguinT (MIT,
[source](https://github.com/theosw/SkyrimBackPocket)); what changed is listed in `NOTICE.md`. The whole work is
GPL-3.0-or-later (`LICENSE`); Back Pocket's MIT notice is in `THIRD_PARTY_NOTICES.md`.
