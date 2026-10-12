#pragma once

#include <cstdint>
#include <optional>

namespace back_pocket::input_binding {
using scan_code = std::uint32_t;
using gamepad_key_code = std::uint32_t;
using gamepad_button_id = std::uint32_t;

inline constexpr gamepad_key_code minimum_gamepad_key_code = 266;
inline constexpr gamepad_key_code maximum_gamepad_key_code = 281;

enum class button_phase : std::uint8_t {
  down,
  held,
  up,
};

struct controller_press {
  bool trigger_down = false;
  bool modified = false;
  bool keyboard_action_observed = false;
};

enum class controller_result : std::uint8_t {
  none,
  invoke,
};

[[nodiscard]] constexpr std::optional<gamepad_button_id>
gamepad_button(const gamepad_key_code key_code) noexcept {
  switch (key_code) {
  case 266:
    return 0x0001;
  case 267:
    return 0x0002;
  case 268:
    return 0x0004;
  case 269:
    return 0x0008;
  case 270:
    return 0x0010;
  case 271:
    return 0x0020;
  case 272:
    return 0x0040;
  case 273:
    return 0x0080;
  case 274:
    return 0x0100;
  case 275:
    return 0x0200;
  case 276:
    return 0x1000;
  case 277:
    return 0x2000;
  case 278:
    return 0x4000;
  case 279:
    return 0x8000;
  case 280:
    return 0x0009;
  case 281:
    return 0x000A;
  default:
    return std::nullopt;
  }
}

// Back Pocket for Controller 1.0.6: the inverse of gamepad_button - the SkyUI key code (266-281) a footer button needs
// for a button the control map reports as an XInput mask (the hold rides on the Favourite button, 0x8000 = Y = 279).
[[nodiscard]] constexpr std::optional<gamepad_key_code>
gamepad_key_code_of(const gamepad_button_id button) noexcept {
  for (gamepad_key_code code = minimum_gamepad_key_code; code <= maximum_gamepad_key_code; ++code) {
    if (gamepad_button(code) == button) {
      return code;
    }
  }
  return std::nullopt;
}
static_assert(gamepad_key_code_of(0x8000) == 279u);

[[nodiscard]] controller_result observe_controller(controller_press& state,
                                                   gamepad_button_id trigger,
                                                   gamepad_button_id observed, button_phase phase,
                                                   bool another_button_down) noexcept;
void note_keyboard_action(controller_press& state) noexcept;
} // namespace back_pocket::input_binding
