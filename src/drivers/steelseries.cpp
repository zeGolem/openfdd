#include "steelseries.hpp"
#include "drivers/driver.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
                          std::vector<std::string> const& parameters) const
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

		set_backlight_luminosity(backlight_value);

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

		set_polling_interval(polling_interval);

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

		set_backlight_pattern(pattern);

	} else if (action_id == "save") {
		save();

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

bool rival_3_wireless::is_compatible(std::shared_ptr<usb_device> device)
{
	auto id = device->get_id();
	if (id.id_vendor == steelseries::vendor_id && id.id_product == 0x1830)
		return true;
	return false;
}

void rival_3_wireless::init_config() noexcept
{
	// TODO
}

const std::unordered_map<std::string, action const> rival_3_wireless::
    get_actions() const noexcept
{
	action static_color{
	    .description = "Static color (doesn't work)",
	    .parameters  = {{
	         .type        = parameter::param_string,
	         .name        = "color",
	         .description = "Mousewheel color (format: 'RRGGBB')",
        }},
	};

	action dpi_presset{
	    .description = "Set the active DPI presset",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "presset",
	         .description = "The presset to enable (1-5)",
        }},
	};

	// clang-format off
	// TODO: Clang format seems to do weird stuff with this :/
	action dpi_presset_config{
	    .description = "Configure a DPI presset",
	    .parameters =
	        {
				{
					.type        = parameter::param_uint,
					.name        = "presset",
					.description = "The presset to change (1-5)",
				},
				{
					.type        = parameter::param_uint,
					.name        = "value",
					.description = "DPI value (100-18000)",
				}
			},
	};
	// clang-format on

	action poll_interval{
	    .description = "Set the poll interval",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "interval",
	         .description = "Interval between polls, in ms (1-4)",
        }},
	};

	action ultra_power_saving{
	    .description = "Ultra power saving mode",
	    .parameters  = {{
	         .type = parameter::param_string,
	         .name = "enabled",
	         .description =
                "Is Ultra Power Saving mode enabled? ('true' or 'false')",
        }},
	};

	action smart_lighting{
	    .description = "Smart lighting mode",
	    .parameters  = {{
	         .type = parameter::param_string,
	         .name = "enabled",
	         .description =
                "Is smart lighting mode enabled? ('true' or 'false')",
        }},
	};

	action sleep_time{
	    .description = "Sleep time",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "time",
	         .description = "Time before going to sleep (in seconds)",
        }},
	};

	action save{
	    .description = "Save to onboard memory",
	    .parameters  = {},
	};

	return {
	    {      "static_color",       static_color},
	    {"dpi_presset_config", dpi_presset_config},
	    {       "dpi_presset",        dpi_presset},
	    { "set_poll_interval",      poll_interval},
	    {"ultra_power_saving", ultra_power_saving},
	    {    "smart_lighting",     smart_lighting},
	    {        "sleep_time",         sleep_time},
	    {              "save",               save},
	};
}

void rival_3_wireless::run_action(
    std::string const&              action_id,
    std::vector<std::string> const& parameters) const
{
	if (action_id == "static_color") {
		if (parameters.size() < 1)
			throw std::runtime_error("Missig arguements for static_color");

		auto const& hex_string = parameters[0];

		if (hex_string.length() != 6)
			throw std::runtime_error(
			    "Color must be in the following format: RRGGBB");

		// Parse the RGB values
		std::uint8_t r = std::stoi(hex_string.substr(0, 2), nullptr, 16);
		std::uint8_t g = std::stoi(hex_string.substr(2, 2), nullptr, 16);
		std::uint8_t b = std::stoi(hex_string.substr(4, 2), nullptr, 16);

		set_static_color(r, g, b);

	} else if (action_id == "dpi_presset") {
		if (parameters.size() < 1)
			throw std::runtime_error("Missig arguements for dpi_presset");

		std::uint8_t profile;
		try {
			auto val = std::stoi(parameters[0]);
			if (val < 1 || val > 5) throw std::runtime_error("Invalid profile");
			profile = val;
		} catch (std::invalid_argument const& e) {
			throw std::runtime_error("Couldn't parse number.");
		}

		// TODO: Save set profile!
		set_dpi(profile, m_config.dpi_values);

	} else if (action_id == "dpi_presset_config") {
		if (parameters.size() < 2)
			throw std::runtime_error(
			    "Missig arguements for dpi_presset_config");

		std::uint8_t  profile;
		std::uint16_t new_value;
		try {
			auto _profile = std::stoi(parameters[0]);
			if (_profile < 1 || _profile > 5)
				throw std::runtime_error("Invalid profile");
			profile = _profile;

			auto _new_value = std::stoi(parameters[1]);
			if (_new_value < 100 || _new_value > 18000)
				throw std::runtime_error("Invalid DPI value");
			new_value = _new_value;

		} catch (std::invalid_argument const& e) {
			throw std::runtime_error("Couldn't parse number.");
		}

		// TODO: Save to m_config
		auto dpi_values         = m_config.dpi_values;
		dpi_values[profile - 1] = new_value;
		set_dpi(m_config.active_profile, dpi_values);

	} else if (action_id == "set_poll_interval") {
		if (parameters.size() < 1)
			throw std::runtime_error("Missig arguements for set_poll_interval");

		std::uint8_t interval;
		try {
			auto _interval = std::stoi(parameters[0]);
			if (_interval > 4 || _interval < 1)
				throw std::runtime_error(
				    "Invalid poll interval, value must be between 1 and 4");
			interval = _interval;
		} catch (std::invalid_argument const& e) {
			throw std::runtime_error("Couldn't parse number.");
		}

		set_poll_interval(interval);

	} else if (action_id == "ultra_power_saving") {
		if (parameters.size() < 1)
			throw std::runtime_error(
			    "Missig arguements for ultra_power_saving");

		bool is_active = parameters[0] == "true";

		// TODO: Save to the config!
		set_powersaving_options(
		    is_active, m_config.smart_lighting_mode, m_config.sleep_time);

	} else if (action_id == "smart_lighting") {
		if (parameters.size() < 1)
			throw std::runtime_error("Missig arguements for smart_lighting");

		bool is_active = parameters[0] == "true";

		// TODO: Save to the config!
		set_powersaving_options(
		    m_config.ultra_power_saving_mode, is_active, m_config.sleep_time);

	} else if (action_id == "sleep_time") {
		if (parameters.size() < 1)
			throw std::runtime_error("Missig arguements for sleep_time");

		std::uint16_t sleep_time;
		try {
			auto _sleep_time = std::stoi(parameters[0]);
			// TODO: Is there a max. value? The GUI goes up to 20 minutes...
			if (_sleep_time < 0)
				throw std::runtime_error("Invalid value for sleep_time");

			sleep_time = _sleep_time;
		} catch (std::runtime_error const& e) {
			throw std::runtime_error("Couldn't parse number");
		}

		// TODO: Save to the config!
		set_powersaving_options(m_config.ultra_power_saving_mode,
		                        m_config.smart_lighting_mode,
		                        sleep_time);

	} else if (action_id == "save") {
		// TODO: Save to disk here!
		save();

	} else
		throw std::runtime_error("Unknown action: " + action_id);
}

void rival_3_wireless::set_dpi(std::uint8_t               active_profile_id,
                               std::vector<std::uint16_t> dpi_profiles) const
{
	// The data to send. This is a bit more than what we use for now
	std::vector<std::uint8_t> data(16);
	data.assign(16, 0); // zero-out all the data by default

	data[0] = 0x20;                // This is the ID
	data[1] = dpi_profiles.size(); // Profile count
	data[2] = active_profile_id;   // Profile ID

	// Need to scale the dpi from 100-18000 to 0-0xd6
	std::vector<std::uint8_t> scaled_dpis(dpi_profiles.size());

	auto scale_value = [](std::uint16_t dpi) -> std::uint8_t {
		auto new_val = ((float)(dpi - 100) / (18000 - 100)) * 0xd6;
		if (new_val > 0xd6) return 0xd6;
		if (new_val < 0) return 0;
		return new_val;
	};

	// TODO: Handle profile_count < 5 properly
	for (std::size_t i = 0; i < dpi_profiles.size(); ++i)
		scaled_dpis[i] = scale_value(dpi_profiles[i]);

	data[3]  = scaled_dpis[0]; // Profile 1 DPI
	data[5]  = scaled_dpis[1]; // Profile 2 DPI
	data[7]  = scaled_dpis[2]; // Profile 3 DPI
	data[9]  = scaled_dpis[3]; // Profile 4 DPI
	data[11] = scaled_dpis[4]; // Profile 5 DPI

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void rival_3_wireless::set_poll_interval(std::uint8_t interval) const
{
	// The data to send. This is a bit more than what we use for now
	std::vector<std::uint8_t> data(16);
	data.assign(16, 0); // zero-out all the data by default

	data[0] = 0x17; // This is the ID
	data[1] = interval + 1;

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void rival_3_wireless::set_powersaving_options(bool          ultra_power_saving,
                                               bool          smart_lighting,
                                               std::uint16_t sleep_time) const
{
	// The data to send. This is a bit more than what we use for now
	std::vector<std::uint8_t> data(16);
	data.assign(16, 0); // zero-out all the data by default

	data[0] = 0x2b; // This is the ID
	data[1] = ultra_power_saving;

	// TODO: I don't really know how this value works :/
	data[2] = smart_lighting ? 0x32 : 0x00;
	if (ultra_power_saving) data[2] = 0x64;

	// TODO: data[3] && data[4] are probably for storing a delay, possibly the
	//       smart lighting delay, but I haven't tested yet.
	data[3] = 0x2c;
	data[4] = 0x01;

	data[5] = sleep_time & 0xff;        // Take the bottom 8 bits of sleep_time
	data[6] = (sleep_time >> 8) & 0xff; // Take the top 8 bits of sleep_time

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void rival_3_wireless::save() const
{
	m_device->control_transfer(0x21, 0x09, 0x0200, 3, {0x09}, 1000);
}

// TODO: This doesn't do anything ;(
void rival_3_wireless::set_static_color(std::uint8_t r,
                                        std::uint8_t g,
                                        std::uint8_t b) const
{
	// ================ NOTE =================
	// This is very much a work in progress,
	// I have yet to figure out how most of it
	// all works. It seems more complicated
	// due to the mouse being wireless
	// =======================================

	// Part 1: Sending the color
	{
		// Create a vector for the USB data we're going to send.
		// We will send 64 bytes
		// TODO: Only 48 bytes seem to be used?
		std::vector<std::byte> data(64);

		data.assign(64,
		            std::byte{0x00}); // Make sure everything is 0-initialized

		// This is what setting the color to ff ff ff looks like:
		//
		// 03 00 00 00 30 00 88 13 00 00 00 00 00 00 00 00 00 00 00 00 00 00
		// 01 00 00 00 00 00 00 00 01 ff ff ff ff ff ff 00 00 00

		data[0] = std::byte{0x03}; // Should be the command ID, though it
		                           // seems to be shared by many functions
		                           // of the Windows software...
		// This is just matching the hexdump I have for setting ff ff ff...
		// I have no idea what any of those mean
		data[4]  = std::byte{0x30};
		data[6]  = std::byte{0x88};
		data[7]  = std::byte{0x13};
		data[22] = std::byte{0x01};
		data[30] = std::byte{0x01};

		// The color (twice in a row, still haven't figured out why
		data[31] = std::byte{r};
		data[32] = std::byte{g};
		data[33] = std::byte{b};
		data[34] = std::byte{r};
		data[35] = std::byte{g};
		data[36] = std::byte{b};

		// m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
	}
	// Part 2: Sending the weird packet to apply it..?
	{
		// Create a vector for the USB data we're going to send.
		// We will send 5 bytes
		std::vector<std::byte> data(64);
		data.assign(64,
		            std::byte{0x00}); // Make sure everything is 0-initialized

		// This is what one of these weird packets look like
		//
		// 03 00 30 00 2c

		data[0] = std::byte{0x03}; // Should be the command ID, though it
		                           // seems to be shared by many functions
		                           // of the Windows software...
		// This is just matching the hexdump I have for setting ff ff ff...
		// I have no idea what any of those mean
		data[1] = std::byte{0x00};
		data[2] = std::byte{0x30};
		data[3] = std::byte{0x00};
		data[4] = std::byte{0x2c};

		// m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
	}
}

} // namespace steelseries
} // namespace drivers
