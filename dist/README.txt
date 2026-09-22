Back Pocket For Controller
==========================
Version 1.0.0

Keeps the items you never sell or drop out of the way. A Back Pocket category in SkyUI's item menus
holds them, and the regular categories stop listing them. They stay in your inventory.

CONTROLLER: in the Inventory, HOLD Y on a favourited item to send it to the Back Pocket, or on an
item in the Back Pocket to take it out. A quick press of Y is still the Favourite button, and on any
other item Y works exactly as before.
KEYBOARD: B moves the highlighted item in or out.

REQUIREMENTS
------------
- SKSE, Address Library for SKSE Plugins, SkyUI.
- Skyrim SE 1.5.97, AE 1.6.1170 or GOG 1.6.1179.

INSTALLATION
------------
Install with a mod manager. No plugin, load order does not matter.
This replaces Back Pocket - disable or remove Back Pocket if you have it. A save made with Back
Pocket keeps its pocketed items.

SETTINGS
--------
SKSE\Plugins\BackPocketForController.ini:
  fHoldSeconds (0.50)                    how long Y has to be held
  toggle_item_scan_code (48 = B)         the keyboard key
  toggle_view_scan_code (0 = off)        a keyboard shortcut between Back Pocket and your last category
  controller_toggle_item_key_code (-1)   an extra controller TAP binding, off by default
  show_notifications, hide_pocketed_from_disenchanting

WHAT CHANGED
------------

Version 1.0.0
First release.

TROUBLESHOOTING
---------------
Documents\My Games\Skyrim Special Edition\SKSE\BackPocketForController-selfcheck.txt says whether
everything came up and what the last Y press did. The log beside it has the detail. The .pdb ships
beside the DLL for Crash Logger.

CREDIT
------
Built on Back Pocket by ThePenguinT (MIT) - https://www.nexusmods.com/skyrimspecialedition/mods/188847

LICENCE
-------
GPL-3.0-or-later. Source: https://github.com/ApocryphaRealm/BackPocketForController
