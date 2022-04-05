#include "apex_100.hpp"
#include "steelseries.hpp"
#include <memory.h>

namespace drivers
{
namespace steelseries
{

bool apex_100::is_compatible(std::shared_ptr<usb_device> dev)
{
	auto id = dev->get_id();
	if (id.id_vendor == steelseries::vendor_id && id.id_product == 0x160e)
		return true;
	return false;
}

const std::unordered_map<std::string, action const> apex_100::get_actions()
    const noexcept
{
	auto luminosity = action{
	    .description = "Backlight luminosity",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "luminosity",
	         .description = "Luminosity percentage",
        }},
	};

	auto backlight_patterns = action{
	    .description = "Backlight patterns",
	    .parameters  = {{
	         .type = parameter::param_string,
	         .name = "pattern",
	         .description =
                "Pattern to use ('static', 'slow', 'medium', 'fast')",
        }},
	};

	auto polling = action{
	    .description = "Polling interval",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "interval",
	         .description = "Interval between polls, between 1ms and 4ms",
        }},
	};

	auto save = action{
	    .description = "Save data to onboard memory",
	    .parameters  = {},
	};

	return {
	    {"backlight_luminosity",         luminosity},
	    {  "backlight_patterns", backlight_patterns},
	    {    "polling_interval",            polling},
	    {                "save",               save},
	};
}

void apex_100::run_action(std::string const&              action_id,
                          std::vector<std::string> const& parameters)
{
	if (action_id == "backlight_luminosity") {
		if (parameters.size() < 1)
			throw std::runtime_error(
			    "Missig arguements for backlight_luminosity");

		std::uint8_t backlight_value;
		try {
			auto val = std::stoi(parameters[0]);
			if (val < 0 || val > 100)
				throw std::runtime_error("Invalid luminosity value");
			backlight_value = val;
		} catch (std::invalid_argument const& e) {
			throw std::runtime_error("Luminosity value isn't a number (" +
			                         parameters[0] + ")");
		}

		m_config.backlight_luminosity = backlight_value;
		set_backlight_luminosity(backlight_value);
		save_config();

	} else if (action_id == "polling_interval") {
		if (parameters.size() < 1)
			throw std::runtime_error("Missig arguements for polling_interval");

		std::uint8_t polling_interval;
		try {
			auto val = std::stoi(parameters[0]);
			if (val < 1 || val > 4)
				throw std::runtime_error("Invalid polling interval value");
			polling_interval = val;
		} catch (std::invalid_argument const& e) {
			throw std::runtime_error("Polling value isn't a number (" +
			                         parameters[0] + ")");
		}

		m_config.polling_interval = polling_interval;
		set_polling_interval(polling_interval);
		save_config();

	} else if (action_id == "backlight_patterns") {
		if (parameters.size() < 1)
			throw std::runtime_error(
			    "Missig arguements for backlight_patterns");

		auto pattern_string = parameters[0];

		backlight_pattern pattern;
		if (pattern_string == "static")
			pattern = backlight_pattern::static_;
		else if (pattern_string == "slow")
			pattern = backlight_pattern::slow;
		else if (pattern_string == "medium")
			pattern = backlight_pattern::medium;
		else if (pattern_string == "fast")
			pattern = backlight_pattern::fast;
		else
			throw std::runtime_error("Invalid backlight pattern");

		m_config.pattern = pattern;
		set_backlight_pattern(pattern);
		save_config();

	} else if (action_id == "save") {
		save();
		save_config();

	} else
		throw std::runtime_error("Invalid action id: " + action_id);
}

void apex_100::set_backlight_luminosity(std::uint8_t luminosity) const
{
	if (luminosity > 100) // No need to check if luminosity < 0
	                      // because it's an unsigned variable.
		throw std::runtime_error("Luminosity should be between [0-100]");

	// Create a vector for the USB data we're going to send.
	// We will send 3 bytes.
	std::vector<std::uint8_t> data(3);

	data[0] = 0x05;       // Command ID
	data[1] = 0x00;       // Null byte for reasons
	data[2] = luminosity; // The luminosity value to set

	m_device->control_transfer(0x21, 0x09, 0x0200, 1, data, 1000);
}

void apex_100::set_backlight_pattern(backlight_pattern pattern) const
{
	// Create a vector for the USB data we're going to send.
	// We will send 3 bytes.
	std::vector<std::uint8_t> data(3);

	data[0] = 0x07;    // Command ID
	data[1] = 0x00;    // Null byte for reasons
	data[2] = pattern; // The pattern to use. The backlight_pattern
	                   // enum is of type std::uint8_t, so no need to
	                   // do any convertion

	m_device->control_transfer(0x21, 0x09, 0x0200, 1, data, 1000);
}

void apex_100::set_polling_interval(std::uint8_t polling_interval) const
{
	if (polling_interval > 4 || polling_interval < 1)
		throw std::runtime_error("Polling interval should be between [1-4]");

	// Create a vector for the USB data we're going to send.
	// We will send 3 bytes.
	std::vector<std::uint8_t> data(3);

	data[0] = 0x04;             // Command ID
	data[1] = 0x00;             // Null byte for reasons
	data[2] = polling_interval; // The polling value to set

	m_device->control_transfer(0x21, 0x09, 0x0200, 1, data, 1000);
}

void apex_100::save() const
{
	// Send a save instruction
	m_device->control_transfer(0x21, 0x09, 0x0200, 1, {0x09}, 1000);
}

nlohmann::json apex_100::serialize_current_config() const noexcept
{
	return {
	    {"backlight_luminosity", m_config.backlight_luminosity},
	    {   "backlight_pattern",              m_config.pattern},
	    {    "polling_interval",     m_config.polling_interval},
	};
}

void apex_100::deserialize_config(nlohmann::json const& config_on_disk)
{
	m_config.backlight_luminosity =
	    config_on_disk.value("backlight_luminosity", 0);
	m_config.pattern =
	    config_on_disk.value("backlight_pattern", backlight_pattern::static_);
	m_config.polling_interval = config_on_disk.value("polling_interval", 1);
}

} // namespace steelseries
} // namespace drivers
