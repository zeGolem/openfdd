#pragma once

#include <optional>
#include <string>
namespace utils
{

struct stoi_checks {
	std::optional<int> min;
	std::optional<int> max;
};

int stoi_safe(std::string input, stoi_checks, std::string value_name_for_error);

} // namespace utils
