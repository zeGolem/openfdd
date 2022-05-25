#include "driver.hpp"
#include <stdexcept>

namespace drivers
{

void driver::run_action(std::string const&              action_id,
                        std::vector<std::string> const& parameters)
{
	if (!m_action_handlers.contains(action_id))
		throw std::runtime_error("Unexpected action: " + action_id);

	auto handler = m_action_handlers.at(action_id);
	return handler(parameters);
}

void driver::register_action(std::string const&     id,
                             action const&          the_action,
                             action::handler const& handler)
{
	m_actions.insert({id, the_action});
	m_action_handlers.insert({id, handler});
}

constexpr std::string parameter::type_to_string(enum type const& t) noexcept
{
	switch (t) {
	case uint:
		return "uint";
	case string:
		return "string";
	case rgb_color:
		return "rgb_color";
	case bool_:
		return "bool";
	}
}
} // namespace drivers
