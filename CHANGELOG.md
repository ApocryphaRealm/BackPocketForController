# Changelog - Back Pocket For Controller
## 1.0.1 - 2026-09-22

### Changed
* **No Back Pocket category in the trading menu.** The point of the Back Pocket is to hold what you do not mean to
  trade, so the merchant screen now gets the filters but no category: pocketed items appear in no list there and
  cannot be sold by accident; take one out in your inventory first (the owner: *"all we really need to do is make
  sure that the back pocket doesn't show up in the trading menu at all"*). This also removes the overlapping category
  icon borokoshow saw when switching between give and take, which came from the list being rebuilt per side.
* **`bRequireFavourite` in the INI** (default `true`, which is 1.0.0's behaviour): with it off, holding Y sends
  whatever is highlighted to the Back Pocket, without favouriting it first.

### Fixed
* The category icon could be drawn over another category's in a menu that rebuilds its list with a different number
  of player categories (the Gift menu; the Barter menu no longer has the category at all). The icon-label array is
  trimmed to our entry, so a label left by a longer list cannot land on someone else's category.

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
