#pragma once
#include <string>
#include <vector>
#include <utility>

// Parses a "range(min,max)" string (e.g. "range(16,32)") into its numeric
// bounds. Throws std::runtime_error if range_str isn't well-formed —
// indicates a malformed devices.json entry, not a bad user/model value.
[[nodiscard]] std::pair<int, int> parse_range(const std::string& range_str);

// Returns true if value is legal against allowed_values:
// - Discrete list (2+ or 1 non-range entries): exact string match.
// - Range marker (single "range(min,max)" entry): value parsed as an
//   integer and checked inclusively against [min, max] -- returns false
//   (not throw) if the number is simply out of bounds.
// Throws std::runtime_error if value can't be parsed as a number at all
// when validating against a range -- this is a genuinely malformed
// answer, distinct from a numeric-but-out-of-bounds one.
[[nodiscard]] bool is_value_valid(const std::vector<std::string>& allowed_values, const std::string& value);
