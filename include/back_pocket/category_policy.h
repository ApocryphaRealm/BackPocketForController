#pragma once

#include <cstdint>

namespace back_pocket::category_policy {
// SkyUI reserves bits 0-9 for player inventory and 10-19 for container
// inventory. Bit 20 is unused by the item menus and remains exactly
// representable by ActionScript's 32-bit bitwise operators.
inline constexpr std::uint32_t player_inventory_filter_mask = 0x000003FFu;
inline constexpr std::uint32_t pocket_filter_flag = 0x00100000u;
// Bit 21: the Favourites pocket, when the pockets are split by favourite (bSeparateFavourites). Also unused by the
// item menus and exactly representable in ActionScript's 32-bit operators.
inline constexpr std::uint32_t favourite_pocket_filter_flag = 0x00200000u;
inline constexpr std::uint32_t any_pocket_filter_mask = pocket_filter_flag | favourite_pocket_filter_flag;
inline constexpr std::uint32_t mixed_inventory_filter_flag = player_inventory_filter_mask;

[[nodiscard]] constexpr bool is_player_inventory_filter(
    const std::uint32_t filter_flag) noexcept {
  return (filter_flag & player_inventory_filter_mask) != 0;
}

[[nodiscard]] constexpr bool is_pocket_category(const std::uint32_t filter_flag) noexcept {
  return filter_flag == pocket_filter_flag || filter_flag == favourite_pocket_filter_flag;
}

[[nodiscard]] constexpr bool is_favourite_pocket_category(const std::uint32_t filter_flag) noexcept {
  return filter_flag == favourite_pocket_filter_flag;
}

// A pocketed row carries exactly one pocket bit: the Favourites pocket's when favourite_pocket is set, Back Pocket's
// otherwise. Both bits are cleared first, so a row that changes pocket never keeps the old one.
[[nodiscard]] constexpr std::uint32_t row_filter_flag(const std::uint32_t original, const bool pocketed,
                                                      const bool favourite_pocket = false) noexcept {
  const std::uint32_t base = original & ~any_pocket_filter_mask;
  return pocketed ? base | (favourite_pocket ? favourite_pocket_filter_flag : pocket_filter_flag) : base;
}
} // namespace back_pocket::category_policy
