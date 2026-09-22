#include "pch.h"

#include "config.h"

#include "back_pocket/input_binding.h"

// Back Pocket for Controller: Back Pocket's settings, read from BackPocketForController.ini with ordinary file I/O.
// Upstream read BackPocket.ini through GetPrivateProfile*W; this project never uses the Win32 profile API for a
// plugin's own INI (MO2's usvfs does not reliably redirect it - rule 16), so the same keys, sections, defaults and
// validation are kept and only the reader changed. The controller's LT tap binding ships OFF: holding Y is this
// mod's controller action (the owner, 2026-09-22).
namespace back_pocket::config {
namespace {
std::string lower(std::string text) {
  for (char& c : text) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return text;
}

std::string trim(const std::string& text) {
  const auto b = text.find_first_not_of(" \t\r\n");
  if (b == std::string::npos) {
    return {};
  }
  const auto e = text.find_last_not_of(" \t\r\n");
  return text.substr(b, e - b + 1);
}

// "section:key" (both lower case) -> value
std::map<std::string, std::string> read_ini() {
  std::map<std::string, std::string> keys;
  const auto path = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "BackPocketForController.ini";
  std::ifstream in(path);
  if (!in) {
    logger::warn("{} not found; using Back Pocket's defaults", path.string());
    return keys;
  }
  std::string line;
  std::string section;
  while (std::getline(in, line)) {
    const std::string t = trim(line);
    if (t.empty() || t[0] == ';' || t[0] == '#') {
      continue;
    }
    if (t.front() == '[' && t.back() == ']') {
      section = lower(t.substr(1, t.size() - 2));
      continue;
    }
    const auto eq = t.find('=');
    if (eq == std::string::npos) {
      continue;
    }
    keys[section + ":" + lower(trim(t.substr(0, eq)))] = trim(t.substr(eq + 1));
  }
  return keys;
}

std::optional<long> read_int(const std::map<std::string, std::string>& keys, const char* key) {
  const auto it = keys.find(key);
  if (it == keys.end()) {
    return std::nullopt;
  }
  try {
    return std::stol(it->second, nullptr, 0);
  } catch (...) {
    logger::warn("{} = \"{}\" is not a number", key, it->second);
    return std::nullopt;
  }
}

std::uint32_t read_scan_code(const std::map<std::string, std::string>& keys, const char* key,
                             const std::string_view label, const std::uint32_t fallback,
                             const bool allow_disabled = false) {
  const long configured = read_int(keys, key).value_or(static_cast<long>(fallback));
  if (configured == static_cast<long>(disabled_scan_code) && allow_disabled) {
    return disabled_scan_code;
  }
  if (configured <= static_cast<long>(disabled_scan_code) || configured > 255) {
    logger::warn("invalid {} scan code {}; using {}", label, configured, fallback);
    return fallback;
  }
  return static_cast<std::uint32_t>(configured);
}

std::optional<std::uint32_t> read_gamepad_key_code(const std::map<std::string, std::string>& keys) {
  const auto configured = read_int(keys, "input:controller_toggle_item_key_code");
  if (!configured.has_value() || *configured == -1) {
    return std::nullopt;  // off unless set: holding Y is the controller action
  }
  if (*configured < static_cast<long>(input_binding::minimum_gamepad_key_code) ||
      *configured > static_cast<long>(input_binding::maximum_gamepad_key_code)) {
    logger::warn("invalid controller toggle_item key code {}; leaving the controller tap binding off", *configured);
    return std::nullopt;
  }
  return static_cast<std::uint32_t>(*configured);
}

bool read_bool(const std::map<std::string, std::string>& keys, const char* key, const bool fallback) {
  const auto it = keys.find(key);
  if (it == keys.end()) {
    return fallback;
  }
  const std::string v = lower(it->second);
  if (v == "true" || v == "1" || v == "yes" || v == "on") {
    return true;
  }
  if (v == "false" || v == "0" || v == "no" || v == "off") {
    return false;
  }
  logger::warn("invalid boolean {} = \"{}\"; using {}", key, it->second, fallback);
  return fallback;
}
} // namespace

settings load() {
  const auto keys = read_ini();
  settings result{
      .toggle_item_scan_code = read_scan_code(keys, "input:toggle_item_scan_code", "toggle_item",
                                              default_toggle_item_scan_code),
      .toggle_view_scan_code = read_scan_code(keys, "input:toggle_view_scan_code", "toggle_view",
                                              default_toggle_view_scan_code, true),
      .controller_toggle_item_key_code = read_gamepad_key_code(keys),
      .show_notifications = read_bool(keys, "display:show_notifications", true),
      .hide_pocketed_from_disenchanting = read_bool(keys, "menus:hide_pocketed_from_disenchanting", true),
  };
  if (result.toggle_view_scan_code != disabled_scan_code &&
      result.toggle_item_scan_code == result.toggle_view_scan_code) {
    logger::warn("item and view actions share scan code {}; disabling the view shortcut",
                 result.toggle_view_scan_code);
    result.toggle_view_scan_code = disabled_scan_code;
  }
  logger::info("configuration loaded: toggle_item={}, controller_toggle_item={}, toggle_view={}, "
               "notifications={}, hide_from_disenchanting={}",
               result.toggle_item_scan_code,
               result.controller_toggle_item_key_code.has_value()
                   ? static_cast<int>(*result.controller_toggle_item_key_code)
                   : -1,
               result.toggle_view_scan_code, result.show_notifications,
               result.hide_pocketed_from_disenchanting);
  return result;
}
} // namespace back_pocket::config
