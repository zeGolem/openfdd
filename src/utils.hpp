#pragma once

#include <optional>
#include <stdexcept>
#include <string>
namespace utils
{

struct stoi_checks {
	std::optional<int> min;
	std::optional<int> max;
};

int stoi_safe(std::string input, stoi_checks, std::string value_name_for_error);

void ensure_range(auto        value,
                  auto        min,
                  auto        max,
                  std::string value_name_for_error)
{
	if (value < min || value > max)
		throw std::runtime_error(
		    value_name_for_error + " out of range. Must be between " +
		    std::to_string(min) + " and " + std::to_string(max));
}

constexpr auto scale(
    auto value, auto from_min, auto from_max, auto to_min, auto to_max)
{
	// Fancy math stuff that I don't really know how to explain, but it works!
	return (((value - from_min) / (from_max - from_min)) * (to_max - to_min)) +
	       to_min;
}

} // namespace utils
