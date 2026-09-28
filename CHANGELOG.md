# Changelog - Back Pocket For Controller
## 1.0.2 - 2026-09-28 - working

### Added
* **Two pockets** (the owner, 2026-09-27: *"let's make it so that back pocket has two different back pockets. One for
  favorited items and the other for non-favorited items. And this system can be toggled on or off in the INI file"*).
  Pocketed items that are favourited show in a new **Favourites Pocket** category, the rest in **Back Pocket**; favouriting
  or unfavouriting a pocketed item moves it across. Nothing new is saved - membership is still the one pocket set, and
  the split is the item's favourite mark - so saves need nothing and `bSeparateFavourites=false` (default `true`)
  returns to one Back Pocket at once. The Favourites Pocket entry takes a second reserved filter bit (0x00200000), follows
  the Back Pocket entry at the end of the player segment, keeps the pocket icon, and is the pocket restored after a
  container / gift tab change when it was the one open. The view key goes category -> Back Pocket -> Favourites ->
  category; messages name the pocket.

### Changed
* Ships at info: `uLogLevel=2` in the INI and as the compiled default (every INI of ours, 2026-09-26).

## 1.0.1 - 2026-09-22

### Changed
* **Fixed a crash when favouriting or unfavouriting an item** (the owner, 2026-09-22). The replay path read
  `ControlMap::contextPriorityStack` as a direct member; that member lives in CommonLibSSE-NG's RUNTIME_DATA, and a
  build for SE and AE with VR off gets those members laid out at AE's offsets - so on SE 1.5.97 the array's size was
  really another field, the loop indexed far past the end, and the game died on the first Y press
  (`crash-2026-09-22-20-36-43.log`, `HoldToPocket.cpp:182`). It goes through `GetRuntimeData()` now, which resolves
  per runtime, and the walk is bounded as well: a context stack deeper than 64, or an entry that is not a context id,
  is refused rather than indexed, so a wrong layout can never again be read as memory to walk.
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
