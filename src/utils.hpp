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

} // namespace utils
