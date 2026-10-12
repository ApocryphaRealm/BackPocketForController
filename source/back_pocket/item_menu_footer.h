#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace back_pocket::item_menu_footer {
enum class menu_kind : std::uint8_t {
  inventory,
  container,
  barter,
  gift,
};

enum class input_family : std::uint8_t {
  keyboard_mouse,
  gamepad,
};

struct presentation {
  bool visible = false;
  std::string_view text;
  // Back Pocket for Controller 1.0.6: the controller has no tap binding by default (the hold on the Favourite button
  // is its action), so with a gamepad and no tap key the footer shows this prompt on this key instead - set only where
  // the hold works (an item in the Back Pocket, in the Inventory).
  std::optional<std::uint32_t> hold_key_code;
  std::string_view hold_text;
};

using presentation_provider = presentation (*)(RE::GFxMovie&, menu_kind, bool footer_selected);

[[nodiscard]] bool install(std::uint32_t keyboard_scan_code,
                           std::optional<std::uint32_t> gamepad_key_code,
                           presentation_provider provider);
[[nodiscard]] bool set_input_family(input_family input);
[[nodiscard]] bool refresh(RE::GFxMovieView& view, bool footer_selected);
// For the DevBench tool: the button the footer last appended (or why it appended none), as a JSON object.
[[nodiscard]] std::string last_footer_json();
} // namespace back_pocket::item_menu_footer
