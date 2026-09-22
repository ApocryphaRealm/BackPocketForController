# Back Pocket for Controller - copyright and licence

Copyright (C) 2026 ApocryphaRealm

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program (`LICENSE`). If not, see
<https://www.gnu.org/licenses/>.

SPDX-License-Identifier: GPL-3.0-or-later

## Why GPL-3.0-or-later

Every mod of this project is GPL-3.0-or-later by default (the owner, 2026-09-13), and for this one the owner said so
directly: *"in gpl 3.0"*. Back Pocket's licence (MIT) permits that: its notice is kept, and the work as a whole is
distributed under GPL-3.0-or-later. The SE 1.5.97 / AE 1.6.x build links CommonLibSSE-NG 3.7.0 (MIT).

## Where it comes from

Back Pocket for Controller is a fork of **Back Pocket** (https://www.nexusmods.com/skyrimspecialedition/mods/188847,
by ThePenguinT; source https://github.com/theosw/SkyrimBackPocket at commit 6d6112c6b46803d88deb7a850719c5ca87de612e),
under the MIT terms. Back Pocket's code is in `source/back_pocket/` and `include/back_pocket/`, its category icon in
`dist/Interface/BackPocket/`, and its MIT notice in `THIRD_PARTY_NOTICES.md` and `LICENSE-BackPocket-MIT.txt`.

Changes made to Back Pocket's code, 2026-09-22 (each marked "Back Pocket for Controller" at the site):
* `config.cpp` rewritten to read `BackPocketForController.ini` with ordinary file I/O instead of the Win32 profile API;
  same keys, defaults and validation. The controller tap binding (`controller_toggle_item_key_code`) defaults to off.
* `inventory_filter`: `request_toggle_item()` added, the entry point the Y hold calls.
* `disenchant_filter.cpp`: its own trampoline allocation removed; the plugin allocates one for both hooks.
* `plugin.cpp` replaced by `source/main.cpp` (same start-up order, plus the Y hold, guards and self-check).

Added: `source/HoldToPocket.cpp` (the Y hold), `SelfCheck`, `Settings`, `DevBenchTool`, the Address Library guard.

Components under other licences, with their notices: `THIRD_PARTY_NOTICES.md`.

Source code: https://github.com/ApocryphaRealm/BackPocketForController
