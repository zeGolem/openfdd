#include "utils.hpp"
#include <stdexcept>
#include <string>

namespace utils
{

int stoi_safe(std::string input,
              stoi_checks checks,
              std::string value_name_for_error)
{
	int converted = 0;
	try {
		converted = std::stoi(input);
	} catch (std::invalid_argument const& e) {
		throw std::runtime_error(value_name_for_error +
		                         " should be a number (got: " + input + ")");
	}

	if (checks.min.has_value() && converted < checks.min.value())
		throw std::runtime_error(value_name_for_error + " should be >" +
		                         std::to_string(checks.min.value()));
	if (checks.max.has_value() && converted > checks.max.value())
		throw std::runtime_error(value_name_for_error + " should be <" +
		                         std::to_string(checks.max.value()));

	return converted;
}

} // namespace utils
