#pragma once

#include "back_pocket/pocket.h"
#include "config.h"

namespace back_pocket::inventory_filter {
[[nodiscard]] bool install(pocket& state, const config::settings& settings);

// Back Pocket for Controller: toggle the highlighted item in or out of the Back Pocket, exactly as the
// keyboard toggle key does (queued to the UI thread and validated against the open item menu).
void request_toggle_item();
} // namespace back_pocket::inventory_filter
