#include "common/command_validation.hpp"
#include <algorithm>
#include <stdexcept>

std::pair<int, int> parse_range(const std::string& range_str) {
    size_t open_paren = range_str.find('(');
    size_t close_paren = range_str.find(')');
    std::string inside = range_str.substr(open_paren + 1, close_paren - open_paren - 1);
    size_t comma = inside.find(',');
    int min_val = std::stoi(inside.substr(0, comma));
    int max_val = std::stoi(inside.substr(comma + 1));
    return {min_val, max_val};
}

bool is_value_valid(const std::vector<std::string>& allowed_values, const std::string& value) {
    if (allowed_values.size() == 1 && allowed_values[0].rfind("range(", 0) == 0) {
        auto [min_val, max_val] = parse_range(allowed_values[0]);
        int numeric_value;
        try {
            numeric_value = std::stoi(value);
        } catch (const std::invalid_argument&) {
            throw std::runtime_error("Value '" + value + "' is not numeric");
        }
        return numeric_value >= min_val && numeric_value <= max_val;
    }
    return std::find(allowed_values.begin(), allowed_values.end(), value) != allowed_values.end();
}
