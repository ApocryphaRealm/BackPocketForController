# Changelog - Back Pocket for Controller

## 1.0.0

First release (the owner, 2026-09-22: *"I just want a very simple mod that allows you to hold Y on controller. On
favorite items to send them to the back pocket"*, *"I still want to be able to hold Y to take items out of the back
pocket as well"*, *"back pocket is mit liscence so lets make this a standalone mod"* - *"in gpl 3.0"*).

* Back Pocket 0.2.2 (ThePenguinT, MIT) forked into one standalone mod, GPL-3.0-or-later.
* Controller: in the Inventory, hold Y on a favourited item to send it to the Back Pocket, or on an item in the Back
  Pocket to take it out; a quick press of Y is still Favourite. The LT tap binding is off by default.
* Settings read with plain file I/O from `BackPocketForController.ini`; no settings page.
* Stands down if the original Back Pocket's DLL is also installed; reads Back Pocket's co-save record, so a Back
  Pocket save keeps its pocketed items.
